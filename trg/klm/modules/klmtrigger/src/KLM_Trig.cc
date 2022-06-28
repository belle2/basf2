


#include <unordered_map>
#include <algorithm>
#include <numeric>
#include <vector>
#include <tuple>
#include <iostream>
#include <bitset>


#include "trg/klm/modules/klmtrigger/group_helper.h"
#include "trg/klm/modules/klmtrigger/KLMAxis.h"
#include "trg/klm/modules/klmtrigger/KLM_Trig.h"
#include "klm/dataobjects/KLMElementNumbers.h"


using namespace Belle2;


//! Total number of sections
const int c_TotalSections_per_EKLM_BKLM = 2;
const int c_MaxSectorID = 7;



const int c_TotalLayers = 15;


constexpr  KLM_type c_BKLM = KLMElementNumbers::c_BKLM;
constexpr  KLM_type c_EKLM = KLMElementNumbers::c_EKLM;


constexpr  isectors_t c_backward_eklm = KLM_TRG_definitions::c_backward_eklm;
constexpr  isectors_t c_backward_bklm = KLM_TRG_definitions::c_backward_bklm;

constexpr  isectors_t c_forward_bklm  = KLM_TRG_definitions::c_forward_bklm;
constexpr  isectors_t c_forward_eklm = KLM_TRG_definitions::c_forward_eklm;


std::size_t countBits(uint64_t n)
{
  return std::bitset<64>(n).count();
}




template <typename AXIS_NAME_T, typename CONTAINER_T>
uint64_t to_bit_mask(const CONTAINER_T& container)
{

  return std::accumulate(container.begin(), container.end(), uint64_t(0),
  [](const auto & lhs, const auto & rhs) {
    const auto bitshift = uint64_t(AXIS_NAME_T(rhs));
    if (bitshift > 32) {
      throw std::runtime_error("from:\nuint64_t to_bit_mask(const CONTAINER_T& container)\ninput number to large.\n\n");
    }
    return lhs | (uint64_t(1) << bitshift);
  });



}

bool sectors_adjacent(int e1, int e2)
{
  if (e1 == 0 && e2 == c_MaxSectorID) {
    return true;
  }
  if (e1 - e2 == 1) {
    return true;
  }
  return false;
}


template <typename CONTAINER_T>
auto to_sector_bit_mask(const CONTAINER_T& container, TriggerCut TriggerCut_, vetoCut vetoCut_ = 0)
{
  int ret = 0;
  auto back = container.back();
  for (const auto& e : container) {
    int bitcount = countBits(layer_mask(e));
    int bitcount_or = countBits(layer_mask(back) | layer_mask(e));
    int bitcount_back = countBits(layer_mask(back));
    if (bitcount >= TriggerCut_) {
      ret |= (1 << sector_t(e));
    } else if (
      bitcount_or >= TriggerCut_
      && bitcount_back < vetoCut_
      && bitcount < vetoCut_
      && (sectors_adjacent(sector_t(e), sector_t(back)))
    ) {
      ret |= (1 << sector_t(e));
      ret |= (1024);
    }
    back = e;
  }
  return ret;
}







Belle2::KLM_trg_summery Belle2::make_trg(std::vector<KLM_Digit_compact>& hits, int eventNr, int NLayerTrigger,
                                         const std::vector<int>& layersUsed)
{




  sort(hits);
  //drop_duplicates(hits);

  erase_remove_if(hits,
  [&layersUsed](const auto & ele) mutable {
    return contains(layersUsed, [&](const auto & e1) { return e1 == layer_t(ele); });
  }
                 );

  for (size_t i = 0; i < hits.size(); ++i) {
    std::get<layer_t>(hits[i]) = (layer_t(hits[i]) + 1) * 2 + plane_t(hits[i]);
  }



  auto grouped = group<KLM_type, section_t, isectors_t, sector_t>::apply(hits,
                 [](const auto & e1) -> layer_count { return countBits(to_bit_mask<layer_t>(e1)); },
                 [](const auto & e1) -> layer_mask { return to_bit_mask<layer_t>(e1); }
                                                                        );


  sort(grouped);


  auto summery2 = group<KLM_type>::apply(grouped,
  [&](const auto & e1) -> n_sections_trig {
    return count_if(e1, group_helper::greater_equal<int>{NLayerTrigger  }, layer_count());
  });


  auto n_triggered_sectors2 = group<KLM_type, section_t, isectors_t>::apply(grouped,
                              [&](const auto & e1) -> sector_mask { return to_sector_bit_mask(e1, TriggerCut(NLayerTrigger)); },
                              [&](const auto & e1) -> sector_mask_or { return to_sector_bit_mask(e1, TriggerCut(NLayerTrigger), vetoCut(NLayerTrigger)); }
                                                                           );


  auto summery1 = group<KLM_type>::apply(n_triggered_sectors2,
  [](const auto & e1) -> back2back_t {
    return  count_if(e1, group_helper::greater<int>{0}, sector_mask()) >= c_TotalSections_per_EKLM_BKLM;
  }
                                        );



  return std::make_tuple(
           KLM_digit_n::event_nr(eventNr),
           BKLM_n_trg_sectors(n_sections_trig(get_first(summery2, c_BKLM))),
           EKLM_n_trg_sectors(n_sections_trig(get_first(summery2, c_EKLM))),


           Sector_mask_Backward_Barrel(sector_mask(get_first(n_triggered_sectors2, c_backward_bklm))),
           Sector_mask_Forward_Barrel(sector_mask(get_first(n_triggered_sectors2, c_forward_bklm))),
           Sector_mask_Backward_Endcap(sector_mask(get_first(n_triggered_sectors2, c_backward_eklm))),
           Sector_mask_Forward_Endcap(sector_mask(get_first(n_triggered_sectors2, c_forward_eklm))),


           Sector_mask_OR_Backward_Barrel(sector_mask_or(get_first(n_triggered_sectors2, c_backward_bklm))),
           Sector_mask_OR_Forward_Barrel(sector_mask_or(get_first(n_triggered_sectors2, c_forward_bklm))),
           Sector_mask_OR_Backward_Endcap(sector_mask_or(get_first(n_triggered_sectors2, c_backward_eklm))),
           Sector_mask_OR_Forward_Endcap(sector_mask_or(get_first(n_triggered_sectors2, c_forward_eklm))),


           BKLM_back_to_back_flag(back2back_t(get_first(summery1, c_BKLM))),
           EKLM_back_to_back_flag(back2back_t(get_first(summery1, c_EKLM)))

         );




}