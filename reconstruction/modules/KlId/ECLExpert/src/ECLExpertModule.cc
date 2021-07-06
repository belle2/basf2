/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/modules/KlId/ECLExpert/ECLExpertModule.h>
#include <mdst/dataobjects/KlId.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

#include <mdst/dataobjects/ECLCluster.h>
#include <tracking/dataobjects/TrackClusterSeparation.h>

#include <mva/interface/Interface.h>
#include <boost/algorithm/string/predicate.hpp>

// here's where the functions are hidden
#include "reconstruction/modules/KlId/KLMExpert/KlId.h"

using namespace Belle2;
using namespace KlongId;
using namespace std;

REG_MODULE(ECLExpert);

ECLExpertModule::ECLExpertModule(): Module(), m_feature_variables(17, 0) //12
  //ECLExpertModule::ECLExpertModule(): Module(), m_feature_variables(9, 0) //12
{
  setDescription("Use to calculate KlId for each ECL cluster.");
  addParam("classifierPath", m_identifier,
           "path to the classifier you want to use. It is recommended to use the default classifiers and not to mess around with this.",
           m_identifier);
  setPropertyFlags(c_ParallelProcessingCertified);
}



ECLExpertModule::~ECLExpertModule() = default;

// --------------------------------------Module----------------------------------------------
void ECLExpertModule::initialize()
{
  // require existence of necessary datastore obj

  m_eclClusters.isRequired();

  m_klids.registerInDataStore();
  m_eclClusters.registerRelationTo(m_klids);


  if (not(boost::ends_with(m_identifier, ".root") or boost::ends_with(m_identifier, ".xml"))) {
    m_weightfile_representation = std::make_unique<DBObjPtr<DatabaseRepresentationOfWeightfile>>(MVA::makeSaveForDatabase(
                                    m_identifier));
  }

  MVA::AbstractInterface::initSupportedInterfaces();
  B2INFO(getName().data() << "::initialize() Using BDT " << m_identifier.data() << " with " << m_feature_variables.size() <<
         " variables");
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
/** init mva file taken from mva/ExpertModule */
void ECLExpertModule::init_mva(MVA::Weightfile& weightfile)
{

  auto supported_interfaces = MVA::AbstractInterface::getSupportedInterfaces();
  MVA::GeneralOptions general_options;
  weightfile.getOptions(general_options);

  m_expert = supported_interfaces[general_options.m_method]->getExpert();
  m_expert->load(weightfile);

  std::vector<float> dummy;
  dummy.resize(m_feature_variables.size(), 0);
  m_dataset = std::make_unique<MVA::SingleDataset>(general_options, dummy, 0);

}


void ECLExpertModule::event()
{
  // Use the neutralHadron hypothesis for the ECL
  const ECLCluster::EHypothesisBit eclHypothesis = ECLCluster::EHypothesisBit::c_neutralHadron;

  //overwritten at the end of the cluster loop
  KlId* klid = nullptr;

  // loop thru clusters in event and classify
  for (ECLCluster& cluster : m_eclClusters) {

    if (!cluster.hasHypothesis(eclHypothesis)) {continue;}

    // Cluster properties
    m_ECLminTrkDistance                 = cluster.getMinTrkDistance();
    m_ECLDeltaL                            = cluster.getDeltaL();
    m_ECLZMVA       = cluster.getZernikeMVA();
    m_ECLZ40        = cluster.getAbsZernike40();
    m_ECLZ51        = cluster.getAbsZernike51();
    m_ECLE1oE9        = cluster.getE1oE9();
    m_ECLE9oE21       = cluster.getE9oE21();
    m_ECLsecondMoment     = cluster.getSecondMoment();
    m_ECLLAT        = cluster.getLAT();
    m_ECLnumberOfCrystals   = cluster.getNumberOfCrystals();
    m_ECLtime       = cluster.getTime();
    m_ECLdeltaTime99      = cluster.getDeltaTime99();
    m_ECLtheta        = cluster.getTheta();
    m_ECLphi        = cluster.getPhi();
    m_ECLr        = cluster.getR();
    m_ECLPulseShapeDiscriminationMVA  = cluster.getPulseShapeDiscriminationMVA();
    m_ECLNumberOfHadronDigits   = cluster.getNumberOfHadronDigits();
    m_ECLEnergy                         = cluster.getEnergy(eclHypothesis);
    m_ECLlogEnergy            = cluster.getEnergyRaw();
    m_ECLlogEnergyHighestCrystal  = cluster.getEnergyHighestCrystal();


//    reduced vars sets
//    m_feature_variables.clear();
    m_feature_variables[0] = m_ECLminTrkDistance;
    m_feature_variables[1] = m_ECLDeltaL;
    m_feature_variables[2] = m_ECLZMVA;
    m_feature_variables[3] = m_ECLZ40;
    m_feature_variables[4] = m_ECLZ51;
    m_feature_variables[5] = m_ECLE1oE9;
    //    m_feature_variables[] = m_ECLE9oE21;
    m_feature_variables[6] = m_ECLsecondMoment;
    m_feature_variables[7] = m_ECLLAT;
    //    m_feature_variables[] = m_ECLnumberOfCrystals;
    m_feature_variables[8] = m_ECLtime;
    m_feature_variables[9] = m_ECLdeltaTime99;
    m_feature_variables[10] = m_ECLtheta;
    m_feature_variables[11] = m_ECLphi;
    m_feature_variables[12] = m_ECLr;
    m_feature_variables[13] = m_ECLPulseShapeDiscriminationMVA;
    //    m_feature_variables[] = m_ECLNumberOfHadronDigits;
    m_feature_variables[14] = m_ECLEnergy;
    m_feature_variables[15] = m_ECLlogEnergy;
    m_feature_variables[16] = m_ECLlogEnergyHighestCrystal;

    for (unsigned int i = 0; i < m_feature_variables.size(); ++i) {
      if (!std::isfinite(m_feature_variables[i])) { m_feature_variables[i] = -999; }
      m_dataset->m_input[i] = m_feature_variables[i];
    }

    double IDMVAOut = m_expert->apply(*m_dataset)[0];
    B2DEBUG(175, "ECL Expert classification: " << IDMVAOut);
    klid = m_klids.appendNew();
    cluster.addRelationTo(klid, IDMVAOut);

  }// for cluster in clusters
} // event
