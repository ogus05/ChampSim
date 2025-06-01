#include "atp/tlb_STP.h"

std::list<champsim::page_number> tlb_STP::predict(const champsim::page_number& vpn, const champsim::address& ip) {
    std::list<champsim::page_number> prefetch_list;
    prefetch_list.push_back(vpn - 2);
    prefetch_list.push_back(vpn - 1);
    prefetch_list.push_back(vpn + 1);
    prefetch_list.push_back(vpn + 2);
    return prefetch_list;
}
