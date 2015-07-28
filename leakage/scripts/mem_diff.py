#! /usr/bin/python
# -*- coding: utf-8 -*-
import sys
import re

kInitState = 0 # initial state, waiting for prelogue
kSoState = 1 # prelogue found, handling the shared libraries
kBacktraceState = 2 # shared libraries processed, waiting for a complete bt
kInCallstackState = 3 # handling callstacks
kEndState = 4 # Memory dump over

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
    # self.dmp_file = dmp_file
    self.mem_db = {}
    self.shared_libraries = {}
    if dmp_file:
      self.loadFile(dmp_file)

  def loadFile(self, dmp_file):
    prelogue = "MEMORY SNAPSHOT IS DUMPPING"
    epilogue = "MEMORY SNAPSHOT DUMPPING FINISHED"

    state = kInitState

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
          self.shared_libraries[handle] = so_path
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
          abs_pc = m.group(1)
          handle = int(m.group(2))
          rel_pc = m.group(3)
          self.cur_mem.addCallstack(abs_pc, handle, rel_pc)
        else:
          self.mem_db[self.cur_mem.callid] = self.cur_mem
          self.cur_mem = None
          state = kBacktraceState

def dumpCallstack(callstacks):
  for callstack in callstacks:
    print "%s: %d + %s" %(callstack[0], callstack[1], callstack[2])

class BlockDiff(object):
  def __init__(self, callstacks, block1, block2):
    self.callstacks = callstacks
    self.block1 = block1
    self.block2 = block2
    self.increment = block2.total - block1.total

  def dumpDiff(self):
    print "Allocation: (%d - %d) bytes, (%d - %d) calls" \
        %(self.block2.total, self.block1.total, \
        self.block2.calls, self.block1.calls)
    dumpCallstack(self.callstacks)
    print

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

  mem_stats.sort(key=lambda v: -v.increment)
  for m in mem_stats:
    m.dumpDiff()

def main():
  if len(sys.argv) == 1 or len(sys.argv) > 3:
    print >>sys.stderr, "usage: python", sys.argv[0], "snapshot1.dmp [snapshot2.dmp]"
    sys.exit(-1)
  snapshots = []
  for dmp in sys.argv[1:]:
    snapshots.append(Snapshot(dmp))
  if len(snapshots) == 1:
    memoryDiff(Snapshot(""), snapshots[0])
  else:
    memoryDiff(snapshots[0], snapshots[1])

if __name__ == "__main__":
  main()

