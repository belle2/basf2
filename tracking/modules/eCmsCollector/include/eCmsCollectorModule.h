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

    int m_exp; ///< experiment number
    int m_run; ///< run number
    int m_evt; ///< event number


    double m_time; ///< event time

    double m_mBC; ///< mBC mass
    double m_deltaE; ///< deltaE
    int    m_pdg; ///< deltaE
    int    m_mode;
    double m_Kpid;
    double m_R2;
    double m_mD; ///< Dmass
    double m_dmDstar; ///< Dmass


    /** Name of the Y4S particle list */
    std::string m_Y4SPListName = "";

  };

} // end namespace Belle2

