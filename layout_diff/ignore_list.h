#pragma once
#include <set>
#include <string>

#include "sandbox/liuyong/layout_diff/tinyxml.h"

class IgnoreList {
 public:
  explicit IgnoreList(const char *config_file);
  ~IgnoreList();

  bool LoadFrom(const char *config_file);
  bool IsIgnoredClass(const std::string &class_name) const;
  bool IsIgnoredPath(const std::string &include_dir) const;
 private:
  std::set<std::string> ignore_classes_;
  std::set<std::string> ignore_pathes_;
};

