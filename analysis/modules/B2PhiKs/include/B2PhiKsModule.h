/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Susanne Koblitz                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef B2PHIKSMODULE_H
#define B2PHIKSMODULE_H

#include <framework/core/Module.h>

#include <string>
#include <vector>

#include <generators/dataobjects/MCParticle.h>

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


//stuff for root output
#include <TTree.h>
#include <TFile.h>
#include <TLorentzVector.h>
#include <TVector3.h>

namespace Belle2 {

  class B2PhiKsModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    B2PhiKsModule();

    /** Destructor. */
    virtual ~B2PhiKsModule() {}

    /** Initializes the module. */
    virtual void initialize();

    virtual void terminate();

    /** Method is called for each event. */
    virtual void event();

  protected:
    std::string  m_mcParticlesColName;         /**< MCParticles collection name. */
    std::string  m_gfTrackCandColName;         /**< Track Candidates collection name. */
    std::string  m_gfTracksColName;            /**< GFTracks collection name. */
    std::string  m_tracksColName;              /**< Tracks collection name. */

    /** output root file name (given as Module parameter) */
    std::string m_dataOutFileName;

    /** Root tree and file for saving the output */
    TTree* m_tree;
    TFile* m_rootFile;

    // intermediate variables
    double m_sigDecayTime;
    double m_tagDecayTime;
    int    m_tagTag;

    // MCParticles for the signal decay chain
    MCParticle* B0Sig;
    MCParticle* PhiSig;
    MCParticle* KpSig;
    MCParticle* KmSig;
    MCParticle* KshortSig;
    MCParticle* PipSig;
    MCParticle* PimSig;
    MCParticle* B0Tag;

    // boost vector for the CMS frame == Y(4S) frame
    TVector3 boostToCMS;
    double cmsE;

    // output variables
    // generator info
    int m_genBtag;
    double m_genB_deltat;
    double m_genB_ptot;
    double m_genPhi_ptot;
    double m_genKs_ptot;
    double m_genKs_pt;
    double m_genKs_pz;

    double m_genPhi_helicity;

    double m_genKp_ptot;
    double m_genKp_pt;
    double m_genKp_pz;
    double m_genKp_phi;
    double m_genKp_costh;
    double m_genKm_ptot;
    double m_genKm_pt;
    double m_genKm_pz;
    double m_genKm_phi;
    double m_genKm_costh;

    double m_genPip_ptot;
    double m_genPip_pt;
    double m_genPip_pz;
    double m_genPip_costh;
    double m_genPim_ptot;
    double m_genPim_pt;
    double m_genPim_pz;
    double m_genPim_costh;

    double m_genKs_fl;

    // reconstructed info related
    int m_kpTrackFound;
    int m_kmTrackFound;
    int m_pipTrackFound;
    int m_pimTrackFound;

    int m_kpTrackFitted;
    int m_kmTrackFitted;
    int m_pipTrackFitted;
    int m_pimTrackFitted;

    int m_pipTrackExtrapolated;
    int m_pimTrackExtrapolated;

    int m_kpTrackCDCRecoHits;
    int m_kmTrackCDCRecoHits;
    int m_pipTrackCDCRecoHits;
    int m_pimTrackCDCRecoHits;

    int m_kpTrackSVDRecoHits;
    int m_kmTrackSVDRecoHits;
    int m_pipTrackSVDRecoHits;
    int m_pimTrackSVDRecoHits;

    int m_kpTrackPXDRecoHits;
    int m_kmTrackPXDRecoHits;
    int m_pipTrackPXDRecoHits;
    int m_pimTrackPXDRecoHits;

    int m_kpTrackCDCHits;
    int m_kmTrackCDCHits;
    int m_pipTrackCDCHits;
    int m_pimTrackCDCHits;

    int m_kpTrackSVDHits;
    int m_kmTrackSVDHits;
    int m_pipTrackSVDHits;
    int m_pimTrackSVDHits;

    int m_kpTrackPXDHits;
    int m_kmTrackPXDHits;
    int m_pipTrackPXDHits;
    int m_pimTrackPXDHits;

    double m_kpTrackD0;
    double m_kmTrackD0;
    double m_pipTrackD0;
    double m_pimTrackD0;

    double m_kpTrackZ0;
    double m_kmTrackZ0;
    double m_pipTrackZ0;
    double m_pimTrackZ0;

    double m_kpTrackPValue;
    double m_kmTrackPValue;
    double m_pipTrackPValue;
    double m_pimTrackPValue;

    double m_kpTOPKPiDeltaLogL;
    double m_kmTOPKPiDeltaLogL;
    double m_pipTOPKPiDeltaLogL;
    double m_pimTOPKPiDeltaLogL;

    double m_kpARICHKPiDeltaLogL;
    double m_kmARICHKPiDeltaLogL;
    double m_pipARICHKPiDeltaLogL;
    double m_pimARICHKPiDeltaLogL;

    TVector3 m_kp3Momentum;
    TVector3 m_km3Momentum;
    TVector3 m_pip3Momentum;
    TVector3 m_pim3Momentum;

    TLorentzVector m_kpLV;
    TLorentzVector m_kmLV;
    TLorentzVector m_pipLV;
    TLorentzVector m_pimLV;

    TLorentzVector m_phiLV;
    TLorentzVector m_ksLV;
    TLorentzVector m_b0LV;

    double m_ksmass;
    double m_ksptot;
    double m_kspt;
    double m_kspz;

    double m_phimass;
    double m_genphimass;

    double m_b0mass;
    double m_mbc;
    double m_deltae;

    TVector3 m_pipPOCA;
    TVector3 m_pimPOCA;

    double m_kp_ptot;
    double m_kp_pt;
    double m_kp_pz;
    double m_kp_costh;
    double m_kp_pterr;
    double m_kp_pzerr;

    double m_km_ptot;
    double m_km_pt;
    double m_km_pz;
    double m_km_pterr;
    double m_km_pzerr;
    double m_km_costh;

    double m_pip_ptot;
    double m_pip_pt;
    double m_pip_pz;
    double m_pip_pterr;
    double m_pip_pzerr;
    double m_pip_costh;

    double m_pim_ptot;
    double m_pim_pt;
    double m_pim_pz;
    double m_pim_pterr;
    double m_pim_pzerr;
    double m_pim_costh;

    double m_ks_mindr;
    double m_ks_maxdr;
    double m_ks_mindz;
    double m_ks_maxdz;

    // Generated info Related functions
    int isBtoPhiKshortDecay(MCParticle* part);
    int isPhiKpKmDecay(MCParticle* part);
    int isKshortPipPimDecay(MCParticle* part);

    void removeFSRPhoton(std::vector<MCParticle*> daughters, std::vector<MCParticle*> &daughtersWithoutFSR);

    void printGenHepevtParticles(std::vector<MCParticle*> partList);
    void dumpParticleGeneratorInfo(MCParticle* part);

    // user functions (root related)
    void initBranches(TTree* tree);
    void initGeneratedInfo();
    void initReconstructedInfo();
    void fillGeneratedInfo(int signalB0Decay, int signalPhiDecay, int signalKshortDecay);
  };

} // end namespace Belle2

#endif // B2PHIKSMODULE_H
