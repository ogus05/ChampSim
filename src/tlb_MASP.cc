#include <unordered_map>
#include <list>
#include <cstdint>
#include "champsim.h"
#include "atp/tlb_MASP.h"

std::list<champsim::page_number> tlb_MASP::predict(const champsim::page_number& vpn, const champsim::address& ip){
    std::list<champsim::page_number> prefetch_list;
    std::uintptr_t pc = ip.to<std::uintptr_t>();

    auto it = table.find(pc);
    if (it != table.end()) {
        auto& entry = it->second;

        int64_t new_stride = static_cast<int64_t>(vpn.to<std::uintptr_t>()) -
                                static_cast<int64_t>(entry.last_vpn.to<std::uintptr_t>());

        if(entry.stride != 0){
            prefetch_list.push_back(vpn + entry.stride);
        }
        if(entry.stride != new_stride && new_stride != 0){
            prefetch_list.push_back(vpn + new_stride);
        }

        entry.stride = new_stride;
        entry.last_vpn = vpn;

        // Move the accessed entry to the back of the LRU queue
        lru_queue.remove(pc);
        lru_queue.push_back(pc);
    } else {
        if(table.size() >= TABLE_SIZE) {
            // If the table is full, remove the oldest entry
            auto oldest_entry = lru_queue.front();
            // Remove the entry from the table
            table.erase(oldest_entry);
            // Remove the entry from the LRU queue
            lru_queue.pop_front();
        }
        // insert new entry first
        table.insert({pc, table_entry{vpn, 0}});

        // manage LRU
        lru_queue.push_back(pc);
    }

    return prefetch_list;
}