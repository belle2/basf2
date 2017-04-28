/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frederik Krohn                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <reconstruction/modules/KlId/ECLExpert/ECLExpertModule.h>
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

REG_MODULE(ECLExpert);

ECLExpertModule::ECLExpertModule(): Module(), m_feature_variables(6, 0)
{
  setDescription("Use to calculate KlId for each ECLCluster.");
  setPropertyFlags(c_ParallelProcessingCertified);
}



ECLExpertModule::~ECLExpertModule()
{
}


// --------------------------------------Module----------------------------------------------
void ECLExpertModule::initialize()
{
  StoreArray<RecoTrack>::required();
  StoreArray<ECLCluster>::required();

  StoreArray<KlId> klids;

  StoreArray<ECLCluster> eclClusters;
  eclClusters.registerRelationTo(klids);
  //run KLMTMVA Expert first


  if (not(boost::ends_with(m_identifier, ".root") or boost::ends_with(m_identifier, ".xml"))) {
    m_weightfile_representation = std::unique_ptr<DBObjPtr<DatabaseRepresentationOfWeightfile>>(new
                                  DBObjPtr<DatabaseRepresentationOfWeightfile>(m_identifier));
  }
  MVA::AbstractInterface::initSupportedInterfaces();

}


void ECLExpertModule::beginRun()
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

void ECLExpertModule::init_mva(MVA::Weightfile& weightfile)
{
  auto supported_interfaces = MVA::AbstractInterface::getSupportedInterfaces();
  MVA::GeneralOptions general_options;
  weightfile.getOptions(general_options);


  m_expert = supported_interfaces[general_options.m_method]->getExpert();
  m_expert->load(weightfile);

  std::vector<float> dummy;
  dummy.resize(m_feature_variables.size(), 0);
  m_dataset = std::unique_ptr<MVA::SingleDataset>(new MVA::SingleDataset(general_options, dummy, 0));
}




void ECLExpertModule::event()
{

  StoreArray<RecoTrack> genfitTracks;
  StoreArray<ECLCluster> eclClusters;
  StoreArray<KlId> klids;


  //overwritten at the end of the cluster loop
  double MVAOut;
  KlId* klid = nullptr;

  // loop thru clusters in event and classify
  for (ECLCluster& cluster : eclClusters) {

    // get various ECLCluster vars from getters
    m_ECLE              = cluster.getEnergy();
    m_ECLE9oE25         = cluster.getE9oE21();
    m_ECLTiming         = cluster.getTime();
    m_ECLEerror         = cluster.getUncertaintyEnergy();
    m_ECLminTrkDistance = cluster.getMinTrkDistance();
    m_ECLdeltaL         = cluster.getDeltaL();

    const TVector3& clusterPos = cluster.getClusterPosition();

    // find closest track
    // obsolete and slow. just kept for future toying.
    //  tuple<RecoTrack*, double, std::unique_ptr<const TVector3>> closestTrackAndDistance = findClosestTrack(clusterPos);
    //m_ECLtrackDist = get<1>(closestTrackAndDistance);


    m_feature_variables[0] = m_ECLE;
    m_feature_variables[1] = m_ECLE9oE25;
    m_feature_variables[2] = m_ECLTiming;
    m_feature_variables[3] = m_ECLEerror;
    m_feature_variables[4] = m_ECLminTrkDistance;
    m_feature_variables[5] = m_ECLdeltaL;


    // rewrite dataset
    for (unsigned int i = 0; i < m_feature_variables.size(); ++i) {
      m_dataset->m_input[i] = m_feature_variables[i];
    }

    //classify dartaset
    MVAOut = m_expert->apply(*m_dataset)[0];

    // KlId, bkg prob, KLM, ECL
    klid = klids.appendNew(MVAOut, -1 , 0, 1);
    cluster.addRelationTo(klid);

  }// for cluster in clusters
} // event

