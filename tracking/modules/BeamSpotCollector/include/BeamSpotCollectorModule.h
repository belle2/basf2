/***************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                      *
 * Copyright(C) 2017 - Belle II Collaboration                              *
 *                                                                         *
 * Author: The Belle II Collaboration                                      *
 * Contributors: Radek Zlebcik
 *                                                                         *
 *                                                                         *
 * This software is provided "as is" without any warranty.                 *
 ***************************************************************************/
#include <calibration/CalibrationCollectorModule.h>

#include <string>

namespace Belle2 {
  /**
   * This collects the track parameters of the mu+mu- events for
   * calibration of the BeamSpot using CAF and AirFlow
   */
  class BeamSpotCollectorModule : public CalibrationCollectorModule {

  public:
    /**
     * Constructor
     */
    BeamSpotCollectorModule();

    /**
     * Initialize the module
     */
    void prepare() override final;

    /**
     * Event processor
     * The filling of the tree
     */
    void collect() override final;

  private:

    /** eventNumber, expNum, runNum */
    int m_evt, m_exp, m_run;
    /** event time */
    double m_time;
    /** track parameters of the first and second mu */
    double m_mu0_d0, m_mu0_z0, m_mu0_phi0, m_mu0_tanlambda, m_mu0_omega;
    double m_mu1_d0, m_mu1_z0, m_mu1_phi0, m_mu1_tanlambda, m_mu1_omega;

    /** Name of the Y4S particle list */
    std::string m_Y4SPListName = "";

  };

} // end namespace Belle2

