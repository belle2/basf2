
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
    int    m_KLMnCluster;
    /**  number of layers hit in KLM cluster */
    int    m_KLMnLayer;
    /** number of innermost layers hit */
    int    m_KLMnInnermostLayer;
    /** global Z position in KLM  */
    double m_KLMglobalZ;
    /** timing of KLM Cluster */
    double m_KLMtime;
    /**  average distance between all KLM clusters */
    double m_KLMavInterClusterDist;
    /** hit depth in KLM, distance to IP */
    double m_KLMhitDepth;
    /** Energy deposit in KLM (0.2 GeV * nHitCells) */
    double m_KLMenergy;
    /**  invariant mass calculated from root vector */
    double m_KLMinvM;
    /** distance KLM Cluster <-> track extrapolated into KLM */
    double m_KLMtrackDist;
    /** target variable for KLM classification */
    int    m_KLMTruth;
    /** distance to next KLM cluster */
    double m_KLMnextCluster;
    /** distance from track separation object  */
    double m_KLMTrackSepDist;
    /** angular distance from track separation object */
    double m_KLMTrackSepAngle;

    /** angular distance from track to cluster at track starting point */
    double m_KLMInitialTrackSepAngle;
    /** angle between track at poca and trackbeginning */
    double m_KLMTrackRotationAngle;
    /** angle between trach momentum and cluster (measured from ip) */
    double m_KLMTrackClusterSepAngle;



    // variables of closest ECL cluster with respect to KLM cluster
    /** distance associated ECL <-> KLM cluster */
    double m_KLMECLDist;
    /** energy measured in associated ECL cluster */
    double m_KLMECLE;
    /** distance between track entry point and cluster center, might be removed */
    double m_KLMECLdeltaL;   // new
    /** track distance between associated ECL cluster and track extrapolated into ECL */
    double m_KLMECLminTrackDist; //new
    /** E in surrounding 9 crystals divided by surrounding 25 crydtalls */
    double m_KLMECLE9oE25;
    /** timing of associated ECL cluster */
    double m_KLMECLTiming;
    /** uncertainty on time in associated ECL cluster */
    double m_KLMECLTerror;
    /** uncertainty on E in associated ECL cluster */
    double m_KLMECLEerror;
    /** primitive distance cluster <-> track for associated ECL cluster */
    double m_KLMtrackToECL;

    // ECL cluster variables for pure ECL Klongs
    /** measured energy */
    double m_ECLE;
    /** energy of 9/25 chrystall rings (E dispersion shape) */
    double m_ECLE9oE25;
    /** timing of ECL */
    double m_ECLTiming;
    /** distance of cluster to IP */
    double m_ECLR;
    /** uncertainty on E measurement in ECL */
    double m_ECLEerror;
    /** more sophisticated distaqnce to track in ECL, might be removed */
    double m_ECLminTrkDistance; // new
    /** disatance between track entrace into cluster and cluster center */
    double m_ECLdeltaL; // new
    /** distance cluster to next track in ECL */
    double m_ECLtrackDist;
    /** ECL trarget variable */
    double m_ECLTruth;

    /** used in both trees */
    double m_isBeamBKG;

    /** root file */
    TFile* m_f = nullptr; //
    /** tree for klm data */
    TTree* m_treeKLM = nullptr;
    /** tree for ecl data */
    TTree* m_treeECL = nullptr;

  }; // end class
} // end namespace Belle2

#endif
