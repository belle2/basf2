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
#include <cassert>

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




template <typename CONTAINER_T>
uint64_t to_bit_mask(const CONTAINER_T& container)
{

  uint64_t ret = 0;
  for (let& e : container) {
    assert(e <= 32);
    ret |= (uint64_t(1) << e);
  }
  return ret;





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
auto to_sector_bit_mask(const CONTAINER_T& container, int TriggerCut_, int vetoCut_ = 0)
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








Belle2::group_helper::KLM_trg_summery Belle2::make_trg(const std::vector<Belle2::group_helper::KLM_Digit_compact>& hits,
                                                       int eventNr, int NLayerTrigger)
{



  auto grouped = group<Subdetector, section, isectors_t, sector>::apply(hits,
  [](let & e1)  {
    let  bit_mask     = to_bit_mask(project(e1,  [](let & t) { return (layer(t) + 1) * 2 + plane(t); }));
    let  layer_count_ = countBits(bit_mask);
    return std::tuple(layer_mask(layer_count_), layer_count(bit_mask));
  }
                                                                       );


  sort(grouped);


  let summery2 = group<Subdetector>::apply(grouped,
  [&](let & e1)  {
    return (n_sections_trig) count_if(project<layer_count>(e1), [NLayerTrigger](const auto  & e) {return e >= NLayerTrigger; });
  });


  let n_triggered_sectors2 = group<Subdetector, section, isectors_t>::apply(grouped,
  [&](let & e1)  { return (sector_mask)    to_sector_bit_mask(e1, NLayerTrigger); },
  [&](let & e1)  { return (sector_mask_or) to_sector_bit_mask(e1, NLayerTrigger, NLayerTrigger); }
                                                                           );


  let summery1 = group<Subdetector>::apply(n_triggered_sectors2,
  [](let & e1)  {
    return back2back_t(count_if(project<sector_mask>(e1)) >= c_TotalSections_per_EKLM_BKLM);
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
