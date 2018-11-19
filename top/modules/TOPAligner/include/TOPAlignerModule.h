/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <top/reconstruction/TOPalign.h>

#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPRecBunch.h>
#include <framework/gearbox/Const.h>

#include <string>

#include "TFile.h"
#include "TTree.h"

namespace Belle2 {

  /**
   * Alignment of TOP
   */
  class TOPAlignerModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPAlignerModule();

    /**
     * Destructor
     */
    virtual ~TOPAlignerModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun() override;

    /**
     * Event processor.
     */
    virtual void event() override;

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun() override;

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate() override;

  private:

    /**
     * track selection
     * @return true if pass selection criteria
     */
    bool selectTrack(const TOP::TOPtrack& trk);

    // module paramenets
    double m_minBkgPerBar; /**< minimal assumed background photons per module */
    double m_scaleN0; /**< scale factor for figure-of-merit N0 */
    int m_targetMid; /**< target module to align. Must be 1 <= Mid <= 16 */
    int m_maxFails; /**< maximum allowed number of failed iterations */
    std::string m_sample; /**< sample type */
    double m_minMomentum; /**< minimal track momentum if sample is "cosmics" */
    double m_deltaEcms; /**< c.m.s energy window if sample is "dimuon" or "bhabha" */
    double m_dr; /**< cut on POCA in r */
    double m_dz; /**< cut on POCA in z */
    double m_minZ; /**< minimal local z of extrapolated hit */
    double m_maxZ; /**< maximal local z of extrapolated hit */
    std::string m_outFileName; /**< Root output file name containing results */
    double m_stepPosition; /**< step size for translations */
    double m_stepAngle; /**< step size for rotations */
    double m_stepTime; /**< step size for t0 */
    double m_stepRefind; /**< step size for scaling of refractive index (dn/n) */
    std::vector<int> m_gridSize; /**< grid size */
    std::vector<double> m_parInit; /**< initial parameter values */
    std::vector<std::string> m_parFixed; /**< names of parameters to be fixed */

    // alignment procedure
    TOP::TOPalign m_align;     /**< alignment object */
    Const::ChargedStable m_chargedStable = Const::muon; /**< track hypothesis */
    int m_countFails = 0;      /**< counter for failed iterations */

    // output
    TFile* m_file = 0;                 /**< TFile */
    TTree* m_alignTree = 0;            /**< TTree containing alignment parameters */
    // tree variables
    int m_iter = 0;  /**< iteration counter */
    int m_ntrk = 0;                      /**< number of tracks used */
    int m_errorCode = 0;                 /**< error code of the alignment procedure */
    std::vector<float> m_vAlignPars;     /**< alignment parameters */
    std::vector<float> m_vAlignParsErr;  /**< error on alignment parameters */
    bool m_valid = false;  /**< true if alignment parameters are valid */
    float m_x = 0; /**< track: extrapolated hit coordinate in local (module) frame */
    float m_y = 0; /**< track: extrapolated hit coordinate in local (module) frame */
    float m_z = 0; /**< track: extrapolated hit coordinate in local (module) frame */
    float m_p = 0; /**< track: extrapolated hit momentum in local (module) frame */
    float m_theta = 0; /**< track: extrapolated hit momentum in local (module) frame */
    float m_phi = 0; /**< track: extrapolated hit momentum in local (module) frame */
    float m_pocaR = 0; /**< r of POCA */
    float m_pocaZ = 0; /**< z of POCA */
    float m_pocaX = 0; /**< x of POCA */
    float m_pocaY = 0; /**< y of POCA */
    float m_cmsE = 0; /**< c.m.s. energy if dimuon or bhabha */
    int m_charge = 0; /**< track charge */
    int m_PDG = 0; /**< track MC truth (simulated data only) */

    // datastore objects
    StoreArray<TOPDigit> m_digits; /**< collection of digits */
    StoreArray<Track> m_tracks;    /**< collection of tracks */
    StoreArray<ExtHit> m_extHits;  /**< collection of extrapolated hits */
    StoreObjPtr<TOPRecBunch> m_recBunch; /**< reconstructed bunch */

  };

} // Belle2 namespace

