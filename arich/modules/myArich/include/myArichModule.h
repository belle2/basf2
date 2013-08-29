/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dino Tahirovic                                           *
 *                                                                        *
 * The purpose of this module is to test the reconstruction of            *
 * the particles with ARICH. It reconstructs particles using              *
 * the datastore arrays Tracks, EXTHit and ARICHLikelihood. To find       *
 * the true values, it searches for the relations from these arrays       *
 * to MCParticles.                                                        *
 * As a result, it saves root ntuple object with relevant data            *
 * which can be used in root script for final analysis.                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MYARICHMODULE_H
#define MYARICHMODULE_H

#include <framework/core/Module.h>
#include <string>

// ROOT
#include <string>
#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TVector3.h>

namespace Belle2 {

  /**
   *
   */
  class myArichModule : public Module {

  public:

    /**
     * Constructor
     */
    myArichModule();

    /**
     * Destructor
     */
    virtual ~myArichModule();

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

    Int_t m_eventNo; /**< Number of event in Tracks */
    Int_t m_trackNo; /**< Number of tracks(hits) per event */

    std::string m_outputFile; /**< output root file */

    /** Input particles from Track(GenFit+ext) or AeroHit(GEANT4 simulation) */
    int m_inputTrackType;

    TFile* file; /**< pointer to output root file */
    TTree* tree; /**< pointer to output tree */

    Float_t m_chi2; /**< Chi2 p-value from MCTrackFit */
    Int_t m_pdg; /**< PDG from the MC-generator */
    Int_t m_primary; /**< MC Flag */

    Float_t m_truePosition[3]; /**< Position of aeroHit (GEANT4) */
    Float_t m_position[3]; /**< Position of extrapolated particle */
    Float_t m_trueMomentum[3]; /**< Momentum of aeroHit (GEANT4) */
    Float_t m_momentum[3]; /**< Momentum of extrapolated particle */
    Float_t m_logl[5]; /**< Log likelihood (ARICHReco) */
    Int_t   m_detPhotons; /**< Detected photons in a ring (ARICHReco) */
    Float_t m_expPhotons[5]; /**< Theoretically expected photons (ARICHReco) */

  };

} // Belle2 namespace

#endif
