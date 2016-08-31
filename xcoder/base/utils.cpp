#include "utils.h"
#include "log.h"
#include "base64.h"
#include <algorithm>

using namespace agora::base;
using namespace std;

void recycle_bin::drop(uint32_t cid, uint32_t now_second, uint32_t when_to_destroy)
{
    m_[cid] = deadline(now_second, when_to_destroy);
}

recycle_bin::cabinet recycle_bin::destroy(uint32_t now_second)
{
    cabinet to_destroy;
    std::copy_if(m_.begin(), m_.end(), inserter(to_destroy, to_destroy.end()), [=](const pair<uint32_t, deadline>& x) {
        return now_second > x.second.when_to_destroy;
    });
    for (const auto& i: to_destroy) {
        m_.erase(i.first);
    }
    log(INFO_LOG, "by ts %u, %u destroyed, %u left", now_second, to_destroy.size(), m_.size());
    return to_destroy;
}

bool recycle_bin::cancle(uint32_t cid, uint32_t now_second)
{
    (void)now_second;
    size_t size = m_.erase(cid);
    return size != 0;
}

isp_ip_list utils::parse_isp_ip(const string& s)
{
    isp_ip_list isp_ips;
    vector<string> segment = split(s, ';');
    for (const string & x : segment) {
        size_t pos = x.find(':');
        if (pos == string::npos) {
            continue;
        }
        isp_ips.push_back(isp_ip(x.substr(0, pos), inet_addr(x.substr(pos + 1, -1).c_str())));
    }
    return isp_ips;
}

uint64_t utils::now_ms()
{
    return now_us() / 1000;
}
