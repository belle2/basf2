/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <bklm/modules/bklmK0LReconstructor/BKLMK0LReconstructorModule.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/gearbox/Const.h>

#include <bklm/dataobjects/BKLMHit2d.h>

#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/ECLCluster.h>

#include <cmath>

#define NLAYER 15
#define HITS_TO_GEV 0.215

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------

REG_MODULE(BKLMK0LReconstructor)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

BKLMK0LReconstructorModule::BKLMK0LReconstructorModule() : Module(), m_KaonMassSq(0.0)
{
  setDescription("BKLM K0L reconstruction module.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("KLMClustersColName", m_KLMClustersColName, "Name of collection holding the KLMClusters", string(""));
  addParam("ECLClustersColName", m_ECLClustersColName, "Name of collection holding the ECLClusters", string(""));
  addParam("Clustering cone-angle window (degrees)", m_MaxHitConeAngle,
           "Hits within this cone angle (from IP) belong to the same cluster",
           double(15.0));
  addParam("KLM-ECL cluster-matching cone-angle window (degrees)", m_MaxKLMECLConeAngle,
           "ECL cluster within this cone angle (from IP) matches the KLM cluster",
           double(20.0));
}

BKLMK0LReconstructorModule::~BKLMK0LReconstructorModule()
{
}

void BKLMK0LReconstructorModule::initialize()
{
  StoreArray<BKLMHit2d>::required();

  // Force creation and persistence of output datastore and relation
  StoreArray<KLMCluster> klmCluster;
  klmCluster.registerInDataStore();
  // DIVOT StoreArray<BKLMHit2d> bklmHit2d;
  // DIVOT klmCluster.registerRelationTo(bklmHit2d);
  StoreArray<ECLCluster> eclCluster;
  klmCluster.registerRelationTo(eclCluster);

  m_KaonMassSq = Const::Klong.getMass() * Const::Klong.getMass();
  m_MaxHitConeAngle = m_MaxHitConeAngle * M_PI / 180.0;
  m_MaxKLMECLConeAngle = m_MaxKLMECLConeAngle * M_PI / 180.0;

}

void BKLMK0LReconstructorModule::beginRun()
{
  StoreObjPtr<EventMetaData> evtMetaData;
  B2INFO("BKLMK0LReconstructor: Experiment " << evtMetaData->getExperiment() << "  run " << evtMetaData->getRun())
}

void BKLMK0LReconstructorModule::event()
{

  // Construct clusters from StoreArray<BKLMHit2d>

  StoreArray<BKLMHit2d> hit2ds;
  std::vector<std::vector<int> > clusterIndices;
  for (int i = 0; i < hit2ds.getEntries(); ++i) {
    if (hit2ds[i]->isOutOfTime()) continue;
    TVector3 pos = hit2ds[i]->getGlobalPosition();
    for (unsigned int j = 0; j < clusterIndices.size(); ++j) {
      for (unsigned int k = 0; k < clusterIndices[j].size(); ++k) {
        if (pos.Angle(hit2ds[clusterIndices[j][k]]->getGlobalPosition()) < m_MaxHitConeAngle) {
          clusterIndices[j].push_back(i);
          goto AddedToExistingCluster;
        }
      }
    }
    clusterIndices.push_back(std::vector<int>(1, i));
AddedToExistingCluster: ;
  }

  // Fill StoreArray<KLMCluster> and find matching ECLClusters

  StoreArray<KLMCluster> klmClusters;
  StoreArray<ECLCluster> eclClusters;
  for (unsigned int j = 0; j < clusterIndices.size(); ++j) {
    int layers[NLAYER + 1] = { 0 };
    unsigned int nHits = clusterIndices[j].size();
    unsigned int innermost = 0;
    TVector3 pos(0.0, 0.0, 0.0);
    for (unsigned int k = 0; k < nHits; ++k) {
      int layer = hit2ds[clusterIndices[j][k]]->getLayer();
      layers[layer] = 1;
      if (layer < hit2ds[clusterIndices[j][innermost]]->getLayer()) {
        innermost = k;
      }
      pos += hit2ds[clusterIndices[j][k]]->getGlobalPosition();
    }
    pos *= (1.0 / nHits);
    TVector3 mom = hit2ds[clusterIndices[j][innermost]]->getGlobalPosition().Unit()
                   * nHits * HITS_TO_GEV;
    int nlayers = 0;
    for (int layer = 1; layer <= NLAYER; ++layer) { nlayers += layers[layer]; }
    KLMCluster* cluster =
      klmClusters.appendNew(pos.x(), pos.y(), pos.z(),
                            hit2ds[clusterIndices[j][innermost]]->getTime(),
                            nlayers,
                            hit2ds[clusterIndices[j][innermost]]->getLayer(),
                            mom.x(), mom.y(), mom.z());
    ECLCluster* bestECLCluster = NULL;
    double bestOpeningAngle = M_PI;
    for (int m = 0; m < eclClusters.getEntries(); ++m) {
      double openingAngle = mom.Angle(eclClusters[m]->getMomentum());
      if (openingAngle < bestOpeningAngle) {
        bestOpeningAngle = openingAngle;
        bestECLCluster = eclClusters[m];
      }
    }
    if (bestOpeningAngle < m_MaxKLMECLConeAngle) {
      cluster->setAssociatedEclClusterFlag();
      cluster->addRelationTo(bestECLCluster);
    }
  }

}

void BKLMK0LReconstructorModule::endRun()
{
}

void BKLMK0LReconstructorModule::terminate()
{
}
