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
    void set_bklm_constant(KLM_TRG_definitions::subdetector sub);

    void run(const KLM_TRG_definitions::KLM_Digit_compact_ts&  hits);


    int get_triggermask(int subdetector, int section);
    int get_triggermask_or(int subdetector, int section);
    const KLM_TRG_definitions::KLM_trig_linear_fits& get_result() const ;


  private:
    KLM_TRG_definitions::KLM_trig_linear_fits m_linear_fits;
    int y_cutoff = 100;
    int m_intercept_cutoff = 500;
    std::map<int64_t, nt::ntuple<KLM_TRG_definitions::geo_id, KLM_TRG_definitions::slopeY, KLM_TRG_definitions::offsetY>> m_BKLMgeomap;
    std::map<int64_t, nt::ntuple<KLM_TRG_definitions::slopeX, KLM_TRG_definitions::offsetX, KLM_TRG_definitions::slopeY, KLM_TRG_definitions::offsetY>>
        m_EKLMgeomap;
    KLM_TRG_definitions::subdetector m_bklm_constant;

    double m_bklm_slopeX;
    double m_bklm_offsetX;


    using sections_trig_t =
      std::vector<nt::ntuple< KLM_TRG_definitions::subdetector, KLM_TRG_definitions::section, KLM_TRG_definitions::sector_mask, KLM_TRG_definitions::sector_mask_or>>;
    sections_trig_t m_sections_trig;

  };




}