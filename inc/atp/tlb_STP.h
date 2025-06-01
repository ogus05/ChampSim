#pragma once

#include "channel.h"
#include "atp/tlb_pref.h"

class tlb_STP : public tlb_prefetcher {
private:
protected:
public:
  tlb_STP() = default;
  ~tlb_STP() = default;
  virtual std::list<champsim::page_number> predict(const champsim::page_number& vpn, const champsim::address& ip) override;
};