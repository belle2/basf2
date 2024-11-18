/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/logging/Logger.h>

#include <TObject.h>
#include <TH2F.h>

namespace Belle2 {

  /**
   *   dE/dx wire gain calibration constants
   */

  class CDCDedx2DCell: public TObject {

  public:

    /**
     * Default constructor
     */
    CDCDedx2DCell(): m_version(), m_twodgains() {};

    /**
     * Constructor
     */
    CDCDedx2DCell(short version, const std::vector<TH2F>& twodgains): m_version(version), m_twodgains(twodgains) {};

    /**
     * Destructor
     */
    ~CDCDedx2DCell() {};

    /**
     * Combine payloads
     **/
    CDCDedx2DCell& operator*=(CDCDedx2DCell const& rhs)
    {
      if (m_version != rhs.getVersion()) {
        B2WARNING("2D cell gain parameters do not match, cannot merge!");
        return *this;
      }
      for (unsigned int layer = 0; layer < getSize(); ++layer) {
        const TH2F* newhist = rhs.getHist(layer);
        if (newhist->GetEntries() > 0)m_twodgains[layer].Multiply(newhist);
        else B2ERROR("ERROR! constant histograms is empty");
      }
      return *this;
    }

    /** Get the version for the 2D correction
     */
    short getVersion() const {return m_version; };

    /** Get the number of histograms for the 2D correction
     */
    unsigned int getSize() const { return m_twodgains.size(); };

    /** Get the 2D histogram for the correction for this layer
     */
    const TH2F* getHist(int layer) const
    {
      if (m_twodgains.size() == 0) {
        B2ERROR("ERROR!");
        return NULL;
      }

      if (m_twodgains.size() == 2) {
        if (layer == 0) return &m_twodgains[0];
        else if (layer == 1) return &m_twodgains[1];
        else {
          B2ERROR("ERROR! const histograms NOT found");
          return NULL;
        }
      } else if (m_twodgains.size() == 56) {
        return &m_twodgains[layer];
      } else {
        B2ERROR("ERROR! Something is wrong # of const histograms");
        return NULL;
      }
      return NULL;
    }

    /** Return dE/dx mean value for the given bin
     * @param layer layer number
     * @param dbin doca bin number
     * @param ebin enta bin number
     */
    double getMean(unsigned int layer, int dbin, int ebin) const
    {
      int mylayer = 0;
      if (layer >= 8 && m_twodgains.size() == 2) mylayer = 1;
      else if (m_twodgains.size() == 56) mylayer = layer;

      return m_twodgains[mylayer].GetBinContent(dbin, ebin);
    }

    /** Return dE/dx mean value for given DOCA and entrance angle
     * @param layer continuous layer number
     * @param doca distance of closest approach
     * @param enta entrance angle
     */
    double getMean(unsigned int layer, double doca, double enta) const
    {
      if (layer > 56) {
        B2ERROR("Asking for a CDC layer beyond 56!");
        return 0;
      }

      int mylayer = 0;
      if (layer >= 8 && m_twodgains.size() == 2) mylayer = 1;
      else if (m_twodgains.size() == 56) mylayer = layer;

      // assume rotational symmetry
      if (enta < -3.1416 / 2.0) enta += 3.1416 / 2.0;
      if (enta > 3.1416 / 2.0) enta -= 3.1416 / 2.0;

      // iterate bin number by one since TH2F bins start at 1 not 0
      double dbinsize = m_twodgains[mylayer].GetXaxis()->GetBinCenter(2) - m_twodgains[mylayer].GetXaxis()->GetBinCenter(1);
      int dbin = std::floor((doca - m_twodgains[mylayer].GetXaxis()->GetBinLowEdge(1)) / dbinsize) + 1;

      double ebinsize = m_twodgains[mylayer].GetYaxis()->GetBinCenter(2) - m_twodgains[mylayer].GetYaxis()->GetBinCenter(1);
      // int ebin = std::floor((std::sin(enta) - m_twodgains[mylayer].GetYaxis()->GetBinLowEdge(1)) / ebinsize) + 1;
      int ebin = std::floor((enta - m_twodgains[mylayer].GetYaxis()->GetBinLowEdge(1)) / ebinsize) + 1;

      double mean = 1.0;
      if (dbin > 0 && dbin <= m_twodgains[mylayer].GetNbinsX() && ebin > 0 && ebin <= m_twodgains[mylayer].GetNbinsY())
        mean = m_twodgains[mylayer].GetBinContent(dbin, ebin);
      else if (dbin > m_twodgains[mylayer].GetNbinsX() && ebin > 0 && ebin <= m_twodgains[mylayer].GetNbinsY())
        mean = m_twodgains[mylayer].GetBinContent(m_twodgains[mylayer].GetNbinsX(), ebin);
      else
        // B2WARNING("Problem with 2D CDC dE/dx calibration! " << doca << "\t" << dbin << "\t" << std::sin(enta) << "\t" << ebin);
        B2WARNING("Problem with 2D CDC dE/dx calibration! " << doca << "\t" << dbin << "\t" << enta << "\t" << ebin);

      return mean;
    };

  private:
    /** dE/dx gains versus DOCA and entrance angle
    may be different for different layers, so store as a vector
    keep a version number to identify which layers are valid */
    short m_version; /**< version number for 2D correction */
    std::vector<TH2F> m_twodgains; /**< 2D histograms of doca/enta gains, layer dependent */

    ClassDef(CDCDedx2DCell, 5); /**< ClassDef */
  };
} // end namespace Belle2
