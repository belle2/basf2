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
#include <analysis/dataobjects/Particle.h>

#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <cdc/dataobjects/CDCSimHit.h>

// What is the difference,
// between RelationArray and RelationIndex
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>

// tracking
#include <genfit/Track.h>
#include <genfit/TrackCand.h>
#include <tracking/dataobjects/Track.h>
#include <tracking/dataobjects/TrackFitResult.h>

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

    /** Do a vertex fit **/
    unsigned doKvFit(const Particle* p, double& confLevel);
    /** Do a mass constrained fit **/
    unsigned doKmFit(Particle& p, double& confLevel);
    /** Do a mass-vertex constrained fit **/
    unsigned doKmvFit(const Particle* p, double& confLevel);
    /** Get mother for a mass-vertex constrained fit **/
    unsigned makeMother(MassVertexFitKFit& km, Particle* mother);
    /** Get mother for a vertex constrained fit **/
    unsigned makeMother(VertexFitKFit& kv, Particle* mother);

    /** Generated info Related functions **/
    int isBtoD0PiDecay(MCParticle* part);
    /** Check if this is a true D->Kpi decay **/
    int isD0Decay(MCParticle* part);
    /** Remove the FSR photon from daughter list **/
    void removeFSRPhoton(std::vector<MCParticle*> daughters, std::vector<MCParticle*>& daughtersWithoutFSR);

  protected:
    std::string  m_mcParticlesColName;     /**< MCParticles collection name. */
    std::string  m_gfTrackCandColName;     /**< Track Candidates collection name. */
    std::string  m_gfTracksColName;        /**< genfit::Tracks collection name. */
    std::string  m_tracksColName;          /**< Tracks collection name. */
    std::string  m_gammasColName;          /**< Gammas collection name. */
    std::string  m_eclsColName;            /**< ECLs collection name. */
    std::string  m_pi0sColName;            /**< pi0s collection name. */

    /** output root file name (given as Module parameter) */
    std::string m_dataOutFileName;

    /** Root tree for saving the output */
    TTree* m_tree;
    /** file for the root tree **/
    TFile* m_rootFile;
    /** D mass variable **/
    double m_md;
    /** Mbc variable **/
    double m_mbc;
    /** DeltaE variable **/
    double m_deltae;
    /** D mass after kinematic fit **/
    double m_md_fit;
    /** Mbc variable after kinematic fit **/
    double m_mbc_fit;
    /** DeltaE variable after kinematic fit **/
    double m_deltae_fit;
    /** CM energy variable **/
    double m_cmse;
    /** B candidates list size **/
    double m_bsize;
    /** x-coordinate of B candidate vertex **/
    double m_bp_vx;
    /** y-coordinate of B candidate vertex **/
    double m_bp_vy;
    /** z-coordinate of B candidate vertex **/
    double m_bp_vz;
    /** x-error of B candidate vertex **/
    double m_bp_evx;
    /** y-error of B candidate vertex **/
    double m_bp_evy;
    /** z-error of B candidate vertex **/
    double m_bp_evz;
    /** x-coordinate of vertex of true B **/
    double m_gen_bp_vx;
    /** y-coordinate of vertex of true B **/
    double m_gen_bp_vy;
    /** z-coordinate of vertex of true B **/
    double m_gen_bp_vz;
    /** chi2 of B candidate vertex **/
    double m_bp_chi2;
    /** x-coordinate of D candidate vertex **/
    double m_d0_vx;
    /** y-coordinate of D candidate vertex **/
    double m_d0_vy;
    /** z-coordinate of D candidate vertex **/
    double m_d0_vz;
    /** x-error of D candidate vertex **/
    double m_d0_evx;
    /** y-error of D candidate vertex **/
    double m_d0_evy;
    /** z-error of D candidate vertex **/
    double m_d0_evz;
    /** x-coordinate of vertex of true D **/
    double m_gen_d0_vx;
    /** y-coordinate of vertex of true D **/
    double m_gen_d0_vy;
    /** z-coordinate of vertex of true D **/
    double m_gen_d0_vz;
    /** chi2 of D candidate (mass-vertex fit) **/
    double m_d0_chi2;
    /** Number of events read **/
    int nevt;
    /** B candidate 4-vector **/
    TLorentzVector m_b;

    // MCParticles for the signal decay chain
    /** MCParticle for the B signal **/
    MCParticle* BPSig;
    /** MCParticle for the D (daughter of B) signal **/
    MCParticle* D0Sig;
    /** MCParticle for the K (daughter of D) signal **/
    MCParticle* KSig;
    /** MCParticle for the pi (daughter of D) signal **/
    MCParticle* PiSig;
    /**  MCParticle for the prompt pi signal **/
    MCParticle* PrimaryPI;
    /** MCParticle for the other side B (tag side) **/
    MCParticle* BMTag;

    /** boost vector for the CMS frame == Y(4S) frame **/
    TVector3 boostToCMS;
    /** CM energy variable (sqrt(s)/2) **/
    double cmsE;
  };

} // end namespace Belle2

#endif // B2DPIMODULE_H
