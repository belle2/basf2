/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/



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




AXIS_NAME(layer_count, int);
AXIS_NAME(layer_mask, uint64_t);
AXIS_NAME(n_triggered, int);
AXIS_NAME(sector_mask, int);
AXIS_NAME(sector_mask_or, int);

AXIS_NAME(n_sections_trig, int);
AXIS_NAME(back2back_t, int);

AXIS_NAME(TriggerCut, int);
AXIS_NAME(vetoCut, int);


using namespace Belle2;
using namespace Belle2::group_helper::KLM_Coordinates_n;
using namespace Belle2::group_helper::KLM_Generic;
using namespace Belle2::group_helper;



//! Total number of sections
const int c_TotalSections_per_EKLM_BKLM = 2;
const int c_MaxSectorID = 7;



const int c_TotalLayers = 15;


constexpr  Subdetector c_BKLM = Subdetector(KLMElementNumbers::c_BKLM);
constexpr  Subdetector c_EKLM = Subdetector(KLMElementNumbers::c_EKLM);


constexpr  isectors_t c_backward_eklm = isectors_t(KLM_TRG_definitions::c_backward_eklm);
constexpr  isectors_t c_backward_bklm = isectors_t(KLM_TRG_definitions::c_backward_bklm);

constexpr  isectors_t c_forward_bklm = isectors_t(KLM_TRG_definitions::c_forward_bklm);
constexpr  isectors_t c_forward_eklm = isectors_t(KLM_TRG_definitions::c_forward_eklm);


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
auto to_sector_bit_mask(const CONTAINER_T& container, TriggerCut TriggerCut_, vetoCut vetoCut_ = vetoCut(0))
{
  int ret = 0;
  auto back = container.back();
  for (const auto& e : container) {
    int bitcount = countBits(layer_mask(e));
    int bitcount_or = countBits(layer_mask(back) | layer_mask(e));
    int bitcount_back = countBits(layer_mask(back));
    if (bitcount >= TriggerCut_) {
      ret |= (1 << sector(e));
    } else if (
      bitcount_or >= TriggerCut_
      && bitcount_back < vetoCut_
      && bitcount < vetoCut_
      && (sectors_adjacent(sector(e), sector(back)))
    ) {
      ret |= (1 << sector(e));
      ret |= (1024);
    }
    back = e;
  }
  return ret;
}




struct to_plane_layer {

  template <typename Tuple_T>
  constexpr explicit to_plane_layer(const Tuple_T& t) :
    m_data((layer(t) + 1) * 2 + plane(t)) { }

  constexpr operator uint64_t() const
  {
    return m_data;
  }

  uint64_t m_data;
};




Belle2::group_helper::KLM_trg_summery Belle2::make_trg(const std::vector<Belle2::group_helper::KLM_Digit_compact>& hits,
                                                       int eventNr, int NLayerTrigger)
{



  auto grouped = group<Subdetector, section, isectors_t, sector>::apply(hits,
  [](const auto & e1)  {
    const auto  bit_mask     = layer_mask(to_bit_mask<to_plane_layer>(e1));
    const auto  layer_count_ = layer_count(countBits(bit_mask));
    return std::tuple(layer_count_,   bit_mask);
  }
                                                                       );


  sort(grouped);


  auto summery2 = group<Subdetector>::apply(grouped,
  [&](const auto & e1) -> n_sections_trig {
    return (n_sections_trig) count_if(e1, group_helper::greater_equal<int>{NLayerTrigger  }, layer_count());
  });


  auto n_triggered_sectors2 = group<Subdetector, section, isectors_t>::apply(grouped,
                              [&](const auto & e1) -> sector_mask    { return (sector_mask)    to_sector_bit_mask(e1, TriggerCut(NLayerTrigger)); },
                              [&](const auto & e1) -> sector_mask_or { return (sector_mask_or) to_sector_bit_mask(e1, TriggerCut(NLayerTrigger), vetoCut(NLayerTrigger)); }
                                                                            );


  auto summery1 = group<Subdetector>::apply(n_triggered_sectors2,
  [](const auto & e1) -> back2back_t {
    return back2back_t(count_if(e1, group_helper::greater<int>{0}, sector_mask()) >= c_TotalSections_per_EKLM_BKLM);
  }
                                           );



  return std::make_tuple(
           event_nr(eventNr),
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
