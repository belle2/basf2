/**************************************************************************
 *
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frederik Krohn                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <reconstruction/modules/KlId/KLMExpert/KLMExpertModule.h>
#include <reconstruction/dataobjects/KlId.h>
#include <framework/datastore/StoreArray.h>
#include <framework/utilities/FileSystem.h>

#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <tracking/dataobjects/TrackClusterSeparation.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <genfit/Exception.h>
#include <cstring>

#include <boost/algorithm/string/predicate.hpp>

#include <mva/interface/Interface.h>

#include "reconstruction/modules/KlId/KLMExpert/helperFunctions.h"

using namespace KlIdHelpers;
using namespace Belle2;
using namespace std;

REG_MODULE(KLMExpert);

KLMExpertModule::KLMExpertModule(): Module(), m_feature_variables(12, 0)
{
  setDescription("Use to calculate KlId for each KLMCluster.");
  setPropertyFlags(c_ParallelProcessingCertified);
}



KLMExpertModule::~KLMExpertModule()
{
}


// --------------------------------------Module----------------------------------------------
void KLMExpertModule::initialize()
{
  // require existence of necessary datastore obj
  StoreArray<KLMCluster>::required();
  StoreArray<RecoTrack>::required();
  StoreArray<ECLCluster>::required();

  StoreArray<ECLCluster> eclClusters;
  StoreArray<KLMCluster> klmClusters;
  klmClusters.registerRelationTo(eclClusters);

  StoreArray<KlId>::registerPersistent();//Transient

  StoreArray<KlId> klids;
  klmClusters.registerRelationTo(klids);


  if (not(boost::ends_with(m_identifier, ".root") or boost::ends_with(m_identifier, ".xml"))) {
    m_weightfile_representation = std::unique_ptr<DBObjPtr<DatabaseRepresentationOfWeightfile>>(new
                                  DBObjPtr<DatabaseRepresentationOfWeightfile>(m_identifier));
  }

  MVA::AbstractInterface::initSupportedInterfaces();

}


void KLMExpertModule::beginRun()
{

  if (m_weightfile_representation) {
    if (m_weightfile_representation->hasChanged()) {
      std::stringstream ss((*m_weightfile_representation)->m_data);
      auto weightfile = MVA::Weightfile::loadFromStream(ss);
      init_mva(weightfile);
    }
  } else {
    auto weightfile = MVA::Weightfile::loadFromFile(m_identifier);

    init_mva(weightfile);

  }
}
/** init mva file taken from mva/ExpertModule */
void KLMExpertModule::init_mva(MVA::Weightfile& weightfile)
{

  auto supported_interfaces = MVA::AbstractInterface::getSupportedInterfaces();
  MVA::GeneralOptions general_options;
  weightfile.getOptions(general_options);

  m_expert = supported_interfaces[general_options.m_method]->getExpert();
  m_expert->load(weightfile);

  std::vector<float> dummy;
  dummy.resize(m_feature_variables.size(), 0);
  m_dataset = std::unique_ptr<MVA::SingleDataset>(new MVA::SingleDataset(general_options, std::move(dummy), 0));

}


void KLMExpertModule::event()
{
  StoreArray<KLMCluster> klmClusters;
  StoreArray<RecoTrack> genfitTracks;
  StoreArray<ECLCluster> eclClusters;
  StoreArray<KlId> KlIds;

  //overwritten at the end of the cluster loop
  KlId* klid = nullptr;
  double IDMVAOut;

  // loop thru clusters in event and classify
  for (KLMCluster& cluster : klmClusters) {

    const TVector3& clusterPos = cluster.getClusterPosition();

    // get various KLMCluster vars
    m_KLMglobalZ         = clusterPos.Z();
    m_KLMnCluster        = klmClusters.getEntries();
    m_KLMnLayer          = cluster.getLayers();
    m_KLMnInnermostLayer = cluster.getInnermostLayer();
    m_KLMtime            = cluster.getTime();
    m_KLMenergy          = cluster.getEnergy();
    m_KLMhitDepth        = cluster.getClusterPosition().Mag2();

    // find nearest ecl cluster and calculate distance
    pair<ECLCluster*, double> closestECLAndDist = findClosestECLCluster(clusterPos);
    ECLCluster* closestECLCluster = get<0>(closestECLAndDist);
    m_KLMECLDist = get<1>(closestECLAndDist);


    if (!(closestECLCluster == nullptr)) {
      m_KLMECLE      = closestECLCluster -> getEnergy();
      m_KLMECLE9oE25 = closestECLCluster -> getE9oE21();
      m_KLMECLTerror = closestECLCluster -> getDeltaTime99();
      m_KLMECLTiming = closestECLCluster -> getTime();
      m_KLMECLEerror = closestECLCluster -> getUncertaintyEnergy();
      m_KLMECLdeltaL = closestECLCluster->getDeltaL();
      m_KLMECLminTrackDist = closestECLCluster->getMinTrkDistance();
    } else {

      m_KLMECLdeltaL       = -999;
      m_KLMECLminTrackDist = -999;

      m_KLMECLE      = -999;
      m_KLMECLE9oE25 = -999;
      m_KLMECLTiming = -999;
      m_KLMECLTerror = -999;
      m_KLMECLEerror = -999;
    }

    // calculate distance to next cluster
    tuple<const KLMCluster*, double, double> closestKLMAndDist = findClosestKLMCluster(clusterPos);
    m_KLMnextCluster = get<1>(closestKLMAndDist);
    m_KLMavInterClusterDist = get<2>(closestKLMAndDist);


    // calculate eucl. distance klmcluster <-> nearest track
    // extrapolate genfit trackfit result to their ends and find the
    tuple<RecoTrack*, double, std::unique_ptr<const TVector3>> closestTrackAndDistance
                                                            = findClosestTrack(clusterPos);
    m_KLMtrackDist = get<1>(closestTrackAndDistance);
    const TVector3* poca = get<2>(closestTrackAndDistance).get();

    if (poca and closestECLCluster) {
      const TVector3& trackECLClusterDist = closestECLCluster->getPosition() - *poca;
      m_KLMtrackToECL = trackECLClusterDist.Mag2();
    } else {
      m_KLMtrackToECL = -999;
    }


    TrackClusterSeparation* trackSep = cluster.getRelatedTo<TrackClusterSeparation>();
    m_KLMTrackSepDist         = trackSep->getDistance();
    m_KLMTrackSepAngle        = trackSep->getTrackClusterAngle();

    m_KLMInitialTrackSepAngle = trackSep->getTrackClusterInitialSeparationAngle();
    m_KLMTrackRotationAngle   = trackSep->getTrackRotationAngle();
    m_KLMTrackClusterSepAngle = trackSep->getTrackClusterSeparationAngle();


    if (isnan(m_KLMglobalZ))              { m_KLMglobalZ              = -999;}
    if (isnan(m_KLMnCluster))             { m_KLMnCluster             = -999;}
    if (isnan(m_KLMnLayer))               { m_KLMnLayer               = -999;}
    if (isnan(m_KLMnInnermostLayer))      { m_KLMnInnermostLayer      = -999;}
    if (isnan(m_KLMtime))                 { m_KLMtime                 = -999;}
    if (isnan(m_KLMenergy))               { m_KLMenergy               = -999;}
    if (isnan(m_KLMhitDepth))             { m_KLMhitDepth             = -999;}
    if (isnan(m_KLMTrackSepDist))         { m_KLMTrackSepDist         = -999;}
    if (isnan(m_KLMTrackSepAngle))        { m_KLMTrackSepAngle        = -999;}
    if (isnan(m_KLMInitialTrackSepAngle)) { m_KLMInitialTrackSepAngle = -999;}
    if (isnan(m_KLMTrackRotationAngle))   { m_KLMTrackRotationAngle   = -999;}
    if (isnan(m_KLMTrackClusterSepAngle)) { m_KLMTrackClusterSepAngle = -999;}

//    reduced vars set
    m_feature_variables[0] = m_KLMnLayer;
    m_feature_variables[1] = m_KLMnInnermostLayer;
    m_feature_variables[2] = m_KLMglobalZ;
    m_feature_variables[3] = m_KLMtime;
    m_feature_variables[4] = m_KLMnextCluster;
    m_feature_variables[5] = m_KLMenergy;
    m_feature_variables[6] = m_KLMtrackToECL;
    m_feature_variables[7] = m_KLMECLEerror;
    m_feature_variables[8] = m_KLMTrackSepDist;
    m_feature_variables[9] = m_KLMInitialTrackSepAngle;
    m_feature_variables[10] = m_KLMTrackRotationAngle;
    m_feature_variables[11] = m_KLMTrackSepAngle;


    // rewrite dataset
    for (unsigned int i = 0; i < m_feature_variables.size(); ++i) {
      m_dataset->m_input[i] = m_feature_variables[i];
    }

    //classify dartaset
    IDMVAOut = m_expert->apply(*m_dataset)[0];

    // KlId, bkg prob, KLM, ECL
    klid = KlIds.appendNew(IDMVAOut, -1, 1, 0);
    cluster.addRelationTo(klid);


  }// for cluster in clusters
} // event




