#pragma once
#include <map>
#include <string>
#include <vector>

class TiXmlElement;

void SortDAG(const std::map<std::string, TiXmlElement*> &records,
             std::vector<TiXmlElement*> *out);

