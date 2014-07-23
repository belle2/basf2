/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dino Tahirovic                                           *
 *                                                                        *
 * The purpose of this module is to test the reconstruction of            *
 * the particles with ARICH.                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHANALYSISMODULE_H
#define ARICHANALYSISMODULE_H

#include <framework/core/Module.h>
#include <string>

// ROOT
#include <string>
#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TVector3.h>

namespace Belle2 {

  /** ARICH reconstruction efficiency test module
    *
    * The purpose of this module is to test the reconstruction of
    * the particles with ARICH. There are two paths (parameter inputTrackType),
    * depending on the input tracks,
    * which can either be obtained from the tracking subdetectors
    * or from GEANT4 simulation directly, without taking into account the inner structure
    * of the Belle II detector. In the case of tracking, the input are
    * fitted tracks, extrapolated hits and Monte Carlo information.
    * The output is a root ntuple object (parameter fileName) with the true and
    * reconstructed position of the track, the true and reconstructed momentum,
    * likelihood value and number of expected photons for each of the five hypotheses
    * and number of detected photons.
    * In the case of GEANT4 simulation only, the input are hits at the aerogel
    * (datastore ntuple ARICHAeroHits). The output is the same as in tracking case,
    * only without the reconstructed values.
    * The output ntuple is used in root scripts
    * (examples/resolutions.C, examples/extArichEfficiency.C)
    * for the final analysis.

    */
  class ARICHAnalysisModule : public Module {

  public:

    /**
     * Constructor
     */
    ARICHAnalysisModule();

    /**
     * Destructor
     */
    virtual ~ARICHAnalysisModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun();

    /**
     * Event processor.
     */
    virtual void event();

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

    /**
     * Prints module parameters.
     */
    void printModuleParams() const;




  private:
    int m_eventNo; /**< Number of event in Tracks */
    int m_trackNo; /**< Number of tracks(hits) per event */

    std::string m_outputFile; /**< output root file */

    TFile* file; /**< pointer to output root file */
    TTree* tree; /**< pointer to output tree */

    Float_t m_chi2; /**< Chi2 p-value from MCTrackFit */
    Int_t m_pdg; /**< PDG from the MC-generator */
    Int_t m_mpdg; /**< PDG of the mother particle */
    Int_t m_status; /**< m_status=1 if MC info was filled, m_status+=10 if likelihood info was filled, m_status+=100 for extHit info */
    Int_t m_flag; /**< MC flag, as defined in MCParticle.h */
    Float_t m_prodVertex[3]; /**< production vertex. */

    Float_t m_truePosition[3]; /**< Position of aeroHit (GEANT4) */
    Float_t m_position[3]; /**< Position of extrapolated particle */
    Float_t m_trueMomentum[3]; /**< Momentum of aeroHit (GEANT4) */
    Float_t m_momentum[3]; /**< Momentum of extrapolated particle */
    Float_t m_logl[5]; /**< Log likelihood (ARICHReco) */
    Int_t   m_detPhotons[5]; /**< Detected photons in a ring (ARICHReco) */
    Float_t m_expPhotons[5]; /**< Theoretically expected photons (ARICHReco) */

  };

} // Belle2 namespace

#endif
