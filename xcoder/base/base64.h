#pragma once

#include <string>

namespace agora { namespace commons {

    struct base64 {
        static std::string encode(const std::string& data);
        static std::string decode(const std::string& data);
    };

}
}
