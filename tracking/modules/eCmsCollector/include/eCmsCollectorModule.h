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
#include <limits>

namespace Belle2 {
  /**
   * This collects the track parameters and momenta of the mu+mu- events for
   * calibration of the eCms using CAF and AirFlow
   */

  static const double realNaN = std::numeric_limits<double>::quiet_NaN();
  static const int intNaN     = std::numeric_limits<int>::quiet_NaN();


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

    int m_exp = intNaN;     ///< experiment number
    int m_run = intNaN;     ///< run number
    int m_evt = intNaN;     ///< event number
    double m_time = realNaN; ///< event time [hours]

    double m_mBC = realNaN;     ///< mBC mass
    double m_deltaE = realNaN;  ///< deltaE
    int    m_pdg = intNaN;      ///< B meson PDG code (can neutral or charged)
    int    m_mode = intNaN;     ///< decay mode ID
    double m_Kpid = realNaN;    ///< Kaon PID
    double m_R2 = realNaN;
    double m_mD = realNaN;      ///< D meson mass
    double m_dmDstar = realNaN; ///< D*-D0 mass
    //double m_cmsE;    ///< input CMS energy

  };

} // end namespace Belle2

