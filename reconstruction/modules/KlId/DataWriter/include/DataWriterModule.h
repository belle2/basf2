/***************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frederik Krohn                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 *************************************************************************/
#ifndef DataWriterModule_H
#define DataWriterModule_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KlId.h>

#include <TTree.h>
#include <TFile.h>
#include <string>

namespace Belle2 {

  /** Module to write Ntuples for KlId BKG classifier training. Writes tuples for both ECL and KLM. You have to specify  an outputpath outPath.   */
  class DataWriterModule : public Module {

  public:

    /** Constructor */
    DataWriterModule();

    /** Destructor */
    virtual ~DataWriterModule();

    /** init */
    virtual void initialize();

    /** beginn run */
    virtual void beginRun();

    /** process event */
    virtual void event();

    /** end run */
    virtual void endRun();

    /** terminate */
    virtual void terminate();

  protected:

  private:

    /** Output path variable. */
    std::string m_outPath = "KlIdBKGClassifierTrainingTuples.root";

    /** varibales to write out. used for classification of clusters  */
    // KLM cluster variables for pure KLM and KLM+ECL Klongs
    /**  number of clusters */
    Float_t    m_KLMnCluster;
    /**  number of layers hit in KLM cluster */
    Float_t    m_KLMnLayer;
    /** number of innermost layers hit */
    Float_t    m_KLMnInnermostLayer;
    /** global Z position in KLM  */
    Float_t m_KLMglobalZ;
    /** timing of KLM Cluster */
    Float_t m_KLMtime;
    /**  average distance between all KLM clusters */
    Float_t m_KLMavInterClusterDist;
    /** hit depth in KLM, distance to IP */
    Float_t m_KLMhitDepth;
    /** Energy deposit in KLM (0.2 GeV * nHitCells) */
    Float_t m_KLMenergy;
    /**  invariant mass calculated from root vector */
    Float_t m_KLMinvM;
    /** target variable for KLM classification */
    Float_t m_KLMTruth;
    /** distance to next KLM cluster */
    Float_t m_KLMnextCluster;
    /** distance from track separation object  */
    Float_t m_KLMTrackSepDist;
    /** angular distance from track separation object */
    Float_t m_KLMTrackSepAngle;

    /** angular distance from track to cluster at track starting point */
    Float_t m_KLMInitialTrackSepAngle;
    /** angle between track at poca and trackbeginning */
    Float_t m_KLMTrackRotationAngle;
    /** angle between trach momentum and cluster (measured from ip) */
    Float_t m_KLMTrackClusterSepAngle;
    /** angle between KLMcluster and Mcparticle */
    Float_t m_KLMAngleToMC;


    // variables of closest ECL cluster with respect to KLM cluster
    /** distance associated ECL <-> KLM cluster */
    Float_t m_KLMECLDist;
    /** energy measured in associated ECL cluster */
    Float_t m_KLMECLE;
    /** distance between track entry point and cluster center, might be removed */
    Float_t m_KLMECLdeltaL;   // new
    /** track distance between associated ECL cluster and track extrapolated into ECL */
    Float_t m_KLMECLminTrackDist; //new
    /** E in surrounding 9 crystals divided by surrounding 25 crydtalls */
    Float_t m_KLMECLE9oE25;
    /** timing of associated ECL cluster */
    Float_t m_KLMECLTiming;
    /** uncertainty on time in associated ECL cluster */
    Float_t m_KLMECLTerror;
    /** uncertainty on E in associated ECL cluster */
    Float_t m_KLMECLEerror;
    /** primitive distance cluster <-> track for associated ECL cluster */
    Float_t m_KLMtrackToECL;
    /** KlId for that object */
    Float_t m_KLMKLid;
    /** momentum of matched mc particle */
    Float_t m_KLMMCMom;
    /** phi of matched mc particle */
    Float_t m_KLMMCPhi;
    /** theta of matched mc particle */
    Float_t m_KLMMCTheta;
    /** measured momentum */
    Float_t m_KLMMom;
    /** measured phi  */
    Float_t m_KLMPhi;
    /** measured theta */
    Float_t m_KLMTheta;

    /** MC particles status */
    Float_t m_KLMMCStatus  ;
    /** MC partilces life time */
    Float_t m_KLMMCLifetime;
    /** pdg code of matched MCparticle */
    Float_t m_KLMMCPDG     ;
    /** pdg code of MCparticles mother, for example pi0 for some gammas */
    Float_t m_KLMMCPrimaryPDG     ;
    /** hypotheis id of closest ecl cluster 5: gamma, 6:hadron  */
    Float_t m_KLMECLHypo;
    /** zernike mva output for closest ECL cluster (based on around 10 z-moments) */
    Float_t m_KLMECLZMVA;
    /** zernike moment 4,0 of closest ecl cluster */
    Float_t m_KLMECLZ40;
    /** zernike moment 5,1 of closest ECL cluster */
    Float_t m_KLMECLZ51;
    /** phi uncertainty oof closeest ecl cluster */
    Float_t m_KLMECLUncertaintyPhi;
    /** theta uncertainty of closest ECL cluster */
    Float_t m_KLMECLUncertaintyTheta;
    /** mc weight */
    Float_t m_KLMMCWeight;
    /** track flag for belle comparision */
    Float_t m_KLMtrackFlag;
    /** ecl flag for belle comparision */
    Float_t m_KLMeclFlag;


    // ECL cluster variables for pure ECL Klongs
    /** measured energy */
    Float_t m_ECLE;
    /** energy of 9/25 chrystall rings (E dispersion shape) */
    Float_t m_ECLE9oE25;
    /** timing of ECL */
    Float_t m_ECLTiming;
    /** distance of cluster to IP */
    Float_t m_ECLR;
    /** uncertainty on E measurement in ECL */
    Float_t m_ECLEerror;
    /** more sophisticated distaqnce to track in ECL */
    Float_t m_ECLminTrkDistance; // new
    /** distance between track entrace into cluster and cluster center */
    Float_t m_ECLdeltaL; // new


    /** Zernike moment 5,1 see Belle2 note on that */
    Float_t m_ECLZ51;
    /** Zernike moment 4,0 see Belle2 note on that */
    Float_t m_ECLZ40;
    /** central crystal devided by 3x3 area with it in its center */
    Float_t m_ECLE1oE9;
    /** second moment, shower shape */
    Float_t m_ECL2ndMom;
    /** number of crystals in the cluster */
    Float_t m_ECLnumChrystals;
    /** lateral shower shape */
    Float_t m_ECLLAT;
    /** output of a BDT that was fitted on some Zernike Moments on a connected region */
    Float_t m_ECLZMVA;
    /** classifier output */
    Float_t m_ECLKLid;
    /** mc status, seen in detector etc. ...*/
    Float_t m_ECLMCStatus  ;
    /** MC particles lifetime */
    Float_t m_ECLMCLifetime;
    /** pdg code of the MCparticle directly related to the cluster */
    Float_t m_ECLMCPDG     ;
    /** pdg code of higher order MC particle,
     * a cluster related to a photon that originates from a pi0 decay get the pi0 code */
    Float_t m_ECLMCPrimaryPDG     ;
    /** KlId for that object */
    Float_t m_ECLDeltaTime        ;
    /** measured energy uncertainty */
    Float_t m_ECLUncertaintyEnergy;
    /** measured uncertainty on theta */
    Float_t m_ECLUncertaintyTheta ;
    /** measured uncertainty of phi */
    Float_t m_ECLUncertaintyPhi   ;
    /** MC particle momentum; -999 if not MCparticle */
    Float_t m_ECLMCMom;
    /** MC particle phi; -999 if not MCparticle  */
    Float_t m_ECLMCPhi;
    /** MC particle momentum; -999 if not MCparticle */
    Float_t m_ECLMCTheta;
    /** measured momentum */
    Float_t m_ECLMom;
    /** measured phi */
    Float_t m_ECLPhi;
    /** measured theta */
    Float_t m_ECLTheta;
    /** measured Z-coordinate */
    Float_t m_ECLZ;
    /** ECL trarget variable */
    Float_t m_ECLTruth;
    /** is beam bkg */
    Float_t m_isBeamBKG;
    /** mc weight */
    Float_t m_ECLMCWeight;

    /** isSignal for the classifier */
    Float_t m_isSignal;


    /** Store array  */
    StoreArray<MCParticle> m_mcParticles;
    /** Store array  */
    StoreArray<KLMCluster> m_klmClusters;
    /** Store array  */
    StoreArray<ECLCluster> m_eclClusters;

    /** root file */
    TFile* m_f = nullptr; //
    /** tree for klm data */
    TTree* m_treeKLM = nullptr;
    /** tree containing ntuples for ECL cluster with N2 (hadron hypothesis) */
    TTree* m_treeECLhadron = nullptr;
    /** tree containing ntuples for ECL cluster with N1 (photon hypothesis) */
    TTree* m_treeECLgamma = nullptr;

    /** write out KLM data */
    bool m_useKLM;

    /** write out KLM data */
    bool m_useECL;


  }; // end class
} // end namespace Belle2

#endif
