#include <vector>
#include <iostream>

#include "base/common/logging.h"
#include "base/strings/string_split.h"

#include "sandbox/liuyong/layout_diff/ignore_list.h"
#include "sandbox/liuyong/layout_diff/tinyxml.h"

IgnoreList::IgnoreList(const char *config) {
  CHECK(LoadFrom(config)) << "Failed to load the ignore config";
}

IgnoreList::~IgnoreList() {
}

bool IgnoreList::LoadFrom(const char *config_file) {
  TiXmlDocument doc;
  if (!doc.LoadFile(config_file)) {
    PLOG(ERROR) << "unable to load config: " << config_file;
    return false;
  }

  TiXmlElement *main = doc.FirstChildElement("SkipList");
  if (!main) {
    LOG(ERROR) << "not a config file";
    return true;
  }

  // the ignored classes.
  TiXmlElement *classes = main->FirstChildElement("IgnoreClassList");
  if (classes) {
    for (TiXmlElement *p = classes->FirstChildElement("Class"); p != NULL;
         p = p->NextSiblingElement("Class")) {
      DCHECK(p->Attribute("name")) << "bad format";
      ignore_classes_.insert(p->Attribute("name"));
    }
  }

  TiXmlElement *headers = main->FirstChildElement("IgnoreHeaderList");
  if (headers) {
    for (TiXmlElement *p = headers->FirstChildElement("Header"); p != NULL;
         p = p->NextSiblingElement("Header")) {
      DCHECK(p->Attribute("path")) << "bad header definition";
      ignore_pathes_.insert(p->Attribute("path"));
    }
  }

  return true;
}

bool IgnoreList::IsIgnoredClass(const std::string &class_name) const {
  return ignore_classes_.find(class_name) != ignore_classes_.end();
}

bool IgnoreList::IsIgnoredPath(const std::string &include_dir) const {
  std::vector<std::string> splits;
  base::SplitString(include_dir, ":", &splits);
  DCHECK_EQ(splits.size() % 2u, 0u) << "bade include dir";
  const unsigned count = splits.size() / 2;
  for (unsigned i = 0; i < count; ++i) {
    if (ignore_pathes_.find(splits[2*i]) != ignore_pathes_.end()) {
      return true;
    }
  }
  return false;
}

