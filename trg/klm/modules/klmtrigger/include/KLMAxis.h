/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#pragma once


#include "trg/klm/modules/klmtrigger/ntuples_full.h"
#include <cstdint>



namespace Belle2 {
  namespace KLM_TRG_definitions {
    nt_new_axis_t(event_nr, int32_t{});
    nt_new_axis_t(sector, int32_t{});
    nt_new_axis_t(section, int32_t{});
    nt_new_axis_t(subdetector, int32_t{});
    nt_new_axis_t(strip, int32_t{});
    nt_new_axis_t(layer, int32_t{});
    nt_new_axis_t(plane, int32_t{});
    nt_new_axis_t(digit_id, int32_t{});



    using KLM_Digit_compact_t = nt::ntuple <
                                event_nr,
                                digit_id,
                                subdetector,
                                section,
                                sector,
                                plane,
                                layer,
                                strip
                                >;

    using KLM_Digit_compact_ts = std::vector < KLM_Digit_compact_t>;

    nt_new_axis_t(x_pos, int32_t{});
    nt_new_axis_t(y_pos, int32_t{});



    nt_new_axis_t(slopeX, double{});
    nt_new_axis_t(offsetX, double{});
    nt_new_axis_t(slopeY, double{});
    nt_new_axis_t(offsetY, double{});

    nt_new_axis_t(slopeXY, double{});
    nt_new_axis_t(interceptXY, double{});
    nt_new_axis_t(ipXY, double{});
    nt_new_axis_t(chisqXY, double{});
    nt_new_axis_t(Nhits, double{});

    nt_new_axis_t(track_id, int{});
    nt_new_axis_t(geo_id, int{});

    using KLM_geo_fit_t =
      nt::ntuple<geo_id, subdetector, section, sector, layer, plane, slopeX, offsetX,  slopeY, offsetY>;
    using KLM_geo_fit_t_old =
      nt::ntuple< subdetector, section, sector, layer, plane, slopeX, offsetX, slopeY, offsetY>;
    using KLM_geo_fit_ts = std::vector< KLM_geo_fit_t>;

    using KLM_trig_linear_fit = nt::ntuple <
                                subdetector,
                                section,
                                sector,
                                plane,
                                track_id,
                                slopeXY,
                                interceptXY,
                                ipXY,
                                chisqXY,
                                Nhits
                                >;


    using KLM_trig_linear_fits = std::vector< KLM_trig_linear_fit>;



    nt_new_axis_t(n_sections_trig, int32_t{});
    nt_new_axis_t(sector_mask, int32_t{});
    nt_new_axis_t(sector_mask_or, int32_t{});
    nt_new_axis_t(back2back, int32_t{});
  };


}
