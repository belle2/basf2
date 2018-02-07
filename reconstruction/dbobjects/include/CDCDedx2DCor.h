/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/logging/Logger.h>

#include <TObject.h>
#include <TH2F.h>

namespace Belle2 {

  /**
   *   dE/dx wire gain calibration constants
   */

  class CDCDedx2DCor: public TObject {

  public:

    /**
     * Default constructor
     */
    CDCDedx2DCor(): m_version(), m_twodgains() {};

    /**
     * Constructor
     */
    CDCDedx2DCor(short version, std::vector<TH2F>& twodgains): m_version(version), m_twodgains(twodgains) {};

    /**
     * Destructor
     */
    ~CDCDedx2DCor() {};

    /** Get the version for the 2D correction
     */
    short getVersion() const {return m_version; };

    /** Get the version for the 2D histogram for the correction
     */
    const TH2F* getHist() const {return &m_twodgains[0]; };

    /** Return dE/dx mean value for given DOCA and entrance angle
     * @param continuous layer number
     * @param distance of closest approach
     * @param entrance angle
     */
    double getMean(int layer, double doca, double enta) const
    {
      if (layer > 56) {
        B2ERROR("Asking for a CDC layer beyond 56!");
        return 0;
      }

      // assume rotational symmetry
      if (enta < -3.1416 / 2.0) enta += 3.1416 / 2.0;
      if (enta > 3.1416 / 2.0) enta -= 3.1416 / 2.0;

      int binx = floor((doca + 1.5) / 0.3);
      int biny = floor((sin(enta) + 1.0) / 0.2);

      double mean;
      if (binx < 0 || biny < 0 || binx >= m_twodgains[0].GetXaxis()->GetNbins() || biny >= m_twodgains[0].GetYaxis()->GetNbins()) {
        B2WARNING("Problem with the 2D correction for CDC dE/dx!");
        mean = 1.0;
      } else mean = m_twodgains[0].GetBinContent(binx, biny);

      return mean;
    };

  private:
    /** dE/dx gains versus DOCA and entrance angle
    may be different for different layers, so store as a vector
    keep a version number to identify which layers are valid */
    short m_version; /**< version number for 2D correction */
    std::vector<TH2F> m_twodgains; /**< 2D histograms of doca/enta gains, layer dependent */

    ClassDef(CDCDedx2DCor, 1); /**< ClassDef */
  };
} // end namespace Belle2
