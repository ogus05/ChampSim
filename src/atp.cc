#include "atp/atp.h"

#include "cache.h" // For prefetch_line()
#include "atp/atp.h"

int32_t out_file::last_checked_cycle = 0;
std::vector<std::pair<uint64_t, uint64_t>> out_file::PQ_hit_ratio_T{};
std::vector<std::pair<uint64_t, uint64_t>> out_file::useless_PQ_entries_T{};
std::vector<std::pair<uint64_t, uint64_t>> out_file::TLB_Miss_rate_T{};
std::vector<double> out_file::IPC_T{};

int64_t out_file::last_checkpoint_instr = 0;
int64_t out_file::last_checkpoint_cycle = 0;
std::pair<uint64_t, uint64_t> out_file::PQ_hit_ratio = {0, 0};
std::pair<uint64_t, uint64_t> out_file::useless_PQ_entries = {0, 0};
std::pair<uint64_t, uint64_t> out_file::TLB_Miss_rate = {0, 0};
double out_file::IPC = 0;

const int32_t out_file::CYCLES_PER_LOG = 1e4;
const int64_t out_file::SWITCH_REPEAT_COUNT = 2;
int64_t out_file::log_count = 0;
int64_t out_file::log_clear_count = 0;

atp_context::atp_context()
{
  tlb_prefetchers = std::make_shared<std::array<std::unique_ptr<tlb_prefetcher>, 3>>();
  (*tlb_prefetchers)[0] = std::make_unique<tlb_STP>();
  (*tlb_prefetchers)[1] = std::make_unique<tlb_H2P>();
  (*tlb_prefetchers)[2] = std::make_unique<tlb_MASP>();
  shared_prefetch_queue = std::make_shared<std::list<std::pair<champsim::page_number, champsim::page_number>>>();
  branch = std::make_shared<branch_info>();
}

void atp_context::make_context(const atp_context* other)
{
  if(other->tlb_prefetchers == nullptr){
    abort();
  }
  tlb_prefetchers = other->tlb_prefetchers;
  shared_prefetch_queue = other->shared_prefetch_queue;
  branch = other->branch;
  switch (4){
    case 4:
      tlb_prefetchers = std::make_shared<std::array<std::unique_ptr<tlb_prefetcher>, 3>>();
      (*tlb_prefetchers)[0] = std::make_unique<tlb_STP>();
      (*tlb_prefetchers)[1] = std::make_unique<tlb_H2P>();
      (*tlb_prefetchers)[2] = std::make_unique<tlb_MASP>();
    case 3:
      shared_prefetch_queue = std::make_shared<std::list<std::pair<champsim::page_number, champsim::page_number>>>();
    case 2:
      // Free distance table - not implemented yet.
    case 1:
      branch = std::make_shared<branch_info>();
    case 0:
      break;
    default:
      throw std::runtime_error("Invalid ATP_CONTEXT value");
  };
}


atp::atp()
{
  context_list.reserve(256);
  context_list.push_back(atp_context());
  current_context = &context_list.back();
}

void atp::insert_to_shared_prefetch_queue(const champsim::page_number& addr, const champsim::page_number& pte)
{
  fmt::print("FILL!! {} {} \n", addr, pte);
  auto it = current_context->shared_prefetch_queue->begin();
  for(; it != current_context->shared_prefetch_queue->end(); it++) {
    if (it->first == addr) {
      break; // 주소가 이미 존재하는 경우
    }
  }
  if(it == current_context->shared_prefetch_queue->end()) {
    out_file::useless_PQ_entries.first += 1;
    current_context->shared_prefetch_queue->push_front(std::make_pair(addr, pte));
  } else{
    current_context->shared_prefetch_queue->erase(it); // 이미 존재하는 경우, 해당 entry를 삭제
    current_context->shared_prefetch_queue->push_front(std::make_pair(it->first, it->second)); // 새로운 entry를 삽입
  }
  while(current_context->shared_prefetch_queue->size() > SPQ_SIZE){
    out_file::useless_PQ_entries.second += 1;
    current_context->shared_prefetch_queue->pop_back();
  }
}

void atp::modify_decision_tree(const champsim::page_number& v_miss_addr)
{
  bool P0_hit = current_context->tlb_prefetchers->at(0)->check_fake_prefetch_queue(v_miss_addr);
  bool P1_hit = current_context->tlb_prefetchers->at(1)->check_fake_prefetch_queue(v_miss_addr);
  bool P2_hit = current_context->tlb_prefetchers->at(2)->check_fake_prefetch_queue(v_miss_addr);
  if (!P0_hit && !P1_hit && !P2_hit) {
    current_context->branch->C0 = current_context->branch->C0 > 0 ? 
      static_cast<uint16_t>(current_context->branch->C0 - 1) : 0;
  } else {
    current_context->branch->C0 = current_context->branch->C0 < ATP_C0_MAX ? 
      static_cast<uint16_t>(current_context->branch->C0 + 1) : ATP_C0_MAX;
    if(!P0_hit){
      current_context->branch->C1 = current_context->branch->C1 > 0 ? 
       static_cast<uint16_t>(current_context->branch->C1 - 1) : 0;
      if(!P1_hit && P2_hit){
        current_context->branch->C2 = current_context->branch->C2 < ATP_C2_MAX ? 
         static_cast<uint16_t>(current_context->branch->C2 + 1) : ATP_C2_MAX;
      } else if(P1_hit && !P2_hit){
        current_context->branch->C2 = current_context->branch->C2 > 0 ? 
         static_cast<uint16_t>(current_context->branch->C2 - 1) : 0;
      }
    } else{
      if(!P1_hit && !P2_hit){
        current_context->branch->C1 = current_context->branch->C1 < ATP_C1_MAX ? 
         static_cast<uint16_t>(current_context->branch->C1 + 1) : ATP_C1_MAX;
      } else if(!P1_hit && P2_hit){
        current_context->branch->C2 = current_context->branch->C2 < ATP_C2_MAX ? 
         static_cast<uint16_t>(current_context->branch->C2 + 1) : ATP_C2_MAX;
      } else if(P1_hit && !P2_hit){
        current_context->branch->C2 = current_context->branch->C2 > 0 ? 
         static_cast<uint16_t>(current_context->branch->C2 - 1) : 0;
      }
    }
  }
}

tlb_prefetcher* atp::get_tlb_prefetcher() {
  if(current_context->branch->C0 > (ATP_C0_MAX / 2) + 1 ){
    if(current_context->branch->C1 > (ATP_C1_MAX / 2) + 1){
      fmt::print("[ATP] Selected STP TLB Prefetcher\n");
      return current_context->tlb_prefetchers->at(0).get();
    } else{
      if(current_context->branch->C2 < (ATP_C2_MAX / 2)){
        fmt::print("[ATP] Selected H2P TLB Prefetcher\n");
        return current_context->tlb_prefetchers->at(1).get();
      } else{
        fmt::print("[ATP] Selected MASP TLB Prefetcher\n");
        return current_context->tlb_prefetchers->at(2).get();
      }
    }
  } else{
    fmt::print("[ATP] No TLB Prefetcher selected, returning nullptr\n");
    fmt::print("[ATP] C0: {}, C1: {}, C2: {}\n", 
             static_cast<int>(current_context->branch->C0), 
             static_cast<int>(current_context->branch->C1), 
             static_cast<int>(current_context->branch->C2));
    return nullptr;
  }
}

void atp::handle_fill(const champsim::page_number& vpn, const champsim::page_number& ppn)
{
  insert_to_shared_prefetch_queue(vpn, ppn);
}

std::list<champsim::page_number> atp::handle_miss(const champsim::page_number& miss_vpn, const champsim::address& ip)
{
  std::list<champsim::page_number> predicted_list;
  modify_decision_tree(miss_vpn);

  for(auto& prefetcher_it : *current_context->tlb_prefetchers.get()){
    prefetcher_it->insert_to_fake_prefetch_queue(miss_vpn, ip);
  }

  tlb_prefetcher* selected_prefetcher = get_tlb_prefetcher();
  if(selected_prefetcher != nullptr) {
    predicted_list = selected_prefetcher->predict(miss_vpn, ip);
  }
  return predicted_list;
}

bool atp::has_pte(const champsim::page_number& vpn) {
  for(auto entry = current_context->shared_prefetch_queue->begin(); entry != current_context->shared_prefetch_queue->end(); entry++){
    if(entry->first == vpn){
      out_file::PQ_hit_ratio.first += 1;
      return true;
    }
  }
  out_file::PQ_hit_ratio.second += 1;
  return false;
}

champsim::page_number atp::pop_pte(const champsim::page_number& vpn) {
  for(auto entry = current_context->shared_prefetch_queue->begin(); entry != current_context->shared_prefetch_queue->end(); entry++){
    if(entry->first == vpn){
      champsim::page_number pte = champsim::page_number(entry->second);
      current_context->shared_prefetch_queue->erase(entry); // Remove the entry from the map
      return pte; // Return the PTE found
    }
  }
  assert(false && "PTE not found in shared_prefetch_queue");
}

void atp::store_and_fill_context(uint32_t process_number) {
  if(ATP_CONTEXT == 0){
    current_context = new atp_context();
    current_context->branch->C0 = context_list[process_number].branch->C0;
    current_context->branch->C1 = context_list[process_number].branch->C1;
    current_context->branch->C2 = context_list[process_number].branch->C2;
  } else{
    if (process_number >= context_list.size()) {
      context_list.resize(process_number + 1);
      context_list[process_number].make_context(current_context);
    }
    current_context = &context_list[process_number];
  }
  out_file::handle_switch();
}

void out_file::handle_clock(int64_t current_cycle) {
  if(current_cycle - last_checked_cycle > CYCLES_PER_LOG){
    last_checked_cycle = current_cycle;

    PQ_hit_ratio_T.push_back(PQ_hit_ratio);
    useless_PQ_entries_T.push_back(useless_PQ_entries);
    TLB_Miss_rate_T.push_back(TLB_Miss_rate);
    IPC_T.push_back(IPC);

    PQ_hit_ratio = {0, 0};
    useless_PQ_entries = {0, 0};
    TLB_Miss_rate = {0, 0};
    IPC = -1;
  }
}

void out_file::handle_switch() {
  
  log_clear_count += 1;
  PQ_hit_ratio_T.clear();
  useless_PQ_entries_T.clear();
  TLB_Miss_rate_T.clear();
  IPC_T.clear();
}

void out_file::set_IPC(int64_t num_retired, int64_t now_cycle) {
  if(IPC < -1e-9){
    std::int64_t interval_instr = num_retired - last_checkpoint_instr;
    std::int64_t interval_cycle = now_cycle - last_checkpoint_cycle;

    IPC = static_cast<double>(interval_instr) / static_cast<double>(interval_cycle);

    last_checkpoint_cycle = now_cycle;
    last_checkpoint_instr = num_retired;
  }
}

static int i = 0;
bool out_file::handle_log(uint64_t num_of_process) {
  if(log_clear_count > SWITCH_REPEAT_COUNT - 1){
    std::ofstream PQ_hit_ratio_file("result/PQ_hit" + std::to_string(i));
    std::ofstream useless_PQ_entries_file("result/useless_PQ" + std::to_string(i));
    std::ofstream TLB_Miss_file("result/TLB_Miss" + std::to_string(i));
    std::ofstream IPC_file("result/IPC" + std::to_string(i));
    i++;
    
    for(auto& e : PQ_hit_ratio_T){
      PQ_hit_ratio_file << e.first << "\t" << e.second << std::endl;
    }
    
    for(auto& e : useless_PQ_entries_T){
      useless_PQ_entries_file << e.first << "\t" << e.second << std::endl;
    }
    
    for(auto& e : TLB_Miss_rate_T){
      TLB_Miss_file << e.first << "\t" << e.second << std::endl;
    }
    
    for(auto& e : IPC_T){
      IPC_file << e << std::endl;
    }
    PQ_hit_ratio_file.close();
    useless_PQ_entries_file.close();
    TLB_Miss_file.close();
    IPC_file.close();
    PQ_hit_ratio_T.clear();
    useless_PQ_entries_T.clear();
    TLB_Miss_rate_T.clear();
    IPC_T.clear();
    log_count += 1;
    if(log_count >= num_of_process){
      return true;
    }
  }
  return false;
}
