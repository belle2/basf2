/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/



#include "trg/klm/modules/klmtrigger/klm_trig_linear_fit.h"
#include "trg/klm/modules/klmtrigger/group_helper.h"
#include "trg/klm/modules/klmtrigger/KLMAxis.h"



AXIS_NAME(x_pos, int);
AXIS_NAME(y_pos, int);

using namespace Belle2::group_helper::KLM_Coordinates_n;
using namespace Belle2::group_helper::KLM_Generic;
using namespace Belle2::group_helper;
namespace Belle2 {



  struct Linear_fit_of_Hits_t {
    template <typename Container_T>
    auto  operator()(const Container_T& container) const
    {
      int sumX = 0, sumY = 0, sumXX = 0, sumXY = 0, sumYY = 0;
      for (const auto& e : container) {
        sumX += x_pos(e);
        sumXX += x_pos(e) * x_pos(e);

        sumY += y_pos(e);
        sumYY += y_pos(e) * y_pos(e);

        sumXY += x_pos(e) * y_pos(e);
      }
      auto nHits = Nhits_t(container.size());
      int denom = sumXX * nHits - sumX * sumX;
      if (denom == 0) {
        return std::tuple(slopeXY_t(1e9), interceptXY_t(1e9), ipXY_t(1e9), chisqXY_t(1e9), nHits);
      }
      auto slopeXY = slopeXY_t((double)(sumXY * nHits - sumX * sumY) / (double)denom);
      auto  interceptXY = interceptXY_t((double)(sumXX * sumY - sumX * sumXY) / (double)denom);

      auto  ipXY = ipXY_t(interceptXY * interceptXY * (1.0 - slopeXY * slopeXY));

      auto  chisqXY = chisqXY_t(slopeXY * slopeXY * sumXX
                                + interceptXY * interceptXY * nHits
                                + sumYY
                                + 2.0 * slopeXY * interceptXY * sumX
                                - 2.0 * slopeXY * sumXY
                                - 2.0 * interceptXY * sumY);

      return  std::tuple(slopeXY, interceptXY, ipXY, chisqXY, nHits) ;
    }
  };
  constexpr Linear_fit_of_Hits_t Linear_fit_of_Hits;








  std::vector<KLM_trig_linear_fit> klm_trig_linear_fit(std::vector<Belle2::group_helper::KLM_Digit_compact>& hits,
                                                       const std::vector<Belle2::group_helper::KLM_geo_fit_t>& geometry)
  {
    sort(hits);

    auto hits_w_geo_fit = vec_join(
                            hits,
                            geometry,
                            comparators::on_common_args,
    [&](const auto & e1, const auto & e2) {
      return  std::tuple(
                event_nr(e1),
                Subdetector(e1),
                section(e1),
                sector(e1),
                plane(e1),
                layer(e1),
                strip(e1),
                x_pos(layer(e1) *  geofit::slopeX(e2) + geofit::offsetX(e2)),
                y_pos(strip(e1) * geofit::slopeY(e2) + geofit::offsetY(e2))
              );
    }

                          );

    sort(hits_w_geo_fit);
    auto linear_fited = group<event_nr, Subdetector, section, sector, plane>::apply(hits_w_geo_fit,
                        Linear_fit_of_Hits
                                                                                   );
    sort(linear_fited);
    return linear_fited;

  }

}