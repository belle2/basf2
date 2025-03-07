/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
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

    int m_evt; ///< event number
    int m_exp; ///< experiment number
    int m_run; ///< run number

    double m_time; ///< event time


    double m_mu0_d0;       ///< d0 track parameter of the first muon in the event
    double m_mu0_z0;       ///< z0 track parameter of the first muon in the event
    double m_mu0_phi0;     ///< phi0 track parameter of the first muon in the event
    double m_mu0_tanlambda;///< tanlambda track parameter of the first muon in the event
    double m_mu0_omega;    ///< omega track parameter of the first muon in the event

    double m_mu1_d0;       ///< d0 track parameter of the second muon in the event
    double m_mu1_z0;       ///< z0 track parameter of the second muon in the event
    double m_mu1_phi0;     ///< phi0 track parameter of the second muon in the event
    double m_mu1_tanlambda;///< tanlambda track parameter of the second muon in the event
    double m_mu1_omega;    ///< omega track parameter of the second muon in the event


    /** Name of the Y4S particle list */
    std::string m_Y4SPListName = "";

  };

} // end namespace Belle2

