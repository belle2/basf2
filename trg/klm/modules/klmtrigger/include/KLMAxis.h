/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#pragma once


#include "trg/klm/modules/klmtrigger/group_helper.h"


namespace Belle2 {
  namespace group_helper {

    namespace KLM_Generic {
      AXIS_NAME(event_nr, int);
    }

    namespace KLM_Coordinates_n {
      AXIS_NAME(sector, int32_t);
      AXIS_NAME(section, int32_t);
      AXIS_NAME(Subdetector, int32_t);
      AXIS_NAME(strip, int32_t);
      AXIS_NAME(layer, int32_t);
      AXIS_NAME(plane, int32_t);
      AXIS_NAME(isectors_t, int);
    }


    namespace geofit {
      AXIS_NAME(slopeY, double);
      AXIS_NAME(offsetY, double);
      AXIS_NAME(slopeX, double);
      AXIS_NAME(offsetX, double);
    }

    using KLM_geo_fit_t =
      std::tuple<KLM_Coordinates_n::Subdetector, KLM_Coordinates_n::section, KLM_Coordinates_n::sector, KLM_Coordinates_n::layer, KLM_Coordinates_n::plane, geofit::slopeX, geofit::offsetX,  geofit::slopeY, geofit::offsetY>;



    using KLM_Digit_compact = std::tuple <
                              KLM_Generic::event_nr,
                              KLM_Coordinates_n::Subdetector,
                              KLM_Coordinates_n::section,
                              KLM_Coordinates_n::isectors_t,
                              KLM_Coordinates_n::sector,
                              KLM_Coordinates_n::plane,
                              KLM_Coordinates_n::layer,
                              KLM_Coordinates_n::strip
                              >;




    AXIS_NAME(slopeXY_t, double);
    AXIS_NAME(interceptXY_t, double);
    AXIS_NAME(ipXY_t, double);
    AXIS_NAME(chisqXY_t, double);
    AXIS_NAME(Nhits_t, int);


    using KLM_trig_linear_fit = std::tuple <
                                KLM_Generic::event_nr,
                                KLM_Coordinates_n::Subdetector,
                                KLM_Coordinates_n::section,
                                KLM_Coordinates_n::sector,
                                KLM_Coordinates_n::plane,
                                slopeXY_t,
                                interceptXY_t,
                                ipXY_t,
                                chisqXY_t,
                                Nhits_t
                                >;



    AXIS_NAME(BKLM_n_trg_sectors, int);
    AXIS_NAME(EKLM_n_trg_sectors, int);
    AXIS_NAME(Sector_mask_Backward_Barrel, int);
    AXIS_NAME(Sector_mask_Forward_Barrel, int);
    AXIS_NAME(Sector_mask_Backward_Endcap, int);
    AXIS_NAME(Sector_mask_Forward_Endcap, int);
    AXIS_NAME(Sector_mask_OR_Backward_Barrel, int);
    AXIS_NAME(Sector_mask_OR_Forward_Barrel, int);
    AXIS_NAME(Sector_mask_OR_Backward_Endcap, int);
    AXIS_NAME(Sector_mask_OR_Forward_Endcap, int);
    AXIS_NAME(BKLM_back_to_back_flag, int);
    AXIS_NAME(EKLM_back_to_back_flag, int);

    using KLM_trg_summery = std::tuple <
                            KLM_Generic::event_nr,
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


  }

}
