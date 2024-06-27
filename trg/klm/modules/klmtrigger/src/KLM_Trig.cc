/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#include "trg/klm/modules/klmtrigger/ntuples_full.h"


#include "trg/klm/modules/klmtrigger/KLM_Trig.h"
#include "trg/klm/modules/klmtrigger/IO_csv.h"
#include "trg/klm/modules/klmtrigger/bit_operations.h"


#include <unordered_map>
#include <algorithm>
#include <numeric>
#include <vector>
#include <tuple>
#include <iostream>
#include <bitset>

using namespace Belle2::KLM_TRG_definitions;






namespace Belle2 {

  void klmtrg_layer_counter_t::clear_layersUsed()
  {
    m_layersUsed.clear();
  }

  void klmtrg_layer_counter_t::add_layersUsed(int layersUsed)
  {
    m_layersUsed.emplace_back(layersUsed);
  }

  void klmtrg_layer_counter_t::set_NLayerTrigger(int NlayerTrigger)
  {
    m_NlayerTrigger = NlayerTrigger;
  }

  void klmtrg_layer_counter_t::run(const KLM_Digit_compact_ts& hits)
  {


    m_sections_trig.clear();
    m_summary1.clear();

    auto hits1 = nt::algorithms::join_vectors(
                   hits,
                   m_layersUsed,
                   nt::comparators::on_common_args,
    [&](auto e1, const auto&) {
      e1.layer = e1.layer * 2 + e1.plane;
      return e1;
    }
                 );

    __CSV__WRITE__(hits1);


    auto grouped = nt_group(
                     hits1[0].subdetector,
                     hits1[0].section,
                     hits1[0].sector
                   ).apply_append(
                     hits,
    [](const auto & e1) {
      auto bit_mask = to_bit_mask<layer>(e1);
      return nt::ntuple(
               ax_maker(layer_count) = countBits(bit_mask),
               ax_maker(layer_mask) = bit_mask
             );
    });


    __CSV__WRITE__(grouped);

    nt::algorithms::filter(grouped, [&](const auto & e) { return e.layer_count >= m_NlayerTrigger;  });





    m_sections_trig = nt_group(
                        grouped[0].subdetector,
                        grouped[0].section
                      ).apply_append(
                        grouped,
    [&](const auto & e1) {
      auto bit_mask = to_bit_mask<sector>(e1);
      n_sections_trig  nsector = countBits(bit_mask);
      return  nt::ntuple(
                sector_mask(bit_mask),
                nsector
              );
    }
                      );

    __CSV__WRITE__(m_sections_trig);



    m_summary1 = nt_group(
                   m_sections_trig[0].subdetector
                 ).apply_append(
                   m_sections_trig,
    [](const auto & e1) {
      auto bit_mask = countBits(to_bit_mask<section>(e1));
      n_sections_trig  nsector = nt::algorithms::sum(e1, [](auto&& e) {return e.n_sections_trig; });

      return  nt::ntuple(
                nsector,
                back2back(bit_mask == 2)

              );
    }
                 );

    __CSV__WRITE__(m_summary1);


  }

  int klmtrg_layer_counter_t::get_n_sections_trig(int subdetector)
  {
    for (const auto& e : m_summary1) {
      if (e.subdetector == subdetector) {
        return e.n_sections_trig;
      }
    }
    return 0;
  }

  int klmtrg_layer_counter_t::get_triggermask(int subdetector, int section)
  {
    for (const auto& e : m_sections_trig) {
      if (e.subdetector == subdetector && e.section == section) {
        return e.sector_mask;
      }
    }
    return 0;
  }

  int klmtrg_layer_counter_t::get_BKLM_back_to_back_flag(int subdetector)
  {
    for (const auto& e : m_summary1) {
      if (e.subdetector == subdetector) {
        return e.back2back;
      }
    }
    return 0;
  }

}
