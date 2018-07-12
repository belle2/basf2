/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric, Marko Staric                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/gearbox/Const.h>
#include <string>
// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
// Hit classes
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPRecBunch.h>
#include <top/dataobjects/TOPBarHit.h>
#include <top/dataobjects/TOPLikelihood.h>
#include <top/dataobjects/TOPPull.h>


namespace Belle2 {

  /**
   * TOP reconstruction module.
   */
  class TOPReconstructorModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPReconstructorModule();

    /**
     * Destructor
     */
    virtual ~TOPReconstructorModule();

    /**
     * Initialize the Module.
     *
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     *
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun();

    /**
     * Event processor.
     *
     */
    virtual void event();

    /**
     * End-of-run action.
     *
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action.
     *
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();


  private:

    // Module steering parameters
    double m_minBkgPerBar = 0;    /**< minimal assumed background photons per bar */
    double m_scaleN0 = 0;         /**< scale factor for N0 */
    double m_sigmaRphi = 0;    /**< track smearing in Rphi (r.m.s) */
    double m_sigmaZ = 0;       /**< track smearing in Z (r.m.s) */
    double m_sigmaTheta = 0;   /**< track smearing in Theta (r.m.s) */
    double m_sigmaPhi = 0;     /**< track smearing in Phi (r.m.s) */
    double m_minTime = 0;      /**< optional lower time limit for photons */
    double m_maxTime = 0;      /**< optional upper time limit for photons */
    int m_PDGCode = 0;   /**< PDG code of hypothesis to construct pulls */

    // others
    int m_debugLevel = 0;       /**< debug level from logger */
    bool m_smearTrack = false;  /**< set to true, if at least one sigma > 0 */

    // Masses of particle hypotheses

    double m_masses[Const::ChargedStable::c_SetSize] = {0};  /**< particle masses */
    int m_pdgCodes[Const::ChargedStable::c_SetSize] = {0};   /**< particle codes */

    // datastore objects

    StoreArray<TOPDigit> m_digits; /**< collection of digits */
    StoreArray<Track> m_tracks; /**< collection of tracks */
    StoreArray<ExtHit> m_extHits; /**< collection of extrapolated hits */
    StoreArray<TOPBarHit> m_barHits; /**< collection of bar hits */
    StoreObjPtr<TOPRecBunch> m_recBunch; /**< reconstructed bunch */
    StoreArray<TOPLikelihood> m_likelihoods; /**< collection of likelihoods */
    StoreArray<TOPPull> m_topPulls; /**< collection of pulls */

  };

} // Belle2 namespace

