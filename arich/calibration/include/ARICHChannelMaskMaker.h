/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {
  /**
   * ARICH channel mask calibration algorithm
   */
  class ARICHChannelMaskMaker : public CalibrationAlgorithm {
  public:

    /// Constructor set the prefix to TestCalibration
    ARICHChannelMaskMaker();

    /// setter for m_minFrac
    void setMinFrac(double minFrac) {m_minFrac = minFrac;}

    /// setter for m_minS2N
    void setMinS2N(double minS2N) {m_minS2N = minS2N;}

    /// setter for m_minHitPerChn
    void setMinHitPerChn(double minHitPerChn) {m_minHitPerChn = minHitPerChn;}

    /// getter for m_minFrac
    double getMinFrac() {return m_minFrac;}

    /// getter for m_minS2N
    double getMinS2N() {return m_minS2N;}

    /// getter for m_minHitPerChn
    double getMinHitPerChn() {return m_minHitPerChn;}

  protected:

    /// Run algo on data
    virtual EResult calibrate() override;
    /// get hapd ring number from moduleID
    int getRing(int modID);

  private:

    /// Minimal number of signal hits (bit 1+2-0-3) in channel / average number of signal hits in channel in the corresponding HAPD ring
    double m_minFrac = 0.3;
    /// Minimal signal/(signal+noise) for channel (for hot channels)
    double m_minS2N = 0.1;
    /// Minimal number of hits in the channel in the outter HAPD ring, collect more data if not satisfied
    double m_minHitPerChn = 30;

  };
} // namespace Belle2
