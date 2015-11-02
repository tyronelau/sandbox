#include <map>
#include <string>
#include <vector>
#include "base/common/logging.h"
#include "base/graph/topological_sorter.h"
#include "sandbox/liuyong/layout_diff/tinyxml.h"

void SortDAG(const std::map<std::string, TiXmlElement*> &records,
             std::vector<TiXmlElement*> *out) {
  base::TopologicalSorter<std::string> sorter;
  for (std::map<std::string, TiXmlElement*>::const_iterator first = records.begin();
       first != records.end(); ++first) {
    sorter.AddNode(first->first);
    TiXmlElement *e = first->second;

    for (TiXmlElement *child = e->FirstChildElement(); child != NULL;
         child = child->NextSiblingElement()) {
      if (!strcmp(child->Value(), "base")) {
        sorter.AddNode(child->Attribute("Name"));
        sorter.AddEdge(child->Attribute("Name"), e->Attribute("QualifiedName"));
      } else if (!strcmp(child->Value(), "field")) {
        if (child->Attribute("DependantType")) {
          sorter.AddNode(child->Attribute("DependantType"));
          sorter.AddEdge(child->Attribute("DependantType"), e->Attribute("QualifiedName"));
        }
      }
    }
  }

  bool cyclic = false;
  std::string node;

  while (sorter.GetNext(&node, &cyclic)) {
    std::map<std::string, TiXmlElement*>::const_iterator cur = records.find(node);
    if (cur != records.end()) {
      out->push_back(cur->second);
    }
  }

  CHECK(!cyclic) << "the dependancies of classes are not acyclic!";
}

