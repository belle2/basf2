#include <tracking/modules/trackFinderVXDTests/StudyMaterialEffectsModule.h>

#include <framework/pcore/ProcHandler.h>
#include <framework/logging/Logger.h>

#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDCluster.h>

#include <vxd/geometry/SensorInfoBase.h>

#include <TVector3.h>
#include "TMath.h"


using namespace std;
using namespace Belle2;


REG_MODULE(StudyMaterialEffects)

StudyMaterialEffectsModule::StudyMaterialEffectsModule() : Module(), m_tree("materialEffectsStudyOutput" , DataStore::c_Persistent)
{
  setDescription("StudyMaterialEffects- should be used with single track pGuns and without magnetic field.");

  setPropertyFlags(c_ParallelProcessingCertified | c_TerminateInAllProcesses);
}


void StudyMaterialEffectsModule::initialize()
{
  m_spacePoints.isRequired("spacePoints");

  m_file = new TFile("materialStudy.root", "RECREATE");

  m_file->cd();

  m_tree.registerInDataStore();
  m_tree.construct("materialEffectsStudyTree", "Raw data of two-hit-combinations for a sectorMap");

  m_tree->get().Branch("ResidualPhiL3L3", &(m_PhiL3L3));
  m_tree->get().Branch("ResidualPhiL3L4", &(m_PhiL3L4));
  m_tree->get().Branch("ResidualPhiL3L6", &(m_PhiL3L6));
  m_tree->get().Branch("ResidualThetaL3L3", &(m_ThetaL3L3));
  m_tree->get().Branch("ResidualThetaL3L4", &(m_ThetaL3L4));
  m_tree->get().Branch("ResidualThetaL3L6", &(m_ThetaL3L6));
  m_tree->get().Branch("ResidualScatterAngleL3L3", &(m_ScatterAngleL3L3));
  m_tree->get().Branch("ResidualScatterAngleL3L4", &(m_ScatterAngleL3L4));
  m_tree->get().Branch("ResidualcatterAngleL3L6", &(m_ScatterAngleL3L6));
  m_tree->get().Branch("ResidualScatterAngleV2GradL3L3", &(m_ScatterAngleGradL3L3));
  m_tree->get().Branch("ResidualScatterAngleV2GradL3L4", &(m_ScatterAngleGradL3L4));
  m_tree->get().Branch("ResidualScatterAngleV2GradL3L6", &(m_ScatterAngleGradL3L6));
  m_tree->get().Branch("ResidualScatterAngleV3GradL3L3", &(m_ScatterAngleV3GradL3L3));
  m_tree->get().Branch("ResidualScatterAngleV3GradL3L4", &(m_ScatterAngleV3GradL3L4));
  m_tree->get().Branch("ResidualScatterAngleV3GradL3L6", &(m_ScatterAngleV3GradL3L6));
  m_tree->get().Branch("ResidualcatterAngleL3L6", &(m_ScatterAngleL3L6));
  m_tree->get().Branch("ResidualXYL3L4", &(m_distXY));
  m_tree->get().Branch("ResidualMomentumL3bL3e", &(m_deltaPL3L3));
  m_tree->get().Branch("ResidualMomentumL3bL4b", &(m_deltaPL3L4));
  m_tree->get().Branch("ResidualMomentumL3bL6e", &(m_deltaPL3L6));

  B2WARNING("StudyMaterialEffectsModule::initialize: nBranches: " << m_tree->get().GetNbranches());
}



void StudyMaterialEffectsModule::event()
{
  // position and momentum in global coordinates:
  B2Vector3D l3HitPosBegin;
  B2Vector3D l3HitPosEnd;
  B2Vector3D l4HitPosBegin;
  B2Vector3D l6HitPosEnd;
  B2Vector3D l3MomentumBegin;
  B2Vector3D l3MomentumEnd;
  B2Vector3D l4MomentumBegin;
  B2Vector3D l6MomentumEnd;

  bool wasFoundL3(false);
  bool wasFoundL4(false);
  bool wasFoundL6(false);


  for (const auto& aSP : m_spacePoints) {
    VxdID vxdID = aSP.getVxdID();
    const SVDTrueHit* trueHit = getTrueHit(aSP);
    if (trueHit == nullptr) continue;

    B2Vector3D entryHitPos = getGlobalPosition(trueHit, vxdID, true);
    B2Vector3D exitHitPos = getGlobalPosition(trueHit, vxdID, false);
    B2Vector3D entryMomentum = getGlobalMomentumVector(trueHit, vxdID, true);
    B2Vector3D exitMomentum = getGlobalMomentumVector(trueHit, vxdID, false);
    if (vxdID.getLayerNumber() == 3) {
      l3HitPosBegin = entryHitPos;
      l3HitPosEnd = exitHitPos;
      l3MomentumBegin = entryMomentum;
      l3MomentumEnd = exitMomentum;
      wasFoundL3 = true;
    }
    if (vxdID.getLayerNumber() == 4) {
      l4HitPosBegin = entryHitPos;
      l4MomentumBegin = entryMomentum;
      wasFoundL4 = true;
    }
    if (vxdID.getLayerNumber() == 6) {
      l6HitPosEnd = exitHitPos;
      l6MomentumEnd = exitMomentum;
      wasFoundL6 = true;
    }
  }

  if (!wasFoundL3 or !wasFoundL4 or !wasFoundL6) return;
  m_COUNTERsuccessfullEvents++;

  m_PhiL3L3 = l3HitPosBegin.Phi() - l3HitPosEnd.Phi();
  m_PhiL3L4 = l3HitPosBegin.Phi() - l4HitPosBegin.Phi();
  m_PhiL3L6 = l3HitPosBegin.Phi() - l6HitPosEnd.Phi();

  m_ThetaL3L3 = l3HitPosBegin.Theta() - l3HitPosEnd.Theta();
  m_ThetaL3L4 = l3HitPosBegin.Theta() - l4HitPosBegin.Theta();
  m_ThetaL3L6 = l3HitPosBegin.Theta() - l6HitPosEnd.Theta();

  m_ScatterAngleL3L3 = sqrt(pow(m_ThetaL3L3, 2) + pow(m_PhiL3L3, 2));
  m_ScatterAngleL3L4 = sqrt(pow(m_ThetaL3L4, 2) + pow(m_PhiL3L4, 2));
  m_ScatterAngleL3L6 = sqrt(pow(m_ThetaL3L6, 2) + pow(m_PhiL3L6, 2));

  m_ScatterAngleGradL3L3 = (l3HitPosEnd - l3HitPosBegin).Angle(l3HitPosBegin) * 180. / TMath::Pi();
  m_ScatterAngleGradL3L4 = (l4HitPosBegin - l3HitPosBegin).Angle(l3HitPosBegin) * 180. / TMath::Pi();
  m_ScatterAngleGradL3L6 = (l6HitPosEnd - l3HitPosBegin).Angle(l3HitPosBegin) * 180. / TMath::Pi();


  m_ScatterAngleV3GradL3L3 = l3MomentumEnd.Angle(l3MomentumBegin) * 180. / TMath::Pi();
  m_ScatterAngleV3GradL3L4 = l4MomentumBegin.Angle(l3MomentumBegin) * 180. / TMath::Pi();
  m_ScatterAngleV3GradL3L6 = l6MomentumEnd.Angle(l3MomentumBegin) * 180. / TMath::Pi();


  m_distXY = (l3HitPosBegin - l4HitPosBegin).Perp();

  m_deltaPL3L3 = l3MomentumBegin.Mag() - l3MomentumEnd.Mag();
  m_deltaPL3L4 = l3MomentumBegin.Mag() - l4MomentumBegin.Mag();
  m_deltaPL3L6 = l3MomentumBegin.Mag() - l6MomentumEnd.Mag();

  m_tree->get().Fill();
}



void StudyMaterialEffectsModule::terminate()
{
  B2WARNING("StudyMaterialEffects: there were " << m_COUNTERsuccessfullEvents << " events with hits resulting in root output");
  m_file->cd();

  if (!ProcHandler::parallelProcessingUsed() or ProcHandler::isOutputProcess()) {
    //use TFile you created in initialize()
    m_tree->write(m_file);
  }
}



B2Vector3D StudyMaterialEffectsModule::getGlobalPosition(const SVDTrueHit* trueHit, VxdID vxdID, bool useEntry)
{
  const Belle2::VXD::SensorInfoBase* aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(vxdID);

  if (useEntry) {
    B2Vector3D pos = aSensorInfo->pointToGlobal(TVector3(trueHit->getEntryU(), trueHit->getEntryV(), 0), true);
    return pos;
  }
  B2Vector3D pos = aSensorInfo->pointToGlobal(TVector3(trueHit->getExitU(), trueHit->getExitV(), 0), true);
  return pos;
}



const SVDTrueHit* StudyMaterialEffectsModule::getTrueHit(const SpacePoint& aSP)
{
//   const auto* aTrueHit = aSP.getRelationsTo<SVDCluster>("ALL")[0]->getRelationsTo<SVDTrueHit>("ALL")[0];;
  const SVDTrueHit* aTrueHit = nullptr;
  for (const SVDCluster& aCluster : aSP.getRelationsTo<SVDCluster>("ALL")) {
    for (const MCParticle& aMcParticle : aCluster.getRelationsTo<MCParticle>("ALL")) {
      if (aMcParticle.hasStatus(MCParticle::c_PrimaryParticle) == false) continue;

      aTrueHit = aCluster.getRelationsTo<SVDTrueHit>("ALL")[0];
//    for (const auto* aTrueHit : aCluster.getRelationsTo<SVDTrueHit>("ALL")) {
//
//    }
    }
  }
  return aTrueHit;
}


B2Vector3D StudyMaterialEffectsModule::getGlobalMomentumVector(const SVDTrueHit* trueHit, VxdID vxdID, bool useEntry)
{
  const Belle2::VXD::SensorInfoBase* aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(vxdID);

  if (useEntry) {
//  B2Vector3D mom = aSensorInfo->pointToGlobal(trueHit->getEntryMomentum());
    return aSensorInfo->vectorToGlobal(trueHit->getEntryMomentum(), true);
  }
//   B2Vector3D pos = aSensorInfo->pointToGlobal(trueHit->getExitMomentum());
  return aSensorInfo->vectorToGlobal(trueHit->getExitMomentum(), true);
}
