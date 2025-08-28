/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#pragma once
#include <vector>
#include "trg/klm/modules/klmtrigger/KLMAxis.h"


namespace Belle2 {


  class klmtrgLayerCounter {
  public:
    void clear_layersUsed();
    void add_layersUsed(int layersUsed);
    void set_NLayerTrigger(int NlayerTrigger);

    void run(const KLM_TRG_definitions::KLM_Digit_compact_ts& hits);

    int get_n_sections_trig(int subdetector);
    int get_triggermask(int subdetector, int section);
    int get_BKLM_back_to_back_flag(int subdetector);


  private:
    std::vector<nt::ntuple<KLM_TRG_definitions::layer>> m_layersUsed;
    int m_NlayerTrigger;





    using sections_trig_t =
      std::vector<nt::ntuple< KLM_TRG_definitions::subdetector, KLM_TRG_definitions::section, KLM_TRG_definitions:: sector_mask, KLM_TRG_definitions::n_sections_trig>>;
    sections_trig_t m_sections_trig;

    using summary1_t =
      std::vector<nt::ntuple< KLM_TRG_definitions::subdetector, KLM_TRG_definitions::n_sections_trig, KLM_TRG_definitions::back2back >>;
    summary1_t m_summary1;


  };



}