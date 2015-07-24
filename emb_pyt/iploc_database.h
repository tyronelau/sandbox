#pragma once
#include <cstdint>
#include <map>
#include <string>

namespace extension {
struct GeoInfo {
  uint32_t ip_start;
  uint32_t ip_end;

  std::string country_name;
  std::string country_abbr;
  std::string province;
  std::string city;
  double lattitude;
  double longtitude;
};

class IpLocDatabase {
 public:
  IpLocDatabase();
  ~IpLocDatabase();

  bool open_database(const char *db_path);
  void close_database();
  bool find_location(uint32_t ip, GeoInfo *info) const;
 private:
  std::map<uint32_t, GeoInfo> locations_; 
};
}

