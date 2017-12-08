/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj, Dino Tahirovic                             *
 *                                                                        *
 * The purpose of this module is to test the reconstruction of            *
 * the particles with ARICH.                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHNTUPLEMODULE_H
#define ARICHNTUPLEMODULE_H

#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/MCParticle.h>

#include <arich/dataobjects/ARICHLikelihood.h>
#include <arich/dataobjects/ARICHAeroHit.h>
#include <arich/dataobjects/ARICHTrack.h>


// ROOT
#include <string>
#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <arich/modules/arichNtuple/ARICHNtupleStruct.h>

namespace Belle2 {

  /**
   *  ARICH reconstruction efficiency test module
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
  class ARICHNtupleModule : public Module {

  public:

    /**
     * Constructor
     */
    ARICHNtupleModule();

    /**
     * Destructor
     */
    virtual ~ARICHNtupleModule();

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

  private:

    // required input
    StoreArray<ARICHTrack> m_arichTracks; /**< Required array of input ARICHTracks */
    StoreArray<ARICHLikelihood> m_arichLikelihoods; /**< Required array of input ARICHLikelihoods */

    // optional input
    StoreArray<Track> m_tracks; /**< Optional input array of Tracks */
    StoreArray<MCParticle> m_arichMCPs; /**< Optional input array of MCParticles */
    StoreArray<ARICHAeroHit> m_arichAeroHits; /**< Optional input array of ARICHAeroHits */

    std::string m_outputFile; /**< output root file */

    TFile* m_file; /**< pointer to output root file */
    TTree* m_tree; /**< pointer to output tree */

    ARICH::ARICHTree m_arich; /**< ntuple structure */
  };

} // Belle2 namespace

#endif
