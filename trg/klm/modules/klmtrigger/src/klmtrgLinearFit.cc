/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/



#include "trg/klm/modules/klmtrigger/klmtrgLinearFit.h"
#include "trg/klm/modules/klmtrigger/IO_csv.h"
#include "trg/klm/modules/klmtrigger/bit_operations.h"

#include "trg/klm/modules/klmtrigger/ntuples_full.h"


#include <exception>

#include <array>

using namespace Belle2::KLM_TRG_definitions;


namespace Belle2 {

  void bitshift(int64_t& out, int64_t& shift,  int64_t in,  int64_t deltashift)
  {
    int64_t mask = 0xFFFFFF;
    if (((mask << deltashift) & in) > 0) {
      throw std::runtime_error("bitshift error");
    }

    out |= (in << shift);
    shift += deltashift;
  }

  template <typename T>
  constexpr int64_t get_index(const T& e, int track_id = 0)
  {
    int64_t ret = 0;
    int64_t shift = 0;
    bitshift(ret, shift, track_id, 5);
    bitshift(ret, shift, e.layer,  6);
    bitshift(ret, shift, e.plane, 2);
    bitshift(ret, shift, e.sector, 6);
    bitshift(ret, shift, e.section, 3);
    bitshift(ret, shift, e.subdetector, 3);


    return ret;
  };

  struct track_maker_t {
    int32_t y_cutoff = 0;
    int m_number_of_tracks = 4;
    struct internal {
      int32_t last_x = 0, last_y = 0;
      int nDigits = 0;
    };

    std::map< int, internal > m_storage;


    template <typename T>
    int operator()(const T& e)
    {
      for (int j = 0; j < m_number_of_tracks; ++j) {

        auto& track = m_storage[get_index(e, j)];
        if (
          std::abs(track.last_y - e.y_pos) < y_cutoff
          ||
          track.nDigits == 0
        ) {
          update_track(j, e);
          return  j;
        }
      }

      const auto last_track = m_number_of_tracks - 1;
      update_track(last_track, e);
      return last_track;

    }

    template <typename T>
    void update_track(int Track_ID, const T& e)
    {
      auto& track = m_storage[get_index(e, Track_ID)];
      track.last_y = e.y_pos;
      track.last_x = e.x_pos;
      track.nDigits++;
    }

  };

  struct Linear_fit_of_Hits_t {
    template <typename Container_T>
    auto  operator()(const Container_T& container) const
    {



      int64_t sumX = 0, sumY = 0, sumXX = 0, sumXY = 0, sumYY = 0;
      for (const auto& e : container) {
        sumX += e.x_pos;
        sumXX += e.x_pos * e.x_pos;

        sumY += e.y_pos;
        sumYY += e.y_pos * e.y_pos;

        sumXY += e.x_pos * e.y_pos;
      }
      int64_t nHits = container.size();
      int denom = sumXX * nHits - sumX * sumX;
      if (denom == 0) {
        return nt::ntuple(slopeXY(1e9), interceptXY(1e9), ipXY(1e9), chisqXY(1e9), Nhits(nHits));
      }

      auto slopeXY_ = slopeXY((double)(sumXY * nHits - sumX * sumY) / (double)denom);
      auto  interceptXY_ = interceptXY((double)(sumXX * sumY - sumX * sumXY) / (double)denom);

      auto  ipXY_ = ipXY(interceptXY_ * interceptXY_ * (1.0 - slopeXY_ * slopeXY_));

      auto  chisqXY_ = chisqXY(slopeXY_ * slopeXY_ * sumXX
                               + interceptXY_ * interceptXY_ * nHits
                               + sumYY
                               + 2.0 * slopeXY_ * interceptXY_ * sumX
                               - 2.0 * slopeXY_ * sumXY
                               - 2.0 * interceptXY_ * sumY);

      return  nt::ntuple(slopeXY_, interceptXY_, ipXY_, chisqXY_, Nhits(nHits));
    }
  };
  constexpr Linear_fit_of_Hits_t Linear_fit_of_Hits;







  void klmtrgLinearFit::clear_geometry()
  {
    m_KLMgeomap.clear();

  }

  void klmtrgLinearFit::add_geometry(const KLM_geo_fit_t& geometry)
  {

    m_KLMgeomap[get_index(geometry)] = geo_KLM_t{
      geometry.slopeX,
      geometry.offsetX,
      geometry.slopeY,
      geometry.offsetY
    };


  }

  void klmtrgLinearFit::run(const KLM_Digit_compact_ts& hits)
  {

    __CSV__WRITE__(hits);
    m_linear_fits.clear();


    auto hits_w_geo_fit = nt::algorithms::add_column(
                            hits,
    [&](const auto & e1) {
      auto ret = nt::ntuple(
                   Belle2::KLM_TRG_definitions::x_pos(0),
                   Belle2::KLM_TRG_definitions::y_pos(0)
                 );

      auto&& e2 = m_KLMgeomap[get_index(e1)];
      ret.x_pos.v = e1.layer * e2.slopeX + e2.offsetX;
      ret.y_pos.v = e1.strip * e2.slopeY + e2.offsetY;
      return ret;

    }
                          );


    __CSV__WRITE__(hits_w_geo_fit);
    nt::algorithms::sort(hits_w_geo_fit);


    auto track_maker_ = track_maker_t{};
    track_maker_.y_cutoff = y_cutoff;


    auto hits_w_geo_fit_w_tracks = nt::algorithms::add_column(
                                     hits_w_geo_fit,
    [&](const auto & e) {
      return nt::ntuple{
        track_id(track_maker_(e))
      };


    }

                                   );


    __CSV__WRITE__(hits_w_geo_fit_w_tracks);



    m_linear_fits = nt_group(
                      hits_w_geo_fit_w_tracks[0].subdetector,
                      hits_w_geo_fit_w_tracks[0].section,
                      hits_w_geo_fit_w_tracks[0].sector,
                      hits_w_geo_fit_w_tracks[0].plane,
                      hits_w_geo_fit_w_tracks[0].track_id
                    ).apply_append(
                      hits_w_geo_fit_w_tracks,
                      Linear_fit_of_Hits
                    );
    __CSV__WRITE__(m_linear_fits);


    nt::algorithms::filter(m_linear_fits, [&](const auto & e) { return std::abs(e.interceptXY) <= m_intercept_cutoff; });


    auto m_linear_fits1 = nt_group(
                            m_linear_fits[0].subdetector,
                            m_linear_fits[0].section,
                            m_linear_fits[0].sector
                          ).apply_append(
                            m_linear_fits,
    [](const auto & es) {
      bool plane0 = false;
      bool plane1 = false;
      for (const auto& e : es) {
        if (e.plane == 0) { plane0 = true; }
        if (e.plane == 1) { plane1 = true; }
      }
      return nt::ntuple{
        ax_maker(trigger_or) = (int)(plane0 || plane1),
        ax_maker(trigger_and) = (int)(plane0 && plane1)
      };
    }
                          );
    __CSV__WRITE__(m_linear_fits1);

    m_sections_trig = nt_group(
                        m_linear_fits1[0].subdetector,
                        m_linear_fits1[0].section
                      ).apply_append(
                        m_linear_fits1,
    [](const auto & e) {
      auto ret = nt::ntuple{
        KLM_TRG_definitions::sector_mask {},
        KLM_TRG_definitions::sector_mask_or{}
      };
      auto trig_and = nt::algorithms::filter_copy(e, [](const auto & x) { return x.trigger_and > 0; });
      ret.sector_mask.v = Belle2::to_bit_mask(nt_span(trig_and, sector));
      ret.sector_mask_or.v = Belle2::to_bit_mask(nt_span(trig_and, sector));
      return ret;

    }
                      );
    __CSV__WRITE__(m_sections_trig);


  }

  const KLM_trig_linear_fits& klmtrgLinearFit::get_result() const
  {
    return m_linear_fits;
  }



  int klmtrgLinearFit::get_triggermask(int subdetector, int section)
  {
    for (const auto& e : m_sections_trig) {
      if (e.subdetector == subdetector && e.section == section) {
        return e.sector_mask;
      }
    }
    return 0;
  }

  int klmtrgLinearFit::get_triggermask_or(int subdetector, int section)
  {
    for (const auto& e : m_sections_trig) {
      if (e.subdetector == subdetector && e.section == section) {
        return e.sector_mask_or;
      }
    }
    return 0;
  }

  void klmtrgLinearFit::set_y_cutoff(int cutoff)
  {
    y_cutoff = cutoff;
  }

  void klmtrgLinearFit::set_intercept_cutoff(int cutoff)
  {
    m_intercept_cutoff = cutoff;
  }

}


