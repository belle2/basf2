/***************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frederik Krohn                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 *************************************************************************/
#ifndef KlIdKLMTMVAExpertModule_H
#define KlIdKLMTMVAExpertModule_H

#include <framework/utilities/FileSystem.h>

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

#include <string>

#include <TMVA/Reader.h>

namespace Belle2 {


  /** Module to perform the KLM KlId classification. This module only classifies KLM clusters.
   * The output is a KlId object on the datastore. It contains KlId, bkgProb and wheter its an ECL or KLM cluster */
  class KlIdKLMTMVAExpertModule : public Module {

  public:

    /** Constructor */
    KlIdKLMTMVAExpertModule();

    /** Destructor */
    virtual ~KlIdKLMTMVAExpertModule();

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


    /** varibales to write out. used for classification of clusters  */
    // KLM variables
    /**  number of clusters */
    Float_t m_KLMnCluster;
    /**  number of layers hit in KLM cluster */
    Float_t m_KLMnLayer;
    /** number of innermost layers hit */
    Float_t m_KLMnInnermostLayer;
    /** global Z position in KLM  */
    Float_t m_KLMglobalZ;
    /** timing of KLM Cluster */
    Float_t m_KLMtime;
    /**  average distance between all KLM clusters */
    Float_t m_KLMavInterClusterDist;
    /** hit depth in KLM, distance to IP */
    Float_t m_KLMhitDepth;
    /** Energy deposit in KLM (0.2 GeV * nHitCells) */
    Float_t m_KLMinvM;
    /** distance KLM Cluster <-> track extrapolated into KLM */
    Float_t m_KLMtrackDist;
    /** distance to next KLM cluster */
    Float_t m_KLMnextCluster;
    /** classifier output from bkg classification */
    Float_t m_KLMBKGProb;
    /** distance from track separation object  */
    Float_t m_KLMTrackSepDist;
    /** angular distance from track separation object.
     * angle between normal vector of track momentum and cluster position. */
    Float_t m_KLMTrackSepAngle;


    // variables of closest ECL cluster with respect to KLM cluster
    /** distance associated ECL <-> KLM cluster, extrapolated by genfit */
    Float_t m_KLMECLDist;
    /** energy measured in associated ECL cluster */
    Float_t m_KLMECLE;
    /** distance between track entry poFloat_t and cluster center, might be removed */
    Float_t m_KLMECLdeltaL;   // new
    /** track distance between associated ECL cluster and track extrapolated into ECL */
    Float_t m_KLMECLminTrackDist; //new
    /** E in surrounding 9 crystals divided by surrounding 25 crydtalls */
    Float_t m_KLMECLE9oE25;
    /** timing of associated ECL cluster */
    Float_t m_KLMECLTiming;
    /** uncertaFloat_ty on time in associated ECL cluster */
    Float_t m_KLMECLTerror;
    /** uncertaFloat_ty on E in associated ECL cluster */
    Float_t m_KLMECLEerror;
    /** primitive distance cluster <-> track for associated ECL cluster */
    Float_t m_KLMtrackToECL;
    /** classifier output from bkg classification of associated ECL cluster */
    Float_t m_KLMECLBKGProb;


    /** default classifier name */
    std::string m_IDClassifierName  = "KLMBDT";
    /** default classifier name */
    std::string m_BKGClassifierName = "KLMBKGClassifierBDT";
    /** default classifier name  */
    std::string m_ECLClassifierName = "ECLBKGClassifierBDT";

    /** TMVA classifier object. */
    TMVA::Reader* m_readerID  = new TMVA::Reader("Verbose");
    /** TMVA classifier object. */
    TMVA::Reader* m_readerBKG = new TMVA::Reader("Verbose");
    /** TMVA classifier object. */
    TMVA::Reader* m_readerECLBKG = new TMVA::Reader("Verbose");


    /** path to training .xml file. */
    std::string m_IDClassifierPath = FileSystem::findFile(
                                       "reconstruction/data/weights/TMVAFactory_KLMBDT.weights.xml");

    /** path to training .xml file. */
    std::string m_BKGClassifierPath = FileSystem::findFile(
                                        "reconstruction/data/weights/TMVAFactory_KLMBKGClassifierBDT.weights.xml");

    /** path to training .xml file. */
    std::string m_ECLBKGClassifierPath = FileSystem::findFile(
                                           "reconstruction/data/weights/TMVAFactory_ECLBKGClassifierBDT.weights.xml");



  }; // end class
} // end namespace Belle2


#endif
