/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* Belle 2 headers. */
#include <framework/gearbox/Const.h>
#include <framework/core/Module.h>

/* C++ headers. */
#include <string>
#include <vector>

namespace Belle2 {

  class TrackExtrapolateG4e;

  /**
   * The geant4e-based track extrapolation module.
   *
   * This module extrapolates tracks outward from the outer perimeter of the CDC
   * using geant4e.
   *
   * This module requires a valid geometry in memory (gGeoManager). Therefore,
   * a geometry building module should have been executed before this module is called.
   *
   */

  class ExtModule : public Module {

  public:

    //! constructor
    ExtModule();

    //! destructor
    ~ExtModule() override;

    //! Initialize for track extrapolation
    void initialize() override;

    //! Perform beginning-of-run actions
    void beginRun() override;

    //! Performs track extrapolation for all tracks in one event
    void event() override;

    //! Perform end-of-run actions
    void endRun() override;

    //! Terminates the module
    void terminate() override;

  protected:

    //! User-selected PDG codes to extrapolate (anti-particles are included implicitly)
    std::vector<int> m_PDGCodes;

    //!  ChargedStable hypotheses
    std::vector<Const::ChargedStable> m_Hypotheses;

    //! User-defined minimum transverse momentum in GeV/c for extrapolation to be started
    double m_MinPt;

    //! User-defined minimum kinetic energy in GeV for extrapolation to continue
    double m_MinKE;

    //! User-define maximum step size in cm (0 for no upper limit)
    double m_MaxStep;

    //! User-defined tracking verbosity:
    //! 0=Silent; 1=Min info per step; 2=sec particles; 3=pre/post step info; 4=like 3 but more info; 5=proposed step length info
    int m_TrackingVerbosity;

    //! User-defined list of Geant4 UI commands that should be applied before the extrapolation starts
    std::vector<std::string> m_UICommands;

    //! User-defined Geant4 visualization support: true to enable
    bool m_EnableVisualization;

    //! User-defined magnetic field stepper to use
    std::string m_MagneticFieldStepperName;

    //! User-defined minimal distance for magnetic field lookup. If distance is smaller, return cached value
    double m_MagneticCacheDistance;

    //! User-defined maximum miss-distance between the trajectory curve and its linear chord(s) approximation
    double m_DeltaChordInMagneticField;

  private:

    //! Pointer to the TrackExtrapoleG4e singleton
    TrackExtrapolateG4e* m_Extrapolator;

  };

} // end of namespace Belle2
