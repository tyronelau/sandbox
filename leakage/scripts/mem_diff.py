#! /usr/bin/python
# -*- coding: utf-8 -*-
import sys
import re
import os
import os.path
import tempfile
import argparse

kInitState = 0 # initial state, waiting for prelogue
kSoState = 1 # prelogue found, handling the shared libraries
kBacktraceState = 2 # shared libraries processed, waiting for a complete bt
kInCallstackState = 3 # handling callstacks
kEndState = 4 # Memory dump over

shared_libraries = {}

g_lib_search_path = [".", "/home/tyrone/src/debug/minote", \
    "/home/tyrone/src/debug/minote/lib"]

g_is_android = False

def findLibInDirectory(path, d):
  (path, sopath) = os.path.split(path)
  failed = False

  while not os.access(os.path.join(d, sopath), os.F_OK):
    if path == "/" or path == "":
      failed = True
      break
    (path, tail) = os.path.split(path)
    sopath = os.path.join(tail, sopath)

  if not failed:
    return os.path.join(d, sopath)
  return None

def findLibraryPath(p):
  for d in g_lib_search_path:
    path = findLibInDirectory(p, d)
    if path:
      return path
  return p

kUnknownType = 0
kExecutableType = 1
kDynamicType = 2

def getObjectFileType(path):
  type_pattern = "Type:\s*(\w*)\s*\("
  load_pattern = "LOAD\s*0x([a-fA-F0-9]+)\s0x([a-fA-F0-9]+)"
  elf_cmd = "readelf -hl %s" %path
  object_type = kUnknownType
  base_address = 0

  for line in os.popen(elf_cmd, "r"):
    m = re.search(type_pattern, line)
    if m:
      exec_type = m.group(1)
      if exec_type == "EXEC":
        object_type = kExecutableType
      elif exec_type == "DYN":
        object_type = kDynamicType
      else:
        object_type = kUnknownType
      continue
    m = re.search(load_pattern, line)
    if m:
      offset = int(m.group(1), 16)
      addr = int(m.group(2), 16)
      if offset == 0:
        base_address = addr

  return (object_type, base_address)

class SourceInfo(object):
  def __init__(self, source, line, symbol):
    self.source = source
    self.line = line
    self.symbol = symbol

class Library(object):
  def __init__(self, path):
    self.path = path
    self.pending_addresses = set()
    self.address_db = {}

  def addPc(self, rel_pc):
    self.pending_addresses.add(rel_pc)

  def queryPc(self, rel_pc):
    if self.address_db.has_key(rel_pc):
      return self.address_db[rel_pc]
    return None

  def parseSourceInfo(self, so_path):
    (object_type, base_address) = getObjectFileType(so_path)

    addresses = [a + base_address for a in self.pending_addresses]
    addresses.sort()

    cmd_fd = tempfile.NamedTemporaryFile(delete=False)
    for a in addresses:
      print >>cmd_fd, "info line *0x%x" %a
    cmd_file = cmd_fd.name
    cmd_fd.close()

    gdb = "gdb"
    if g_is_android:
      gdb = "arm-linux-androideabi-gdb"

    gdb_command = "%s %s -batch -x %s" %(gdb, so_path, cmd_file)
    fd = os.popen(gdb_command, "r")

    index = 0
    for line in fd:
      pattern = "Line\s+(\d+)\s*of\s*\"([^\"]*)\"\s*starts at address [^<]*<(.*)> and ends at"
      m = re.search(pattern, line)
      if m:
        lineno = m.group(1)
        source = m.group(2)
        symbol = m.group(3)

        rel_pc = addresses[index] - base_address
        index += 1
        self.address_db[rel_pc] = SourceInfo(source, lineno, symbol)
        continue

      pattern2 = "No line number information available for address [^<]*<([^\+]*)\+(\d+)>"
      m = re.search(pattern2, line)
      if m:
        lineno = ""
        source = ""
        symbol = m.group(1)
        offset = m.group(2)
        symbol = os.popen("c++filt %s" %symbol, "r").read().strip()

        rel_pc = addresses[index] - base_address
        index += 1
        self.address_db[rel_pc] = SourceInfo(source, lineno, symbol + "+" + offset)
        continue

      pattern3 = "No line number information available for address"
      m = re.search(pattern3, line)
      if m:
        rel_pc = addresses[index] - base_address
        index += 1
        self.address_db[rel_pc] = SourceInfo("", -1, "0x%x" %rel_pc)
        continue

    if index != len(addresses):
      print >>sys.stderr, "Error in parsing gdb output for", so_path
  
  def loadDebugInfo(self):
    path = findLibraryPath(self.path)
    if not path:
      print >>sys.stderr, "Failed to find the object file:", self.path
      return
    print >>sys.stderr, "Found the object file:", path
    self.parseSourceInfo(path)

class MemoryBlock(object):
  def __init__(self, callid, total, calls):
    self.callid = callid
    self.total = total
    self.calls = calls
    self.callstacks = []

  def addCallstack(self, abs_pc, handle, rel_pc):
    self.callstacks.append((abs_pc, handle, rel_pc))

class Snapshot(object):
  def __init__(self, dmp_file):
    self.mem_db = {}
    if dmp_file:
      self.loadFile(dmp_file)

  def loadFile(self, dmp_file):
    prelogue = "MEMORY SNAPSHOT IS DUMPPING"
    epilogue = "MEMORY SNAPSHOT DUMPPING FINISHED"

    state = kInitState
    global shared_libraries

    fd = open(dmp_file)
    for line in fd:
      line = line.strip()
      if state == kInitState:
        if line == prelogue:
          state = kSoState
          continue
      elif state == kSoState:
        so_pattern = "handle (\d+):\s*(.*)"
        m = re.match(so_pattern, line)
        if m:
          handle = int(m.group(1))
          so_path = m.group(2)
          shared_libraries[handle] = so_path
        else: # so paths is over
          state = kBacktraceState
      elif state == kBacktraceState:
        mem_start_pattern = "Allocated (\d+) bytes in (\d+) calls:\s*(\d+)"
        m = re.match(mem_start_pattern, line)
        if m:
          total = int(m.group(1))
          calls = int(m.group(2))
          callid = int(m.group(3))
          state = kInCallstackState
          self.cur_mem = MemoryBlock(callid, total, calls)
      elif state == kInCallstackState:
        call_pattern = "([0-9A-Fa-f]+):\s*(\d+)\s*\+\s*([0-9A-Fa-f]+)"
        m = re.match(call_pattern, line)
        if m:
          abs_pc = int(m.group(1), 16)
          handle = int(m.group(2))
          rel_pc = int(m.group(3), 16)
          self.cur_mem.addCallstack(abs_pc, handle, rel_pc)
        else:
          self.mem_db[self.cur_mem.callid] = self.cur_mem
          self.cur_mem = None
          state = kBacktraceState

def dumpCallstack(callstacks, debug_info):
  for (abs_pc, handle, rel_pc) in callstacks:
    if not debug_info.has_key(handle):
      print >>sys.stderr, "Error in dumping callstack: No library found"
      sys.exit(-1)
    lib = debug_info[handle]
    src = lib.queryPc(rel_pc)
    if not src:
      src = SourceInfo("", -1, "0x%x" %rel_pc)
    if src.source:
      print "%s!%s(%s:%s)" %(lib.path, \
          src.symbol, src.source, src.line)
    else:
      print "%s!%s" %(lib.path, src.symbol)

class BlockDiff(object):
  def __init__(self, callstacks, block1, block2):
    self.callstacks = callstacks
    self.block1 = block1
    self.block2 = block2
    self.increment = block2.total - block1.total

  def dumpDiff(self, debug_info):
    if self.block2.total == self.block1.total:
      return 0

    print "Allocation: (%d - %d) bytes, (%d - %d) calls" \
        %(self.block2.total, self.block1.total, \
        self.block2.calls, self.block1.calls)
    dumpCallstack(self.callstacks, debug_info)
    print
    return self.block2.total - self.block1.total

def memoryDiff(snapshot1, snapshot2):
  mem1 = [m for m in snapshot1.mem_db.itervalues()]
  mem2 = [m for m in snapshot2.mem_db.itervalues()]
  mem1.sort(key=lambda v: v.callid)
  mem2.sort(key=lambda v: v.callid)

  mem_stats = []

  M = len(mem1)
  N = len(mem2)
  i = 0
  j = 0
  while i < M and j < N:
    block1 = mem1[i]
    block2 = mem2[j]
    if block1.callid < block2.callid:
      mem_stats.append(BlockDiff(block1.callstacks, block1, \
          MemoryBlock(block1.callid, 0, 0)))
      i += 1
    elif block1.callid == block2.callid:
      if block1.total != block2.total:
        mem_stats.append(BlockDiff(block1.callstacks, block1, block2))
      i += 1
      j += 1
    else:
      mem_stats.append(BlockDiff(block2.callstacks, MemoryBlock(block2.callid, \
          0, 0), block2))
      j += 1

  while i < M:
    mem_stats.append(BlockDiff(mem1[i].callstacks, mem1[i], \
        MemoryBlock(mem1[i].callid, 0, 0)))
    i += 1

  while j < N:
    mem_stats.append(BlockDiff(mem2[j].callstacks, MemoryBlock(mem2[j].callid, \
        0, 0), mem2[j]))
    j += 1

  libraries = {}
  for diff in mem_stats:
    if diff.increment == 0:
      continue
    for (abs_pc, handle, rel_pc) in diff.callstacks:
      if not libraries.has_key(handle):
        libraries[handle] = Library(shared_libraries[handle])
      lib = libraries[handle]
      lib.addPc(rel_pc)

  # start to perform time-consuming jobs
  for lib in libraries.itervalues():
    lib.loadDebugInfo()

  mem_stats.sort(key=lambda v: -v.increment)
  increased = 0
  for m in mem_stats:
    increased += m.dumpDiff(libraries)
  print "Total increase", increased, "bytes"

def main():
  parser = argparse.ArgumentParser(description='Memory dump comparation')
  parser.add_argument("--android", action="store_true", default=False)
  parser.add_argument("--solib-path", dest="solib_path", default=".")
  parser.add_argument("snapshot", metavar='SNAPSHOT', type=str, nargs="+", \
      help="list of memory snapshot")

  args = parser.parse_args()
  if len(args.snapshot) > 2:
    print >>sys.stderr, "Too many snapshots"
    sys.exit(-1)

  global g_lib_search_path
  g_lib_search_path = [os.path.expandvars(os.path.expanduser(path)) \
      for path in args.solib_path.split(":")]

  snapshots = []
  for dmp in args.snapshot:
    snapshots.append(Snapshot(dmp))
  if len(snapshots) == 1:
    memoryDiff(Snapshot(""), snapshots[0])
  else:
    memoryDiff(snapshots[0], snapshots[1])

if __name__ == "__main__":
  main()

