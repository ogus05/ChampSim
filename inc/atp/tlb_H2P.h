#pragma once

#include "channel.h"
#include "atp/tlb_pref.h"

class tlb_H2P : public tlb_prefetcher {
private:
    champsim::page_number A;
    champsim::page_number B;
protected:
public:
    tlb_H2P();
    virtual std::list<champsim::page_number> predict(const champsim::page_number& vpn, const champsim::address& ip) override;
};