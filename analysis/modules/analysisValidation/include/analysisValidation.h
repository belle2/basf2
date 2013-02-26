/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Phillip Urquijo
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ANALYSISVALIDATIONMODULE_H
#define ANALYSISVALIDATIONMODULE_H

#include <framework/core/Module.h>

#include <string>
#include <vector>

#include <generators/dataobjects/MCParticle.h>
#include <analysis/dataobjects/Particle.h>

#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <cdc/dataobjects/CDCSimHit.h>

#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>

// tracking
#include <GFTrack.h>
#include <GFTrackCand.h>
#include <tracking/dataobjects/Track.h>
#include <tracking/dataobjects/TrackFitResult.h>

// ECL
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLPi0.h>
#include <ecl/dataobjects/ECLGamma.h>

//PID
#include <reconstruction/dataobjects/PIDLikelihood.h>

// kinematic fit
#include <analysis/KFit/MassFitKFit.h>
#include <analysis/KFit/MassVertexFitKFit.h>
#include <analysis/KFit/VertexFitKFit.h>

#include <analysis/raveInterface/RaveVertexFitter.h>
#include <analysis/raveInterface/RaveSetup.h>
#include <GFRaveVertexFactory.h>
#include <RKTrackRep.h>

//stuff for root output
#include <TTree.h>
#include <TFile.h>
#include <TLorentzVector.h>
#include <TVector3.h>
#include <TH1I.h>

using namespace Belle2;
using namespace Belle2::analysis;

namespace Belle2 {

  class analysisValidationModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    analysisValidationModule();

    /** Destructor. */
    virtual ~analysisValidationModule() {}

    /** Initializes the module. */
    virtual void initialize();

    virtual void terminate();

    /** Method is called for each event. */
    virtual void event();

    /** Check that the MDST containers relevant for analysis are filled **/
    void checkDSTContents();

    /** Check that the UDST containers relevant for analysis are filled **/
    void checkUDSTContents();

    /** Test Fitters/interfaces **/
    unsigned doKvFit(const Particle* p, double& confLevel);
    unsigned doRaveFit(const Particle* p, double& confLevel);
    unsigned makeMother(VertexFitKFit& kv, Particle* mother);
    void testfitters();

    /** More tests to come for validation ... **/

  protected:
    std::string  m_mcParticlesColName;         /**< MCParticles collection name. */
    std::string  m_gfTrackCandColName;         /**< Track Candidates collection name. */
    std::string  m_gfTracksColName;            /**< GFTracks collection name. */
    std::string  m_tracksColName;              /**< Tracks collection name. */
    std::string  m_ECLgammasColName;           /**< Gammas collection name. */
    std::string  m_eclsColName;                /**< ECLs collection name. */
    std::string  m_pi0sColName;                /**< pi0s collection name. */
    std::string  m_ParticlesColName;           /**< Particles collection name. */
    std::string  m_TestType;                   /**< Analysis test type */

    /** output root file name (given as Module parameter) */
    std::string m_dataOutFileName;

    /** Root tree and file for saving the output */
    TFile* m_rootFile;
    //
    TH1I* h_nGFTracks;
    TH1I* h_nTracks;
    TH1I* h_nECLGammas;
    TH1I* h_nECLShowers;
    TH1I* h_nMCParticles;
    TH1I* h_nParticles;

    TH1I* h_nmcphotons;  // To be added

    unsigned int m_nTracks;
    unsigned int m_nECLgammas;
    unsigned int m_nMCParticles;
    unsigned int m_event;
    unsigned int m_nmcphotons;

    unsigned int nevt;

    double m_hmasskfit;
    double m_hmassrfit;
  };

} // end namespace Belle2

#endif // B2DPIMODULE_H
