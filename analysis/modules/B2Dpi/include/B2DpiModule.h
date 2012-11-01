/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tagir Aushev, Karim Trabelsi, Anze Zupanc                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef B2DPIMODULE_H
#define B2DPIMODULE_H

#include <framework/core/Module.h>

#include <string>
#include <vector>

#include <generators/dataobjects/MCParticle.h>
#include "analysis/particle/Particle.h"

#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <cdc/dataobjects/CDCSimHit.h>

#include <top/dataobjects/TOPTrack.h>
#include <top/dataobjects/TOPLikelihoods.h>

// What is the difference,
// between RelationArray and RelationIndex
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>

// tracking
#include <GFTrack.h>
#include <GFTrackCand.h>
#include <tracking/dataobjects/Track.h>

// ECL
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLPi0.h>

// kinematic fit
#include <analysis/KFit/MassFitKFit.h>
#include <analysis/KFit/MassVertexFitKFit.h>
#include <analysis/KFit/VertexFitKFit.h>

//stuff for root output
#include <TTree.h>
#include <TFile.h>
#include <TLorentzVector.h>
#include <TVector3.h>

using namespace Belle2;
using namespace Belle2::analysis;

namespace Belle2 {

  class B2DpiModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    B2DpiModule();

    /** Destructor. */
    virtual ~B2DpiModule() {}

    /** Initializes the module. */
    virtual void initialize();

    virtual void terminate();

    /** Method is called for each event. */
    virtual void event();

    //
    unsigned doKvFit(Particle& p, double& confLevel);
    unsigned doKmFit(Particle& p, double& confLevel);
    unsigned doKmvFit(Particle& p, double& confLevel);

    // Generated info Related functions
    int isBtoD0PiDecay(MCParticle* part);
    int isD0Decay(MCParticle* part);
    void removeFSRPhoton(std::vector<MCParticle*> daughters, std::vector<MCParticle*> &daughtersWithoutFSR);

  protected:
    std::string  m_mcParticlesColName;         /**< MCParticles collection name. */
    std::string  m_gfTrackCandColName;         /**< Track Candidates collection name. */
    std::string  m_gfTracksColName;            /**< GFTracks collection name. */
    std::string  m_tracksColName;              /**< Tracks collection name. */
    std::string  m_gammasColName;              /**< Gammas collection name. */
    std::string  m_eclsColName;                /**< ECLs collection name. */
    std::string  m_pi0sColName;                /**< pi0s collection name. */

    /** output root file name (given as Module parameter) */
    std::string m_dataOutFileName;

    /** Root tree and file for saving the output */
    TTree* m_tree;
    TFile* m_rootFile;

    double m_md;
    double m_mbc;
    double m_deltae;
    double m_md_fit;
    double m_mbc_fit;
    double m_deltae_fit;
    double m_cmse;
    double m_bsize;
    double m_bp_vx;
    double m_bp_vy;
    double m_bp_vz;
    double m_bp_evx;
    double m_bp_evy;
    double m_bp_evz;
    double m_gen_bp_vx;
    double m_gen_bp_vy;
    double m_gen_bp_vz;
    double m_bp_chi2;

    double m_d0_vx;
    double m_d0_vy;
    double m_d0_vz;
    double m_d0_evx;
    double m_d0_evy;
    double m_d0_evz;
    double m_gen_d0_vx;
    double m_gen_d0_vy;
    double m_gen_d0_vz;
    double m_d0_chi2;

    int nevt;

    TLorentzVector m_b;

    // MCParticles for the signal decay chain
    MCParticle* BPSig;
    MCParticle* D0Sig;
    MCParticle* KSig;
    MCParticle* PiSig;
    MCParticle* PrimaryPI;
    MCParticle* BMTag;

    // boost vector for the CMS frame == Y(4S) frame
    TVector3 boostToCMS;
    double cmsE;
  };

} // end namespace Belle2

#endif // B2DPIMODULE_H
