/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <calibration/CalibrationCollectorModule.h>

#include <TVector3.h>

#include <string>

namespace Belle2 {
  /**
   * This collects the track parameters and momenta of the mu+mu- events for
   * calibration of the eCms using CAF and AirFlow
   */
  class eCmsCollectorModule : public CalibrationCollectorModule {

  public:
    /**
     * Constructor
     */
    eCmsCollectorModule();

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

    int m_exp;     ///< experiment number
    int m_run;     ///< run number
    int m_evt;     ///< event number
    double m_time; ///< event time [hours]

    double m_mBC;     ///< mBC mass
    double m_deltaE;  ///< deltaE
    int    m_pdg;     ///< B meson PDG code (can neutral or charged)
    int    m_mode;    ///< decay mode ID
    double m_Kpid;    ///< Kaon PID
    double m_R2;
    double m_mD;      ///< D meson mass
    double m_dmDstar; ///< D*-D0 mass
    //double m_cmsE;    ///< input CMS energy

  };

} // end namespace Belle2

