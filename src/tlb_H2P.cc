#include "atp/tlb_H2P.h"

tlb_H2P::tlb_H2P() {
    A = champsim::page_number(0); B = champsim::page_number(0);
}

std::list<champsim::page_number> tlb_H2P::predict(const champsim::page_number& vpn, const champsim::address& ip)
{
    std::list<champsim::page_number> prefetch_list;

    if (A != champsim::page_number(0)) {
        int64_t d_E_B = champsim::offset(B, vpn);
        int64_t d_B_A = champsim::offset(A, B);

        if (d_E_B != 0)
            prefetch_list.push_back(vpn + d_E_B);
        if (d_B_A != 0 && d_B_A != d_E_B)
            prefetch_list.push_back(vpn + d_B_A);
    }
    A = B;
    B = vpn;


    return prefetch_list;
}
