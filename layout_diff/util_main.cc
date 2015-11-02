#include <iostream>

#include "base/common/base.h"
#include "base/common/gflags.h"
#include "base/common/logging.h"

#include "sandbox/liuyong/layout_diff/layout_checker.h"

DEFINE_string(out_file, "", "the file to be stored");
DEFINE_string(dot_file, "influenced_classes.dot", "the hierachy to be stored");
DEFINE_string(ignore_file, "ignores.cfg", "the file in which the ignored classes/pathes are defined");

void PrintUsage(const char *exe) {
  std::cerr << "Usage: " << exe << "[-out_file OUT_FILE] [--dot_file DOT_FILE]"
      " IN_FILE [,IN_FILE]*" << std::endl;
  return;
}

int main(int argc, char *argv[]) {
  base::InitApp(&argc, &argv, "merging the layouts of the libries");

  if (argc == 1) {
    PrintUsage(argv[0]);
    return -1;
  }
  LayoutChecker checker(FLAGS_dot_file, FLAGS_ignore_file);

  for (int i = 1; i < argc; ++i) {
    if (!checker.MergeFrom(argv[i])) {
      return -1;
    }
  }

  if (!checker.CheckLayouts()) {
    return -1;
  }

  if (FLAGS_out_file != "") {
    CHECK(checker.WriteFile(FLAGS_out_file));
  }

  return 0;
}

