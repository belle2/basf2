/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Luka Santelj                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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

    /// setter for m_minOcc
    void setMinOcc(double minOcc) {m_minOcc = minOcc;}

    /// setter for m_maxOcc
    void setMaxOcc(double maxOcc) {m_maxOcc = maxOcc;}

    /// setter for m_minHitAtMinOcc
    void setMinHitAtMinOcc(double minHitAtMinOcc) {m_minHitAtMinOcc = minHitAtMinOcc;}

    /// getter for m_minOcc
    double getMinOcc() {return m_minOcc;}

    /// getter for m_maxOcc
    double getMaxOcc() {return m_maxOcc;}

    /// getter for m_minHitAtMinOcc
    double getMinHitAtMinOcc() {return m_minHitAtMinOcc;}

  protected:

    /// Run algo on data
    virtual EResult calibrate() override;

  private:

    /// Minimal occupany (for dead channels)
    double m_minOcc = 0.00003;
    /// Maximal occupany (for hot channels)
    double m_maxOcc = 0.005;
    /// Minimal number of hits expected at minimal occupancy, collect more data if not satisfied (m_minOcc*nevents>m_minHitAtMinOcc)
    double m_minHitAtMinOcc = 20.;

  };
} // namespace Belle2
