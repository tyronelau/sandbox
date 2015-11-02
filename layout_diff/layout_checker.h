#pragma once
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "sandbox/liuyong/layout_diff/ignore_list.h"

class TiXmlElement;

class LayoutChecker {
 public:
  LayoutChecker(const std::string &dot_file_path, const std::string &ignore_file_path);
  ~LayoutChecker();

  bool MergeFrom(const char *in);
  bool CheckLayouts() const;
  bool WriteFile(const std::string &out_file);
 private:
  static void MergeRecursive(std::map<std::string, TiXmlElement*> *out,
                             TiXmlElement *in, const std::string &source);

  void SortDepends(std::vector<std::string> *out) const;
  void OutputInfluencedClasses(const std::string &class_name) const;
  bool CompareTwoRecords(TiXmlElement *e1, TiXmlElement *e2);

  IgnoreList ignore_list_;

  typedef std::vector<TiXmlElement*> ClassInfo;
  std::map<std::string, ClassInfo> records_info_;

  // dependency graph
  std::set<std::string> classes_;
  std::set<std::pair<std::string, std::string> > depends_;

  // output file
  std::string dot_file_;
};

