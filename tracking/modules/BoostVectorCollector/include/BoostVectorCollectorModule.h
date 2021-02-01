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

    double m_mu0_pid; ///< stores PID of first muon
    double m_mu1_pid; ///< stores PID of second muon

    TVector3 m_mu0_p; ///< three momentum of first muon
    TVector3 m_mu1_p; ///< three momentum of second muon

    /** Name of the Y4S particle list */
    std::string m_Y4SPListName = "";

  };

} // end namespace Belle2

