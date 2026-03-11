/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 *                                                                        *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vector>

#include <TH2Poly.h>
#include <TH1.h>
#include <TLine.h>

namespace Belle2 {

  /**
   * ARICH merger histogram (72 bins).
   *
   * Numbering:
   * - TH1 bin i (1..72) corresponds directly to merger number i.
   * - Sector base is (X-1)*12, X=1..6.
   * - Within each sector: ring1 (3 blocks), ring2 (4), ring3 (4), ring4 (1),
   *   with right-to-left numbering inside each ring.
   */
  class ARICHMergerHist : public TH2Poly {

  public:
    /// Default constructor.
    ARICHMergerHist() = default;

    /// Constructor.
    /// @param name Histogram name
    /// @param title Histogram title
    /// @param globalRotationDeg Global rotation applied to the whole layout (degrees)
    /// @param rightToLeftNumbering If true, bins inside each ring are numbered right-to-left
    ARICHMergerHist(const char* name,
                    const char* title,
                    double globalRotationDeg = 0.0,
                    bool rightToLeftNumbering = true);

    /// Fill from TH1. Histogram must have exactly 72 bins.
    /// @param hist Input TH1 with 72 bins
    void fillFromTH1(TH1* hist);

    /// Draw with sector lines and labels.
    /// @param option ROOT draw option string
    void Draw(Option_t* option = "") override;

  protected:
    std::vector<int> m_merger2bin;       /**< Merger number (1..72) → TH2Poly bin index. */
    double m_globalRotationDeg = 0.0;    /**< Global rotation applied to the geometry (degrees). */
    bool m_rightToLeft = true;           /**< If true, numbering in each ring is right-to-left. */
    TLine m_lines[6];                    /**< Sector boundary lines drawn on top of the histogram. */

    /// Convert polar coordinates to cartesian.
    /// @param r Radius
    /// @param angDeg Angle in degrees
    /// @param x Output x coordinate
    /// @param y Output y coordinate
    static void pol2xy(double r, double angDeg, double& x, double& y);

    /// \cond
    ClassDefOverride(ARICHMergerHist, 1);
    /// \endcond
  };

} // namespace Belle2