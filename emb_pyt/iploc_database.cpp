#include <utility>

#include "iploc_database.h"

namespace extension {
IpLocDatabase::IpLocDatabase() {
}

IpLocDatabase::~IpLocDatabase() {
}

bool IpLocDatabase::open_database(const char *db_path) {
  uint32_t ip_start;
  uint32_t ip_end;
  char abbr[64];
  char country[64];
  char state[64];
  char city[64];
  double latitude;
  double longtitude;
  char postcode[64];
  char timezone[64];

  char buf[512];
  char *line = buf;
  size_t n = 512;
  FILE *fp = fopen(db_path, "r");
  if (!fp)
    return false;

  uint32_t total = 0, succeeded = 0;

  while (getline(&line, &n, fp) != EOF) {
    ++total;

    if (sscanf(buf, "%u %u %63s %63s %63s %63s %lf %lf %63s %63s",
        &ip_start, &ip_end, abbr, country, state, city, &latitude,
        &longtitude, postcode, timezone) != 10)
      continue;

    ++succeeded;
    GeoInfo info;
    info.ip_start = ip_start;
    info.ip_end = ip_end;
    info.country_name = country;
    info.country_abbr = abbr;
    info.province = state;
    info.city = city;
    info.lattitude = latitude;
    info.longtitude = longtitude;

    locations_.insert(std::make_pair(ip_end, info));
  }

  fclose(fp);

  printf("Total: %u, Succeeded: %u\n", total, succeeded);
  return true;
}

void IpLocDatabase::close_database() {
  locations_.clear();
}

bool IpLocDatabase::find_location(uint32_t ip, GeoInfo *info) const {
  auto it = locations_.lower_bound(ip);
  if (it == locations_.end() || it->second.ip_start > ip)
    return false;
  
  *info = it->second;
  return true;
}
}

