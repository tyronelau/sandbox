#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "base/common/base.h"
#include "base/common/logging.h"
#include "base/file/file_path.h"
#include "base/strings/string_split.h"
#include "base/strings/string_number_conversions.h"
#include "base/graph/topological_sorter.h"
#include "sandbox/liuyong/layout_diff/tinyxml.h"
#include "sandbox/liuyong/layout_diff/dag_sort.h"
#include "sandbox/liuyong/layout_diff/layout_checker.h"

static std::string BeautifyIncludeDir(const char *loc) {
  std::vector<std::string> sources;
  base::SplitString(loc, ":", &sources);
  CHECK_EQ(sources.size() % 2, 0u) << "bad Include loc: " << loc;
  std::ostringstream sout;
  const unsigned level = sources.size() / 2;
  for (unsigned i = 0; i < level; ++i) {
    sout << " " << sources[2*i] << ":" << sources[2*i + 1] << ": ";
    if (i != level - 1) {
      sout << "included from";
    }
    sout << "\n";
  }
  return sout.str();
}

static std::string GetDependencyHierachy(const char *loc) {
  if (!loc) {
    return std::string();
  }

  std::vector<std::string> sources;
  base::SplitStringWithOptions(loc, ":", true, true, &sources);
  std::ostringstream sout;

  sout << "Dependency hierachy: ";
  for (unsigned i = 0; i < sources.size(); ++i) {
    sout << sources[i];
    if (i + 1 < sources.size()) {
      sout << " ---> ";
    }
  }

  return sout.str();
}

std::string GetBaseClasses(TiXmlElement *e) {
  std::ostringstream sout;
  for (TiXmlElement *base = e->FirstChildElement("base"); base != NULL;
       base = base->NextSiblingElement("base")) {
    sout << ((!strcmp(base->Attribute("IsVirtual"), "true")) ? "virtual " : " ")
         << base->Attribute("Name") << ",";
  }
  return sout.str();
}

void GetDepends(TiXmlElement *record, std::vector<std::string> *depends) {
  std::set<std::string> depend_classes;
  for (TiXmlElement *p = record->FirstChildElement(); p != NULL;
       p = p->NextSiblingElement()) {
    if (!strcmp(p->Value(), "base")) {
      depend_classes.insert(p->Attribute("Name"));
    } else if (!strcmp(p->Value(), "field") && p->Attribute("DependantType")) {
      depend_classes.insert(p->Attribute("DependantType"));
    }
  }

  for (auto it = depend_classes.begin(); it != depend_classes.end(); ++it) {
    depends->push_back(*it);
  }
}

namespace {
class LogHelper {
 public:
  explicit LogHelper(const std::string &class_name)
    :class_name_(class_name) {
    LOG(INFO) << "Start to compare class {" << class_name_ << "}";
  }

  ~LogHelper() {
    LOG(INFO) << "End checking the layout of {" << class_name_ << "}";
  }
 private:
  const std::string &class_name_;
};
}

// Prerequisite: the order of the records must be input according to the dependancies.
bool LayoutChecker::CompareTwoRecords(TiXmlElement *e1, TiXmlElement *e2) {
  std::string class_name = e1->Attribute("QualifiedName");
  if (class_name != e2->Attribute("QualifiedName")) {
    class_name = "(" + class_name + ", " + e2->Attribute("QualifiedName") + ")";
  }

  LogHelper log(class_name);

  // compare the base classes.
  TiXmlElement *base1 = e1->FirstChildElement("base");
  TiXmlElement *base2 = e2->FirstChildElement("base");

  for (; base1 != NULL && base2 != NULL; base1 = base1->NextSiblingElement("base"),
       base2 = base2->NextSiblingElement("base")) {
    if (strcmp(base1->Attribute("Name"), base2->Attribute("Name")) ||
        strcmp(base1->Attribute("IsVirtual"), base2->Attribute("IsVirtual"))) {
      break;
    }

    // find out if the layout of the base class is changed.
    // if (layouts[base1->Attribute("Name")]) {
    //  LOG(INFO) << "base class {" << base1->Attribute("Name") << "} layout changed";
    //  break;
  }

  if (base1 != NULL || base2 != NULL) {
    std::ostringstream sout;
    sout << "\n\nstruct/union/class {" << class_name << "} defined in\n"
         << BeautifyIncludeDir(e1->Attribute("IncludeDir")) << "resided in "
         << GetDependencyHierachy(e1->Attribute("SourceLocation"))
         << "\n\nis conflict with the one defined in\n"
         << BeautifyIncludeDir(e2->Attribute("IncludeDir"))
         << "resided in " << GetDependencyHierachy(e2->Attribute("SourceLocation")) << "\n\n";
    sout << "Reason: bases changed. the former has {\033[31m" << GetBaseClasses(e1) << "\033[m\n";
    sout << " while the latter has \033[31m" << GetBaseClasses(e2) << "\033[m\n";
    std::cerr << sout.str();
    return false;
  }

  // compare the fields.
  TiXmlElement *field1 = e1->FirstChildElement("field");
  TiXmlElement *field2 = e2->FirstChildElement("field");

  for (int i = 0; field1 != NULL && field2 != NULL; field1 = field1->NextSiblingElement("field"),
       field2 = field2->NextSiblingElement("field"), ++i) {
    if (strcmp(field1->Attribute("Name"), field2->Attribute("Name"))) {
      std::ostringstream sout;
      sout << "\n\nstruct/union/class {" << class_name << "} defined in\n"
         << BeautifyIncludeDir(e1->Attribute("IncludeDir")) << "resided in "
         << GetDependencyHierachy(e1->Attribute("SourceLocation"))
         << "\n\n is conflict with the one defined in\n"
         << BeautifyIncludeDir(e2->Attribute("IncludeDir"))
         << "resided in " << GetDependencyHierachy(e2->Attribute("SourceLocation")) << "\n\n";
      sout << "Reason: the " << i << "-th class data member is different in name!\n"
          << "the former is \033[31m" << field1->Attribute("Name")
          << "\033[m, while the latter is \033[31m"
          << field2->Attribute("Name") << "\033[m\n";
      std::cerr << sout.str();
      return false;
    }

    // check if the type is changed.
    if (strcmp(field1->Attribute("Type"), field2->Attribute("Type"))) {
      // check if they are both anonymous, if so, fall through into a slow path
      TiXmlElement *a = NULL;
      TiXmlElement *b = NULL;
      if (records_info_.find(field1->Attribute("Type")) != records_info_.end()) {
        a = records_info_[field1->Attribute("Type")];
      }

      if (records_info_.find(field2->Attribute("Type")) != records_info_.end()) {
        b = records_info_[field2->Attribute("Type")];
      }

      bool failed = true;
      if (a && b) {
        bool anony1 = false;
        bool anony2 = false;
        a->QueryBoolAttribute("IsAnonymous", &anony1);
        b->QueryBoolAttribute("IsAnonymous", &anony2);
        if (anony1 && anony2 && CompareTwoRecords(a, b)) {
          failed = false;
        }
      }

      if (failed) {
        std::ostringstream sout;
        sout << "\n\nstruct/union/class {" << class_name << "} defined in\n"
          << BeautifyIncludeDir(e1->Attribute("IncludeDir")) << "resided in "
          << GetDependencyHierachy(e1->Attribute("SourceLocation"))
          << "\n\n is conflict with the one defined in\n"
          << BeautifyIncludeDir(e2->Attribute("IncludeDir")) << "resided in "
          << GetDependencyHierachy(e2->Attribute("SourceLocation")) << "\n\n";
        sout << "Reason: the " << i << "-th class data member is different in type!\n"
          << "the former is \033[31m" << field1->Attribute("Type")
          << "\033[m, while the latter is \033[31m"
          << field2->Attribute("Type") << "\033[m\n";
        std::cerr << sout.str();
        return false;
      }
    }

    // check if the size of the field is changed.
    int offset1, offset2, size1, size2;
    CHECK(base::StringToInt(field1->Attribute("OffsetInBits"), &offset1));
    CHECK(base::StringToInt(field2->Attribute("OffsetInBits"), &offset2));
    CHECK(base::StringToInt(field1->Attribute("SizeInBits"), &size1));
    CHECK(base::StringToInt(field2->Attribute("SizeInBits"), &size2));

    if (offset1 != offset2 || size1 != size2) {
      std::ostringstream sout;
      sout << "\n\nstruct/union/class {" << class_name << "} defined in\n"
         << BeautifyIncludeDir(e1->Attribute("IncludeDir")) << "resided in "
         << GetDependencyHierachy(e1->Attribute("SourceLocation"))
         << "\n\n is conflict with the one defined in\n"
         << BeautifyIncludeDir(e2->Attribute("IncludeDir"))
         << "resided in " << GetDependencyHierachy(e2->Attribute("SourceLocation")) << "\n\n";
      sout << "Reason: the " << i << "-th class data member is different in size!\n"
          << "the former occupies \033[31m[" << offset1 << ", " << offset1 + size1
          << "]\033[m bits, while the latter \033[31m["
          << offset2 << ", " << offset2 + size2 << "]\033[m bits\n";
      std::cerr << sout.str();
      return false;
    }

    // if the type is not a built-in type, check if the record type is changed.
    // if (field1->Attribute("DependantType") && field2->Attribute("DependantType")) {
    //  const std::string &type1 = field1->Attribute("DependantType");
    //  const std::string &type2 = field2->Attribute("DependantType");
    //  if (type1 != type2) {
    //    LOG(INFO) << "Field {" << field1->Attribute("Name") << "}, dependent type is changed";
    //    is_same = false;
    //    break;
    //  }
      // if (layouts[field1->Attribute("DependantType")]) {
      //  LOG(INFO) << "the layout of the type of Field {" << field1->Attribute("Name")
      //            << "} already changed";
      //  is_same = false;
      //  break;
      // }
    // }
  }

  if (field1 != NULL || field2 != NULL) {
    std::ostringstream sout;
    sout << "\n\nstruct/union/class {" << class_name << "} defined in\n"
         << BeautifyIncludeDir(e1->Attribute("IncludeDir")) << "resided in "
         << GetDependencyHierachy(e1->Attribute("SourceLocation"))
         << "\n\n is conflict with the one defined in\n"
         << BeautifyIncludeDir(e2->Attribute("IncludeDir"))
         << "resided in " << GetDependencyHierachy(e2->Attribute("SourceLocation")) << "\n\n";
    if (field1 != NULL) {
      sout << "Reason: the former has \033[31madditional\033[m data members.\n ";
    } else {
      sout << "Reason: the latter has \033[31madditional\033[m data members.\n";
    }
    std::cerr << sout.str();
    return false;
  }
 
  if (strcmp(e1->Attribute("Size"), e2->Attribute("Size"))) {
    std::ostringstream sout;
    sout << "\n\nstruct/union/class {" << class_name << "} defined in\n"
         << BeautifyIncludeDir(e1->Attribute("IncludeDir")) << "resided in "
         << GetDependencyHierachy(e1->Attribute("SourceLocation"))
         << "\n\n is conflict with the one defined in\n"
         << BeautifyIncludeDir(e2->Attribute("IncludeDir"))
         << "resided in " << GetDependencyHierachy(e2->Attribute("SourceLocation")) << "\n";
    sout << "\ndifferent in size: new(" << e1->Attribute("Size")
      << "), old(" << e2->Attribute("Size") << ")" << ", possible reason: "
      << "different alignment by the pragma directive";
    std::cerr << sout.str();
    return false;
  }

  return true;
}

LayoutChecker::LayoutChecker(const std::string &dot_file_path, const std::string &ignore_file_path)
:ignore_list_(ignore_file_path.c_str()), dot_file_(dot_file_path) {
}

LayoutChecker::~LayoutChecker() {
  for (auto it = records_info_.begin(); it != records_info_.end(); ++it) {
    std::vector<TiXmlElement*> &elems = it->second;
    for (auto p = elems.begin(); p != elems.end(); ++p) {
      delete *p;
    }
  }
}

void LayoutChecker::MergeRecursive(std::map<std::string, TiXmlElement*> *out,
                                   TiXmlElement *in, const std::string &source_location) {
  std::map<std::string, TiXmlElement*> &records = *out;
  if (!strcmp(in->Value(), "Record")) {
    const std::string &class_name = in->Attribute("QualifiedName");
    TiXmlElement *e = in->Clone()->ToElement();
    const char *src = e->Attribute("SourceLocation");
    if (!src) {
      src = "";
    }
    e->SetAttribute("SourceLocation", source_location + ":" + src);
    records[class_name] = e;
  }

  for (TiXmlElement *child = in->FirstChildElement(); child != NULL;
       child = child->NextSiblingElement()) {
    MergeRecursive(out, child, source_location);
  }
}



bool LayoutChecker::MergeFrom(const char *in) {
  TiXmlDocument doc;

  if (!doc.LoadFile(in)) {
    PLOG(ERROR) << "Error in opening the file " << in << ", simply ignore it";
    return true;
  }

  std::map<std::string, TiXmlElement*> records;
  TiXmlElement *parent = doc.FirstChildElement();
  if (!parent || (strcmp(parent->Value(), "Library") &&
                  strcmp(parent->Value(), "TranslationUnit"))) {
    // Not a layout file.
    return true;
  }

  MergeRecursive(&records, parent, parent->Attribute("SourceLocation"));

  // Now check the local conflict.
  std::vector<TiXmlElement*> sorted;
  SortDAG(records, &sorted);

  // Now check the global conflict.
  // check the order of dependencies
  for (auto it = sorted.begin(); it != sorted.end(); ++it) {
    const std::string &class_name = (*it)->Attribute("QualifiedName");
    if (ignore_list_.IsIgnoredClass(class_name)) {
      LOG(WARNING) << "class: " << class_name << " be ignored";
      continue;
    } else if (ignore_list_.IsIgnoredPath((*it)->Attribute("IncludeDir"))) {
      LOG(WARNING) << "class " << class_name << " defined in include dirs " <<
          (*it)->Attribute("IncludeDir") << " be ignored";
      continue;
    }

    if (classes_.find(class_name) == classes_.end()) {
      // new class
      std::vector<std::string> depends;
      GetDepends(*it, &depends);
      for (auto p = depends.begin(); p != depends.end(); ++p) {
        depends_.insert(std::make_pair(class_name, *p));
      }
      classes_.insert(class_name);
    } else {
      // existed class
      std::vector<std::string> depends;
      GetDepends(*it, &depends);
      for (auto p = depends.begin(); p != depends.end(); ++p) {
        TiXmlElement *pclass = records[*p];
        assert(pclass != NULL);
        bool is_anonymous = false;
        pclass->QueryBoolAttribute("IsAnonymous", &is_anonymous);
        if (is_anonymous) {
          // if the dependent class is anonymous, ignored. We'll check it
          // entirely afterwards.
          continue;
        }

        if (depends_.find(std::make_pair(class_name, *p)) == depends_.end() ||
            depends_.find(std::make_pair(*p, class_name)) != depends_.end()) {
          const ClassInfo &class_definitions = records_info_[class_name];
          CHECK_GT(class_definitions.size(), 0u);

          CHECK(!CompareTwoRecords(*it, class_definitions[0]));
          OutputInfluencedClasses(class_name);
          LOG(WARNING) << "Incoherence: class/union/struct{" << class_name << "} defined in\n"
              << BeautifyIncludeDir((*it)->Attribute("IncludeDir")) << " is conflict with that in\n"
              << BeautifyIncludeDir(records_info_[class_name][0]->Attribute("IncludeDir"));
          return false;
        }
      }
    }
    auto position = records_info_[class_name].begin();
    records_info_[class_name].insert(position, *it);
  }

  return true;
}

void LayoutChecker::SortDepends(std::vector<std::string> *out) const {
  base::TopologicalSorter<std::string> sorter;
  for (auto it = classes_.begin(); it != classes_.end(); ++it) {
    sorter.AddNode(*it);
  }

  for (auto it = depends_.begin(); it != depends_.end(); ++it) {
    sorter.AddEdge(it->second, it->first);
  }

  bool cyclic = false;
  std::string node;

  while (sorter.GetNext(&node, &cyclic)) {
    out->push_back(node);
  }

  CHECK(!cyclic) << "the dependancies of classes are not acyclic!";
}

namespace {
struct Node {
  std::string name;
  std::vector<Node*> edges;
  enum Color {kWhite, kGray, kBlack};
  Color color;

  Node(): color(kWhite) {}
};

template <class Visitor>
void BFSVisit(Node *start, Visitor visitor) {
  // visit this new fresh node.
  visitor.VisitNode(start);
  for (auto it = start->edges.begin(); it != start->edges.end(); ++it) {
    visitor.VisitEdge(start, *it);
    if ((*it)->color == Node::kWhite) {
      (*it)->color = Node::kGray;
      BFSVisit(*it, visitor);
    }
  }

  start->color = Node::kBlack;
}

class LayoutVisitor {
 public:
  explicit LayoutVisitor(std::ostream *out) :out_(*out) {
  }

  void VisitNode(Node *node) {
    out_ << "\"" << node->name << "\" [shape=box];\n";
  }

  void VisitEdge(Node *begin, Node *end) {
    out_ << "\"" << begin->name << "\" -> \"" << end->name << "\" [color=red];\n";
  }
 private:
  std::ostream &out_;
};
}

void LayoutChecker::OutputInfluencedClasses(
    const std::string &class_name) const {
  std::map<std::string, Node> nodes;
  for (auto it = classes_.begin(); it != classes_.end(); ++it) {
    nodes[*it] = Node();
    nodes[*it].name = *it;
  }

  for (auto it = depends_.begin(); it != depends_.end(); ++it) {
    auto edge = *it;
    Node &p = nodes[edge.second];
    Node &q = nodes[edge.first];
    p.edges.push_back(&q);
  }

  std::ofstream fout(dot_file_);
  fout << "digraph InfluenceGraph {\n";
  BFSVisit(&nodes[class_name], LayoutVisitor(&fout));
  fout << "}\n";
}

bool LayoutChecker::CheckLayouts() const {
  std::vector<std::string> classes;
  SortDepends(&classes);

  for (auto it = classes.begin(); it != classes.end(); ++it) {
    auto cur = records_info_.find(*it);
    if (cur == records_info_.end()) {
      continue;
    }

    const std::vector<TiXmlElement*> &records = cur->second;

    if (records.size() < 2) {
      continue;
    }

    for (auto p = records.begin(); p != records.end(); ++p) {
      for (auto q = p + 1; q != records.end(); ++q) {
        if (!CompareTwoRecords(*p, *q)) {
          LOG(WARNING) << "Incoherence: class/union/struct {" << *it << "} defined in \n"
             << BeautifyIncludeDir((*p)->Attribute("IncludeDir")) << " is conflict with the one in\n"
             << BeautifyIncludeDir((*q)->Attribute("IncludeDir"));
          OutputInfluencedClasses(*it);
          return false;
        }
      }
    }
  }
  return true;
}

bool LayoutChecker::WriteFile(const std::string &out_file) {
  TiXmlDocument doc;
  doc.LinkEndChild(new TiXmlDeclaration("1.0", "utf-8", ""));
  TiXmlElement *root = new TiXmlElement("Library");
  base::FilePath path(out_file);
  root->SetAttribute("SourceLocation", path.value());

  std::map<std::string, TiXmlElement*> records;
  for (auto it = records_info_.begin(); it != records_info_.end(); ++it) {
    records[it->first] = *(it->second.begin());
    it->second.erase(it->second.begin());
  }

  std::vector<TiXmlElement*> dag;
  SortDAG(records, &dag);
  for (auto first = dag.begin(); first != dag.end(); ++first) {
    // (*first)->RemoveAttribute("SourceLocation");
    root->LinkEndChild(*first);
  }
  doc.LinkEndChild(root);

  if (!doc.SaveFile(out_file)) {
    PLOG(ERROR) << "Error in saving the file " << out_file;
    return false;
  }
  return true;
}

