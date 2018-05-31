#include "udp.h"

const sockaddr_in *udp::Address::ptr() const {
    return address.get();
}
