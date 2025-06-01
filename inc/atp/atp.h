#pragma once

#include <cstdint>
#include <unordered_map>
#include <list>
#include <vector>

#include "champsim.h"
#include "modules.h"
#include "atp/tlb_pref.h"
#include "atp/tlb_STP.h"
#include "atp/tlb_H2P.h"
#include "atp/tlb_MASP.h"

static constexpr std::size_t SPQ_SIZE = 64;
static constexpr uint8_t ATP_C0_MAX = 255; // 6 bits
static constexpr uint8_t ATP_C1_MAX = 63; // 4 bits
static constexpr uint8_t ATP_C2_MAX = 3;  // 2 bits
static constexpr uint32_t ALLOWED_PREF_ON_THE_FLY = 64;

// 0 for no ATP context.
// 1 for C0, C1, C2.
// 2 for Free distance table - not implemented yet.
// 3 for Shared prefetch queue.
// 4 for Fake prefetch queue.
static constexpr uint8_t ATP_CONTEXT = 0;

class out_file{
  static int32_t last_checked_cycle;
  static std::vector<std::pair<uint64_t, uint64_t>> PQ_hit_ratio_T;
  static std::vector<std::pair<uint64_t, uint64_t>> useless_PQ_entries_T;
  static std::vector<std::pair<uint64_t, uint64_t>> TLB_Miss_rate_T;
  static std::vector<double> IPC_T;
  
  static int64_t last_checkpoint_instr;
  static int64_t last_checkpoint_cycle;
  public:
  static std::pair<uint64_t, uint64_t> PQ_hit_ratio;
  static std::pair<uint64_t, uint64_t> useless_PQ_entries;
  static std::pair<uint64_t, uint64_t> TLB_Miss_rate;
  static double IPC;
  
  static const int32_t CYCLES_PER_LOG;
  static const int64_t SWITCH_REPEAT_COUNT;
  static int64_t log_count;
  static int64_t log_clear_count;
  
  static void handle_clock(int64_t current_cycle);
  static void handle_switch();
  static void set_IPC(int64_t num_retired, int64_t now_cycle);
  static bool handle_log(uint64_t num_of_process);
};



struct branch_info{
  uint16_t C0;
  uint16_t C1;
  uint16_t C2;
  branch_info() : C0(0), C1(0), C2(0) {}
};

struct atp_context
{
  std::shared_ptr<branch_info> branch;
  std::shared_ptr<std::array<std::unique_ptr<tlb_prefetcher>, 3>> tlb_prefetchers; // 0: STP, 1: H2P, 2: MASP

  std::shared_ptr<std::list<std::pair<champsim::page_number, champsim::page_number>>> shared_prefetch_queue;
  
  atp_context();

  void make_context(const atp_context* other);
};

class atp
{
private:
  std::vector<atp_context> context_list;
  atp_context* current_context;

  void insert_to_shared_prefetch_queue(const champsim::page_number& vpn, const champsim::page_number& ppn);
  void modify_decision_tree(const champsim::page_number& vpn_miss);
  tlb_prefetcher* get_tlb_prefetcher();

public:
  atp();
  // executed when the pagewalk reponses.
  void handle_fill(const champsim::page_number& vpn, const champsim::page_number& ppn);

  // executed when the page walk is happening.
  std::list<champsim::page_number> handle_miss(const champsim::page_number& miss_vpn, const champsim::address& ip);

  bool has_pte(const champsim::page_number& vpn);

  champsim::page_number pop_pte(const champsim::page_number& vpn);

  void store_and_fill_context(uint32_t process_number);
};
