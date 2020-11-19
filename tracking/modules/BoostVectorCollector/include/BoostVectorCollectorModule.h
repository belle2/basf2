/***************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                      *
 * Copyright(C) 2020 - Belle II Collaboration                              *
 *                                                                         *
 * Author: The Belle II Collaboration                                      *
 * Contributors: Radek Zlebcik
 *                                                                         *
 *                                                                         *
 * This software is provided "as is" without any warranty.                 *
 ***************************************************************************/
#include <calibration/CalibrationCollectorModule.h>

#include <TVector3.h>

#include <string>

namespace Belle2 {
  /**
   * This collects the track parameters and momenta of the mu+mu- events for
   * calibration of the BoostVector using CAF and AirFlow
   */
  class BoostVectorCollectorModule : public CalibrationCollectorModule {

  public:
    /**
     * Constructor
     */
    BoostVectorCollectorModule();

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

    int m_exp; ///< experiment number
    int m_run; ///< run number
    int m_evt; ///< event number


    double m_time; ///< event time

    double m_mu0_d0, m_mu0_z0, m_mu0_phi0, m_mu0_tanlambda, m_mu0_omega;
    double m_mu1_d0, m_mu1_z0, m_mu1_phi0, m_mu1_tanlambda, m_mu1_omega;

    double m_mu0_pid;
    double m_mu1_pid;

    TVector3 m_mu0_p, m_mu1_p;

    /** Name of the Y4S particle list */
    std::string m_Y4SPListName = "";

  };

} // end namespace Belle2

