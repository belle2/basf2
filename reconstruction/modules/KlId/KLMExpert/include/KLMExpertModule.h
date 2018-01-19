/***************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frederik Krohn                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 *************************************************************************/
#ifndef KLMExpertModule_H
#define KLMExpertModule_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/database/DBObjPtr.h>

#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/KlId.h>

#include <mva/dataobjects/DatabaseRepresentationOfWeightfile.h>
#include <mva/interface/Weightfile.h>
#include <mva/interface/Expert.h>

#include <string>


namespace Belle2 {


  /** Module to perform the KLM KlId classification. This module only classifies KLM clusters.
   * The output is a KlId object on the datastore. It contains KlId, bkgProb and wheter its an ECL or KLM cluster */
  class KLMExpertModule : public Module {

  public:

    /** Constructor */
    KLMExpertModule();

    /** Destructor */
    virtual ~KLMExpertModule();

    /** init */
    virtual void initialize() override;

    /** beginn run */
    virtual void beginRun() override;

    /** process event */
    virtual void event() override;

    /** terminate and free */
    virtual void terminate() override
    {
      m_expert.reset();
      m_dataset.reset();
    }

  private:

    /**
     * Initialize mva expert, dataset and features
     * Called everytime the weightfile in the database changes in begin run
     */
    void init_mva(MVA::Weightfile& weightfile);


    /** varibales to write out. used for classification of clusters  */
    // KLM variables
    /**  number of clusters */
    float m_KLMnCluster;
    /**  number of layers hit in KLM cluster */
    float m_KLMnLayer;
    /** number of innermost layers hit */
    float m_KLMnInnermostLayer;
    /** global Z position in KLM  */
    float m_KLMglobalZ;
    /** timing of KLM Cluster */
    float m_KLMtime;
    /**  average distance between all KLM clusters */
    float m_KLMavInterClusterDist;
    /** hit depth in KLM, distance to IP */
    float m_KLMhitDepth;
    /** Energy deposit in KLM (0.2 GeV * nHitCells) */
    float m_KLMenergy;
    /** distance to next KLM cluster */
    float m_KLMnextCluster;
    /** distance from track separation object  */
    float m_KLMTrackSepDist;
    /** angular distance from track separation object.
     * angle between normal vector of track momentum and cluster position. */
    float m_KLMTrackSepAngle;

    /** angular distance from track to cluster at track starting point */
    float m_KLMInitialTrackSepAngle;
    /** angle between track at poca and trackbeginning */
    float m_KLMTrackRotationAngle;
    /** angle between trach momentum and cluster (measured from ip) */
    float m_KLMTrackClusterSepAngle;

    // variables of closest ECL cluster with respect to KLM cluster
    /** distance associated ECL <-> KLM cluster, extrapolated by genfit */
    float m_KLMECLDist;
    /** energy measured in associated ECL cluster */
    float m_KLMECLE;
    /** distance between track entry pofloat and cluster center, might be removed */
    float m_KLMECLdeltaL;   // new
    /** track distance between associated ECL cluster and track extrapolated into ECL */
    float m_KLMECLminTrackDist; //new
    /** E in surrounding 9 crystals divided by surrounding 25 crydtalls */
    float m_KLMECLE9oE25;
    /** timing of associated ECL cluster */
    float m_KLMECLTiming;
    /** uncertanty on time in associated ECL cluster */
    float m_KLMECLTerror;
    /** uncertanty on E in associated ECL cluster */
    float m_KLMECLEerror;

    /** output of a BDT fitted on various Z-moments for the closest ECL cluster */
    float m_KLMECLZMVA;
    /** zernike moment 4,0 of closest ECL */
    float m_KLMECLZ40;
    /** zernike moment 5,1 of closest ECL */
    float m_KLMECLZ51;

    /** storearray */
    StoreArray<KLMCluster> m_klmClusters;
    /** storearray */
    StoreArray<KlId> m_klids;

    /** vars to be classified */
    std::vector<float> m_feature_variables;

    /** mva identifier. no ending means its loaded from the database  */
    std::string m_identifier =
      "KLM_fBDT_10xbkg100k"; // assuming 1 times beambkg and generic BBbar events

    /** Database pointer to the Database representation of the weightfile */
    std::unique_ptr<DBObjPtr<DatabaseRepresentationOfWeightfile>>
                                                               m_weightfile_representation;
    /** Pointer to the current MVA Expert */
    std::unique_ptr<MVA::Expert> m_expert;
    /** Pointer to the current dataset */
    std::unique_ptr<MVA::SingleDataset> m_dataset;


  }; // end class
} // end namespace Belle2


#endif
