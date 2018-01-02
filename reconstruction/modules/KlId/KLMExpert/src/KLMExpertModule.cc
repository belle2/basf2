/**************************************************************************
 *
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
#include <mdst/dataobjects/KlId.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

#include <mdst/dataobjects/ECLCluster.h>
#include <tracking/dataobjects/TrackClusterSeparation.h>

#include <mva/interface/Interface.h>
#include <boost/algorithm/string/predicate.hpp>

// here's where the functions are hidden
#include "reconstruction/modules/KlId/KLMExpert/KlId.h"

using namespace KlId;
using namespace Belle2;
using namespace std;

REG_MODULE(KLMExpert);

KLMExpertModule::KLMExpertModule(): Module(), m_feature_variables(18, 0) //12
{
  setDescription("Use to calculate KlId for each KLM cluster.");
  addParam("classifierPath", m_identifier,
           "path to the classifier you want to use. It is recommended to use the default classifiers and not to mess around with this.",
           m_identifier);
  setPropertyFlags(c_ParallelProcessingCertified);
}



KLMExpertModule::~KLMExpertModule()
{
}


// --------------------------------------Module----------------------------------------------
void KLMExpertModule::initialize()
{
  // require existence of necessary datastore obj

  m_klmClusters.isRequired();

  m_klids.registerInDataStore();
  m_klmClusters.registerRelationTo(m_klids);


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

  //overwritten at the end of the cluster loop
  KlId* klid = nullptr;

  // loop thru clusters in event and classify
  for (KLMCluster& cluster : m_klmClusters) {

    const TVector3& clusterPos = cluster.getClusterPosition();

    // get various KLMCluster vars
    m_KLMglobalZ         = clusterPos.Z();
    m_KLMnCluster        = m_klmClusters.getEntries();
    m_KLMnLayer          = cluster.getLayers();
    m_KLMnInnermostLayer = cluster.getInnermostLayer();
    m_KLMtime            = cluster.getTime();
    m_KLMenergy          = cluster.getEnergy();
    m_KLMhitDepth        = cluster.getClusterPosition().Mag();

    // find nearest ecl cluster and calculate distance
    pair<ECLCluster*, double> closestECLAndDist = findClosestECLCluster(clusterPos);
    ECLCluster* closestECLCluster = get<0>(closestECLAndDist);
    m_KLMECLDist = get<1>(closestECLAndDist);

    // get variables of the closest ECL cluster might be removed in future
    if (!(closestECLCluster == nullptr)) {
      m_KLMECLE                = closestECLCluster -> getEnergy();
      m_KLMECLE9oE25           = closestECLCluster -> getE9oE21();
      m_KLMECLTerror           = closestECLCluster -> getDeltaTime99();
      m_KLMECLTiming           = closestECLCluster -> getTime();
      m_KLMECLEerror           = closestECLCluster -> getUncertaintyEnergy();
      m_KLMECLdeltaL           = closestECLCluster -> getDeltaL();
      m_KLMECLminTrackDist     = closestECLCluster -> getMinTrkDistance();
      m_KLMECLZMVA             = closestECLCluster -> getZernikeMVA();
      m_KLMECLZ40              = closestECLCluster -> getAbsZernike40();
      m_KLMECLZ51              = closestECLCluster -> getAbsZernike51();
    } else {
      m_KLMECLdeltaL           = -999;
      m_KLMECLminTrackDist     = -999;
      m_KLMECLE                = -999;
      m_KLMECLE9oE25           = -999;
      m_KLMECLTiming           = -999;
      m_KLMECLTerror           = -999;
      m_KLMECLEerror           = -999;
      m_KLMECLZMVA             = -999;
      m_KLMECLZ40              = -999;
      m_KLMECLZ51              = -999;
    }

    // calculate distance to next cluster
    tuple<const KLMCluster*, double, double> closestKLMAndDist = findClosestKLMCluster(clusterPos);
    m_KLMnextCluster = get<1>(closestKLMAndDist);
    m_KLMavInterClusterDist = get<2>(closestKLMAndDist);

    m_KLMTrackSepDist         = -999;
    m_KLMTrackSepAngle        = -999;
    m_KLMInitialTrackSepAngle = -999;
    m_KLMTrackRotationAngle   = -999;
    m_KLMTrackClusterSepAngle = -999;

    auto trackSeperations = cluster.getRelationsTo<TrackClusterSeparation>();
    float best_dist = 1e10;
    float dist;
    for (auto trackSeperation :  trackSeperations) {
      dist = trackSeperation.getDistance();
      if (dist < best_dist) {
        best_dist = dist;
        m_KLMTrackSepDist         = trackSeperation.getDistance();
        m_KLMTrackSepAngle        = trackSeperation.getTrackClusterAngle();
        m_KLMInitialTrackSepAngle = trackSeperation.getTrackClusterInitialSeparationAngle();
        m_KLMTrackRotationAngle   = trackSeperation.getTrackRotationAngle();
        m_KLMTrackClusterSepAngle = trackSeperation.getTrackClusterSeparationAngle();
      }
    }

//    reduced vars set
    m_feature_variables[0] = m_KLMnLayer;
    m_feature_variables[1] = m_KLMnInnermostLayer;
    m_feature_variables[2] = m_KLMglobalZ;
    m_feature_variables[3] = m_KLMtime;
    m_feature_variables[4] = m_KLMnextCluster;
    m_feature_variables[5] = m_KLMenergy;
    m_feature_variables[6] = m_KLMTrackSepDist;
    m_feature_variables[7] = m_KLMInitialTrackSepAngle;
    m_feature_variables[8] = m_KLMTrackRotationAngle;
    m_feature_variables[9] = m_KLMTrackSepAngle;
    m_feature_variables[10] = m_KLMhitDepth;
    m_feature_variables[11] = m_KLMECLE;
    m_feature_variables[12] = m_KLMECLE9oE25;
    m_feature_variables[13] = m_KLMECLTiming;
    m_feature_variables[14] = m_KLMECLminTrackDist;
    m_feature_variables[15] = m_KLMECLZMVA;
    m_feature_variables[16] = m_KLMECLZ40;
    m_feature_variables[17] = m_KLMECLZ51;


    for (unsigned int i = 0; i < m_feature_variables.size(); ++i) {
      if (std::isfinite(m_feature_variables[i])) { m_feature_variables[i] = -999; }
      m_dataset->m_input[i] = m_feature_variables[i];
    }

    double IDMVAOut = m_expert->apply(*m_dataset)[0];
    B2DEBUG(175, "KLM Expert classification: " << IDMVAOut);
    klid = m_klids.appendNew();
    cluster.addRelationTo(klid, IDMVAOut);

  }// for cluster in clusters
} // event
