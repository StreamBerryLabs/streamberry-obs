#pragma once

#include "../common.hpp"
#include <vector>
#include <string>

namespace berrystreamcam {

class MdnsScanner {
public:
    MdnsScanner();
    ~MdnsScanner();

    // Scan for Streamberry devices via mDNS/Bonjour
    std::vector<std::string> scan();

private:
    void* mdns_handle_;
};

} // namespace berrystreamcam
