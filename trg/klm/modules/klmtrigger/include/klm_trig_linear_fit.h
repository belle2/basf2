/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <algorithm>
#include <numeric>
#include <vector>
#include <tuple>
#include <iostream>
#include <bitset>
#include <map>



#include "trg/klm/modules/klmtrigger/KLMAxis.h"


namespace Belle2 {
  class klm_trig_linear_fit_t {
  public:
    void clear_geometry();
    void add_geometry(const KLM_TRG_definitions::KLM_geo_fit_t&  geometry);
    void set_y_cutoff(int cutoff);
    void set_intercept_cutoff(int cutoff);


    void run(const KLM_TRG_definitions::KLM_Digit_compact_ts&  hits);


    int get_triggermask(int subdetector, int section);
    int get_triggermask_or(int subdetector, int section);
    const KLM_TRG_definitions::KLM_trig_linear_fits& get_result() const ;


  private:
    KLM_TRG_definitions::KLM_trig_linear_fits m_linear_fits;
    int y_cutoff = 100;
    int m_intercept_cutoff = 500;

    using geo_KLM_t =
      nt::ntuple<KLM_TRG_definitions::slopeX, KLM_TRG_definitions::offsetX, KLM_TRG_definitions::slopeY, KLM_TRG_definitions::offsetY>;

    std::map<int64_t, geo_KLM_t> m_KLMgeomap;

    KLM_TRG_definitions::subdetector m_bklm_constant;




    using sections_trig_t =
      std::vector<nt::ntuple< KLM_TRG_definitions::subdetector, KLM_TRG_definitions::section, KLM_TRG_definitions::sector_mask, KLM_TRG_definitions::sector_mask_or>>;
    sections_trig_t m_sections_trig;

  };




}