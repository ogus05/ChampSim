#include "atp/tlb_pref.h"
#include <stdint.h>
#include "champsim.h"
#include "address.h"
#include "extent.h"

tlb_prefetcher::tlb_prefetcher() {
  // Initialize the fake prefetch queue and FIFO queue.
  fake_prefetch_queue.clear();
}

bool tlb_prefetcher::check_fake_prefetch_queue(const champsim::page_number& vpn)
{
  for(auto& e : fake_prefetch_queue){
    if(e == vpn){
      return true;
    }
  }
  return false;
}

void tlb_prefetcher::insert_to_fake_prefetch_queue(const champsim::page_number& vpn, const champsim::address& ip) {
  std::list<champsim::page_number> predicted_values = predict(vpn, ip);
  for(auto predicted_value : predicted_values) {
    fake_prefetch_queue.push_front(predicted_value);
  }

  while(fake_prefetch_queue.size() > FPQ_SIZE) {
    fake_prefetch_queue.pop_back();
  }
}

