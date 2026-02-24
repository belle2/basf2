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
   *   TH1 bin i (1..72) corresponds directly to merger number i.
   *   Sector base is (X-1)*12, X=1..6.
   *   Within each sector:
   *     ring1 (3 blocks), ring2 (4), ring3 (4), ring4 (1),
   *   with right-to-left numbering inside each ring
   */
  class ARICHMergerHist : public TH2Poly {

  public:
    ARICHMergerHist() = default;

    ARICHMergerHist(const char* name,
                    const char* title,
                    double globalRotationDeg = 0.0,
                    bool rightToLeftNumbering = true);

    /**
     * Fill from TH1.
     * Histogram must have exactly 72 bins.
     */
    void fillFromTH1(TH1* hist);

    /**
     * Draw with sector lines.
     */
    void Draw(Option_t* option = "") override;

  protected:
    std::vector<int> m_merger2bin;   /**< merger number (1..72) → TH2Poly bin index */
    double m_globalRotationDeg = 0.0;
    bool m_rightToLeft = true;

    TLine m_lines[6];

    static void pol2xy(double r, double angDeg, double& x, double& y);

    ClassDefOverride(ARICHMergerHist, 1);
  };

} // namespace Belle2