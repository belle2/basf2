
/***************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frederik Krohn                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 *************************************************************************/
#ifndef BKGClassifierDataWriterModule_H
#define BKGClassifierDataWriterModule_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

#include <TTree.h>
#include <TFile.h>
#include <string>

namespace Belle2 {

  /** Module to write Ntuples for KlId BKG classifier training. Writes tuples for both ECL and KLM. You have to specify  an outputpath outPath.   */
  class BKGClassifierDataWriterModule : public Module {

  public:

    /** Constructor */
    BKGClassifierDataWriterModule();

    /** Destructor */
    virtual ~BKGClassifierDataWriterModule();

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
    /** distance KLM Cluster <-> track extrapolated into KLM */
    Float_t m_KLMtrackDist;
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
    /** more sophisticated distaqnce to track in ECL, might be removed */
    Float_t m_ECLminTrkDistance; // new
    /** disatance between track entrace into cluster and cluster center */
    Float_t m_ECLdeltaL; // new
    /** distance cluster to next track in ECL */
    Float_t m_ECLtrackDist;
    /** ECL trarget variable */
    Float_t m_ECLTruth;

    /** is beam bkg */
    Float_t m_isBeamBKG;

    /** root file */
    TFile* m_f = nullptr; //
    /** tree for klm data */
    TTree* m_treeKLM = nullptr;
    /** tree for ecl data */
    TTree* m_treeECL = nullptr;

  }; // end class
} // end namespace Belle2

#endif
