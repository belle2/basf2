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

    void defineHisto() override final;

    void initialize() override final;
    /**
     * Initialize the module
     */
    void beginRun() override final;

    /**
     * Event processor
     * The filling of the tree
     */
    void event() override final;

  private:


    TH1D* m_hB0 = nullptr;
    TH1D* m_hBp = nullptr;

  };

} // end namespace Belle2

