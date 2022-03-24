/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <calibration/CalibrationCollectorModule.h>

#include <limits>
#include <vector>

namespace Belle2 {
  /**
   * This collects the B-meson properties in the hadronic B-decays
   * It is used for the Ecms calibration in AirFlow
   */

  /// NaN with double precision for dummy entry
  static const double realNaN = std::numeric_limits<double>::quiet_NaN();


  /// Collector for the collision energy calibration based on the hadronic modes
  class EcmsCollectorModule : public CalibrationCollectorModule {

  public:
    /**
     * Constructor
     */
    EcmsCollectorModule();

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

    void resize(int n);  ///< resize the event members to n candidates

    int m_exp = -1;          ///< experiment number
    int m_run = -1;          ///< run number
    int m_evt = -1;          ///< event number
    double m_time = realNaN; ///< event time [hours]

    std::vector<double> m_pBcms = {};   ///< B mesons CMS momentum
    std::vector<double> m_mB  = {};     ///< B mesons mass
    std::vector<int>    m_pdg = {};      ///< B meson PDG code (can neutral or charged)
    std::vector<int>    m_mode = {};     ///< decay mode ID
    std::vector<double> m_Kpid = {};    ///< Kaon PID
    std::vector<double> m_R2 = {};      ///< the R2 variable used for the continuum suppression
    std::vector<double> m_mD = {};      ///< D meson mass
    std::vector<double> m_dmDstar = {}; ///< D*-D0 mass

  };

} // end namespace Belle2

