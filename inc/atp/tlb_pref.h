#ifndef TLB_PREF_H
#define TLB_PREF_H

#include "champsim.h"
#include <unordered_set>
#include <list>

static constexpr std::size_t FPQ_SIZE = 16;

class tlb_prefetcher
{
private:
  std::list<champsim::page_number> fake_prefetch_queue;

protected:

public:
  tlb_prefetcher();
  bool check_fake_prefetch_queue(const champsim::page_number& vpn);
  void insert_to_fake_prefetch_queue(const champsim::page_number& vpn, const champsim::address& ip);
  virtual std::list<champsim::page_number> predict(const champsim::page_number& vpn, const champsim::address& ip) = 0;
};

#endif