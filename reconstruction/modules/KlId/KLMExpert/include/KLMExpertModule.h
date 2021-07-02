/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

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
    float m_KLMnCluster{ -1.};
    /**  number of layers hit in KLM cluster */
    float m_KLMnLayer{ -1.};
    /** number of innermost layers hit */
    float m_KLMnInnermostLayer{ -1.};
    /** global Z position in KLM  */
    float m_KLMglobalZ{0.};
    /** timing of KLM Cluster */
    float m_KLMtime{ -1.};
    /**  average distance between all KLM clusters */
    float m_KLMavInterClusterDist{ -1.};
    /** hit depth in KLM, distance to IP */
    float m_KLMhitDepth{ -1.};
    /** Energy deposit in KLM (0.2 GeV * nHitCells) */
    float m_KLMenergy{ -1.};
    /** distance to next KLM cluster */
    float m_KLMnextCluster{ -1.};
    /** distance from track separation object  */
    float m_KLMTrackSepDist{ -1.};
    /** angular distance from track separation object.
     * angle between normal vector of track momentum and cluster position. */
    float m_KLMTrackSepAngle{ -1.};

    /** angular distance from track to cluster at track starting point */
    float m_KLMInitialTrackSepAngle{ -1.};
    /** angle between track at poca and trackbeginning */
    float m_KLMTrackRotationAngle{ -1.};
    /** angle between trach momentum and cluster (measured from ip) */
    float m_KLMTrackClusterSepAngle{ -1.};

    // variables of closest ECL cluster with respect to KLM cluster
    /** distance associated ECL <-> KLM cluster, extrapolated by genfit */
    float m_KLMECLDist{ -1.};
    /** energy measured in associated ECL cluster */
    float m_KLMECLE{ -1.};
    /** distance between track entry pofloat and cluster center, might be removed */
    float m_KLMECLdeltaL{ -1.};   // new
    /** track distance between associated ECL cluster and track extrapolated into ECL */
    float m_KLMECLminTrackDist{ -1.}; //new
    /** E in surrounding 9 crystals divided by surrounding 25 crydtalls */
    float m_KLMECLE9oE25{ -1.};
    /** timing of associated ECL cluster */
    float m_KLMECLTiming{ -1.};
    /** uncertanty on time in associated ECL cluster */
    float m_KLMECLTerror{ -1.};
    /** uncertanty on E in associated ECL cluster */
    float m_KLMECLEerror{ -1.};

    /** output of a BDT fitted on various Z-moments for the closest ECL cluster */
    float m_KLMECLZMVA{ -1.};
    /** zernike moment 4,0 of closest ECL */
    float m_KLMECLZ40{ -1.};
    /** zernike moment 5,1 of closest ECL */
    float m_KLMECLZ51{ -1.};

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
