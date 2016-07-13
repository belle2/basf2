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

KLMExpertModule::KLMExpertModule(): Module()
{
  setDescription("Use to calculate KlId for each KLMCluster.");

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

  StoreArray<KlId>::registerTransient();//Transient

  StoreArray<KlId> klids;
  klmClusters.registerRelationTo(klids);

  // reserve n variables in the vector !!:w
  m_feature_variables.reserve(23);

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

void KLMExpertModule::init_mva(MVA::Weightfile& weightfile)
{

  auto supported_interfaces = MVA::AbstractInterface::getSupportedInterfaces();
  MVA::GeneralOptions general_options;
  weightfile.getOptions(general_options);

  // Overwrite signal fraction from training
  if (m_signal_fraction_override > 0)
    weightfile.addSignalFraction(m_signal_fraction_override);

  m_expert = supported_interfaces[general_options.m_method]->getExpert();
  m_expert->load(weightfile);

  std::vector<float> dummy;
  dummy.resize(m_feature_variables.size(), 0);
  m_dataset = std::unique_ptr<MVA::SingleDataset>(new MVA::SingleDataset(general_options, dummy, 0));

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
    m_KLMinvM            = cluster.getMomentum().M2();
    m_KLMhitDepth        = cluster.getClusterPosition().Mag2();

    // find nearest ecl cluster and calculate distance
    pair<ECLCluster*, double> closestECLAndDist = findClosestECLCluster(clusterPos);
    ECLCluster* closestECLCluster = get<0>(closestECLAndDist);
    m_KLMECLDist = get<1>(closestECLAndDist);


    if (!(closestECLCluster == nullptr)) {
      m_KLMECLE      = closestECLCluster -> getEnergy();
      m_KLMECLE9oE25 = closestECLCluster -> getE9oE25();
      m_KLMECLTerror = closestECLCluster -> getErrorTiming();
      m_KLMECLTiming = closestECLCluster -> getTiming();
      m_KLMECLEerror = closestECLCluster -> getErrorEnergy();
      m_KLMECLdeltaL = closestECLCluster->getTemporaryDeltaL();
      m_KLMECLminTrackDist = closestECLCluster->getTemporaryMinTrkDistance();
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
    tuple<RecoTrack*, double, const TVector3*> closestTrackAndDistance
      = findClosestTrack(clusterPos);
    m_KLMtrackDist = get<1>(closestTrackAndDistance);
    const TVector3* poca = get<2>(closestTrackAndDistance);

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


    m_feature_variables.push_back(m_KLMnCluster);
    m_feature_variables.push_back(m_KLMnLayer);
    m_feature_variables.push_back(m_KLMnInnermostLayer);
    m_feature_variables.push_back(m_KLMglobalZ);
    m_feature_variables.push_back(m_KLMtime);
    m_feature_variables.push_back(m_KLMinvM);
    m_feature_variables.push_back(m_KLMtrackDist);
    m_feature_variables.push_back(m_KLMnextCluster);
    m_feature_variables.push_back(m_KLMavInterClusterDist);
    m_feature_variables.push_back(m_KLMTrackSepDist);
    m_feature_variables.push_back(m_KLMTrackSepAngle);
    m_feature_variables.push_back(m_KLMInitialTrackSepAngle);
    m_feature_variables.push_back(m_KLMECLDist);
    m_feature_variables.push_back(m_KLMECLE);
    m_feature_variables.push_back(m_KLMECLE9oE25);
    m_feature_variables.push_back(m_KLMECLTiming);
    m_feature_variables.push_back(m_KLMECLEerror);
    m_feature_variables.push_back(m_KLMtrackToECL);
    m_feature_variables.push_back(m_KLMECLdeltaL);
    m_feature_variables.push_back(m_KLMECLminTrackDist);


    // rewrite dataset
    for (unsigned int i = 0; i < m_feature_variables.size(); ++i) {
      m_dataset->m_input[i] = m_feature_variables[i];
    }

    //classify dartaset
    IDMVAOut = m_expert->apply(*m_dataset)[0];


    // normalize if not errorcode
    if (IDMVAOut > -1.) {
      IDMVAOut = (IDMVAOut + 1) / 2.0;
    }

    // KlId, bkg prob, KLM, ECL
    klid = KlIds.appendNew(IDMVAOut, -1, 1, 0);
    cluster.addRelationTo(klid);

    m_feature_variables.clear();


  }// for cluster in clusters
} // event




