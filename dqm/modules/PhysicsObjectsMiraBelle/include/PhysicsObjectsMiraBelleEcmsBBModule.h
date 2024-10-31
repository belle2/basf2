/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/core/HistoModule.h>
#include <TH1D.h>

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
  class PhysicsObjectsMiraBelleEcmsBBModule : public HistoModule {

  public:
    /**
     * Constructor
     */
    PhysicsObjectsMiraBelleEcmsBBModule();

    /**
     * Initialize the histograms
     */
    void defineHisto() override final;

    /**
     * Register the histograms
     */
    void initialize() override final;

    /**
     * Reset the histograms
     */
    void beginRun() override final;

    /**
     * Event processor
     * Filling of the histograms
     */
    void event() override final;

  private:


    TH1D* m_hB0 = nullptr; ///< EB* histogram for neutral B
    TH1D* m_hBp = nullptr; ///< EB* histogram for charged B

    /** Trigger identifier string used to select events for the histograms */
    std::string m_triggerIdentifier = "";

    std::string m_BmListName = ""; ///< List name for charged B candidates
    std::string m_B0ListName = ""; ///< List name for neutral B candidates


  };

} // end namespace Belle2

