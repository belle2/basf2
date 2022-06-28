#ifndef KLMAxis_H
#define KLMAxis_H


#include "group_helper.h"

using namespace Belle2::group_helper;
namespace Belle2 {


  AXIS_NAME(sector, int64_t);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(section, int64_t);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(Subdetector, int64_t);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(plane, int64_t);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(strip, int64_t);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(layer, int64_t);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(x, double);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(y, double);// cppcheck-suppress  noExplicitConstructor
  using KLM_geo_t = std::tuple<Subdetector, section, sector, layer, plane,   strip,  x, y>;

  namespace geofit {
    AXIS_NAME(sector, double);// cppcheck-suppress  noExplicitConstructor
    AXIS_NAME(section, double);// cppcheck-suppress  noExplicitConstructor
    AXIS_NAME(Subdetector, double);// cppcheck-suppress  noExplicitConstructor
    AXIS_NAME(plane, double);// cppcheck-suppress  noExplicitConstructor
    AXIS_NAME(strip, double);// cppcheck-suppress  noExplicitConstructor
    AXIS_NAME(layer, double);// cppcheck-suppress  noExplicitConstructor
    AXIS_NAME(slope, double);// cppcheck-suppress  noExplicitConstructor
    AXIS_NAME(offset, double);// cppcheck-suppress  noExplicitConstructor
    using KLM_geo_fit_t = std::tuple<Subdetector, section, sector, layer, plane,    slope, offset>;
  }

  AXIS_NAME(event_nr_t, int);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(KLM_type, int);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(section_t, int);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(sector_t, int);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(layer_t, int);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(plane_t, int);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(strip_t, int);// cppcheck-suppress  noExplicitConstructor

  using KLM_Digit = std::tuple <
                    event_nr_t,
                    KLM_type,
                    section_t,
                    sector_t,
                    plane_t,
                    layer_t,
                    strip_t
                    >;




  AXIS_NAME(layer_count, int);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(layer_mask, uint64_t);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(n_triggered, int);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(sector_mask, int);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(sector_mask_or, int);// cppcheck-suppress  noExplicitConstructor

  AXIS_NAME(n_sections_trig, int);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(back2back_t, int);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(isectors_t, int);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(TriggerCut, int);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(vetoCut, int);

  AXIS_NAME(pos_x_t, int);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(pos_y_t, int);// cppcheck-suppress  noExplicitConstructor


  AXIS_NAME(slopeXY_t, double);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(interceptXY_t, double);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(ipXY_t, double);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(chisqXY_t, double);// cppcheck-suppress  noExplicitConstructor

  using KLM_Digit_compact = std::tuple< KLM_type, section_t, isectors_t, sector_t, plane_t, layer_t>;


  namespace KLM_digit_n {
    AXIS_NAME(event_nr, int);// cppcheck-suppress  noExplicitConstructor
    AXIS_NAME(strip, double);// cppcheck-suppress  noExplicitConstructor
    AXIS_NAME(charge, double);// cppcheck-suppress  noExplicitConstructor
    AXIS_NAME(ctime, double);// cppcheck-suppress  noExplicitConstructor
    AXIS_NAME(Subdetector, double);// cppcheck-suppress  noExplicitConstructor
    AXIS_NAME(TDC, double);// cppcheck-suppress  noExplicitConstructor
    AXIS_NAME(time, double);// cppcheck-suppress  noExplicitConstructor
    AXIS_NAME(sector, double);// cppcheck-suppress  noExplicitConstructor
    AXIS_NAME(section, double);// cppcheck-suppress  noExplicitConstructor
    AXIS_NAME(layer, double);// cppcheck-suppress  noExplicitConstructor
    AXIS_NAME(plane, double);// cppcheck-suppress  noExplicitConstructor



    using KLM_digit_t = std::tuple <
                        event_nr,
                        Subdetector,
                        section,
                        sector,
                        layer,
                        plane,
                        strip,
                        charge,
                        ctime,
                        TDC,
                        time
                        >;

  }

  AXIS_NAME(BKLM_n_trg_sectors, int);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(EKLM_n_trg_sectors, int);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(Sector_mask_Backward_Barrel, int);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(Sector_mask_Forward_Barrel, int);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(Sector_mask_Backward_Endcap, int);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(Sector_mask_Forward_Endcap, int);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(Sector_mask_OR_Backward_Barrel, int);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(Sector_mask_OR_Forward_Barrel, int);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(Sector_mask_OR_Backward_Endcap, int);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(Sector_mask_OR_Forward_Endcap, int);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(BKLM_back_to_back_flag, int);// cppcheck-suppress  noExplicitConstructor
  AXIS_NAME(EKLM_back_to_back_flag, int);// cppcheck-suppress  noExplicitConstructor

  using KLM_trg_summery = std::tuple <
                          KLM_digit_n::event_nr,
                          BKLM_n_trg_sectors,
                          EKLM_n_trg_sectors,
                          Sector_mask_Backward_Barrel,
                          Sector_mask_Forward_Barrel,
                          Sector_mask_Backward_Endcap,
                          Sector_mask_Forward_Endcap,
                          Sector_mask_OR_Backward_Barrel,
                          Sector_mask_OR_Forward_Barrel,
                          Sector_mask_OR_Backward_Endcap,
                          Sector_mask_OR_Forward_Endcap,
                          BKLM_back_to_back_flag,
                          EKLM_back_to_back_flag

                          >;

  using KLM_trig_linear_fit = std::tuple <
                              KLM_digit_n::event_nr,
                              KLM_type,
                              section_t,
                              sector_t,
                              plane_t,
                              slopeXY_t,
                              interceptXY_t,
                              ipXY_t,
                              chisqXY_t
                              >;

  using KLM_Digit_with_geo_t =
    std::tuple<KLM_digit_n::event_nr,  KLM_type, section_t, sector_t, plane_t, layer_t, strip_t, pos_x_t, pos_y_t >;

}
#endif