/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <TObject.h>
#include <framework/logging/Logger.h>
#include <TH2F.h>

namespace Belle2 {

//---------------------------------------------------
  /** DB object to store the optimal number of crystals to be used in a cluster energy sum, and the corresponding corrections for bias and contained (raw) energy. Found for 8 test energies. Crystals are typically collected into 8 groups per thetaID, except for a couple of rings in the endcaps, where there are 16 groups. */
  class ECLnOptimal: public TObject {

//---------------------------------------------------
  public:

    /** Constructor. */
    ECLnOptimal() :
      m_eUpperBoundariesFwd(),
      m_eUpperBoundariesBrl(),
      m_eUpperBoundariesBwd(),
      m_groupNumber(),
      m_nOptimal(),
      m_peakFracEnergy(),
      m_bias(),
      m_logPeakEnergy()
    { };

    //---------------------------------------------------
    //..Getters for the various calibration components

    /** Get vector of energies that specify energy ranges in the forward endcap */
    const std::vector<float>& getUpperBoundariesFwd() const {return m_eUpperBoundariesFwd;}

    /** Get vector of energies that specify energy ranges in the barrel */
    const std::vector<float>& getUpperBoundariesBrl() const {return m_eUpperBoundariesBrl;}

    /** Get vector of energies that specify energy ranges in the backward endcap */
    const std::vector<float>& getUpperBoundariesBwd() const {return m_eUpperBoundariesBwd;}

    /** Get the vector of group number for each cellID */
    const std::vector<int>& getGroupNumber() const {return m_groupNumber;}

    /** Get 2D histogram of nOptimal (one xBin per group number, one yBin per energy range */
    const TH2F getNOptimal() const {return m_nOptimal;}

    //..The following have 3 xBins per group for energy interpolation.
    //  Value for nominal energy bin is histogram x bin ix = 3*iGroup + 1;
    //  Value for lower energy bin is ix = 3*iGroup + 2;
    //  Value for higher energy bin is ix = 3*iGroup + 3;
    //  In all three cases, the histogram y bin is iy = iEnergy + 1

    /** Get 2D histogram of the peak fraction of contained energy, one yBin
     per energy group, 3 xBins per group number. */
    const TH2F getPeakFracEnergy() const {return m_peakFracEnergy;}

    /** Get the 2D histogram of bias (reconstructed - true) from beam backgrounds, one yBin
     per energy group, 3 xBins per group number. */
    const TH2F getBias() const {return m_bias;}

    /** Get the 2D histogram of log peak energies (GeV), one yBin
     per energy group, 3 xBins per group number. */
    const TH2F getLogPeakEnergy() const {return m_logPeakEnergy;}

    //---------------------------------------------------
    //..Setters for the various components

    /** Set vector of energies that specify energy ranges in the forward endcap */
    void setUpperBoundariesFwd(const std::vector<float>& eUpperBoundariesFwd) {m_eUpperBoundariesFwd = eUpperBoundariesFwd;}

    /** Set vector of energies that specify energy ranges in the barrel */
    void setUpperBoundariesBrl(const std::vector<float>& eUpperBoundariesBrl) {m_eUpperBoundariesBrl = eUpperBoundariesBrl;}

    /** Set vector of energies that specify energy ranges in the backward endcap */
    void setUpperBoundariesBwd(const std::vector<float>& eUpperBoundariesBwd) {m_eUpperBoundariesBwd = eUpperBoundariesBwd;}

    /** Set the vector of group numbers for each crystal */
    void setGroupNumber(const std::vector<int>& groupNumber) {m_groupNumber = groupNumber;}

    /** Set the 2D histogram of nOptimal */
    void setNOptimal(const TH2F& nOptimal) {m_nOptimal = nOptimal;}

    /** Set the 2D histogram of peak fractional contained energy */
    void setPeakFracEnergy(const TH2F& peakFracEnergy) {m_peakFracEnergy = peakFracEnergy;}

    /** Set the 2D histogram of beam background bias (reconstructed - true) (GeV) */
    void setBias(const TH2F& bias) {m_bias = bias;}

    /** Set the 2D histogram of log of peak contained energy in GeV */
    void setLogPeakEnergy(const TH2F& logPeakEnergy) {m_logPeakEnergy = logPeakEnergy;}

//---------------------------------------------------
  private:
    std::vector<float> m_eUpperBoundariesFwd; /**< upper edges of the energy ranges in the forward endcap (GeV) */
    std::vector<float> m_eUpperBoundariesBrl; /**< upper edges of the energy ranges in the barrel (GeV) */
    std::vector<float> m_eUpperBoundariesBwd; /**< upper edges of the energy ranges in the backward endcap (GeV) */
    std::vector<int> m_groupNumber; /**< group number of each crystal */
    TH2F m_nOptimal; /**< 2D histogram containing optimal number of crystals for each group number (x) and energy range (y) */

    //..For the following, there are 3 bins in x for each group.
    //  First contains the value for the specified group number and energy point,
    //  corresponding to the optimal number of crystals.
    //  Second is the value, for that number of crystals, for the specified group
    //  but the next lower energy point.
    //  Third is the value for the next higher energy point.
    TH2F m_peakFracEnergy; /**< 2D histogram of peak fractional contained energy */
    TH2F m_bias; /**< 2D histogram of bias = sum of ECLCalDigit energy minus true (GeV) */
    TH2F m_logPeakEnergy; /**< log of peak contained energy in GeV */

    ClassDef(ECLnOptimal, 1); /**< ClassDef */
  };
}
