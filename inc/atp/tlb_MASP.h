#pragma once

#include "channel.h"
#include "atp/tlb_pref.h"


struct table_entry {
    champsim::page_number last_vpn; // 이전 virtual page
    int64_t stride;                 // 이전 stride
};
class tlb_MASP : public tlb_prefetcher {
private:
    constexpr static std::size_t TABLE_SIZE = 64; // Fake Prefetch Queue size
    std::list<std::uintptr_t> lru_queue; // FIFO queue for eviction
    std::unordered_map<std::uintptr_t, table_entry> table;
    
public:

    tlb_MASP() = default;

    virtual std::list<champsim::page_number> predict(const champsim::page_number& vpn, const champsim::address& ip) override;

    void clear();
};