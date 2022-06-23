
#include "trg/klm/modules/klmtrigger/klm_trig_linear_fit.h"
#include "trg/klm/modules/klmtrigger/group_helper.h"
#include "trg/klm/modules/klmtrigger/KLMAxis.h"
#include "trg/klm/modules/klmtrigger/group_io.h"

namespace Belle2 {


  std::vector<KLM_trig_linear_fit> Linear_fit_of_Hits(const std::vector<KLM_Digit_with_geo_t>& Hits)
  {
    auto hits_w_geo_fit = group<KLM_digit_n::event_nr, KLM_type, section_t, sector_t, plane_t>::apply1(Hits,
    [](const auto & container) {
      int sumX = 0, sumY = 0, sumXX = 0, sumXY = 0, sumYY = 0;
      for (const auto& e : container) {
        sumX += pos_x_t(e);
        sumXX += pos_x_t(e) * pos_x_t(e);

        sumY += pos_y_t(e);
        sumYY += pos_y_t(e) * pos_y_t(e);

        sumXY += pos_x_t(e) * pos_y_t(e);
      }
      auto nHits = container.size();
      int denom = sumXX * nHits - sumX * sumX;
      if (denom == 0) {
        return std::make_tuple(slopeXY_t(1e9), interceptXY_t(1e9), ipXY_t(1e9), chisqXY_t(1e9));
      }
      slopeXY_t slopeXY = (double)(sumXY * nHits - sumX * sumY) / (double)denom;
      interceptXY_t interceptXY = (double)(sumXX * sumY - sumX * sumXY) / (double)denom;

      ipXY_t ipXY = interceptXY * interceptXY * (1.0 - slopeXY * slopeXY);

      chisqXY_t chisqXY = slopeXY * slopeXY * sumXX
                          + interceptXY * interceptXY * nHits
                          + sumYY
                          + 2.0 * slopeXY * interceptXY * sumX
                          - 2.0 * slopeXY * sumXY
                          - 2.0 * interceptXY * sumY;

      return  std::make_tuple(slopeXY, interceptXY, ipXY, chisqXY);
    }
                                                                                                      );
    sort(hits_w_geo_fit);
    return hits_w_geo_fit;

  }






  std::vector<KLM_Digit_with_geo_t> join_klmDigits_with_geometry(const std::vector<Belle2::KLM_Digit>& hits,
      const std::vector<Belle2::geofit::KLM_geo_fit_t>& geometry)
  {

    return vec_join(hits, geometry,
    [](const auto & e1, const auto & e2) {
      return (sector_t(e1) == geofit::sector(e2)) &&
             (section_t(e1) == geofit::section(e2)) &&
             (KLM_type(e1) == geofit::Subdetector(e2)) &&
             (plane_t(e1) == geofit::plane(e2)) &&
             (layer_t(e1) == geofit::layer(e2));

    },
    [&](const auto & e1, const auto & e2) {
      constexpr double x_slope = 72.81389381173733;
      constexpr double x_ofset = 1627.463846928903;
      return  std::make_tuple(
                KLM_digit_n::event_nr(event_nr_t(e1)),
                KLM_type(e1),
                section_t(e1),
                sector_t(e1),
                plane_t(e1),
                layer_t(e1),
                strip_t(e1),
                pos_x_t(layer_t(e1) * x_slope + x_ofset),
                pos_y_t(strip_t(e1) * geofit::slope(e2) + geofit::offset(e2))
              );
    }

                   );
  }




  std::vector<KLM_trig_linear_fit> klm_trig_linear_fit(std::vector<Belle2::KLM_Digit>& hits,
                                                       const std::vector<Belle2::geofit::KLM_geo_fit_t>& data)
  {
    sort(hits);

    auto hits_w_geo_fit = join_klmDigits_with_geometry(hits, data);
    auto linear_fited = Linear_fit_of_Hits(hits_w_geo_fit);
    return linear_fited;
  }

}