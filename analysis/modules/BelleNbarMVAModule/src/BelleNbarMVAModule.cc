/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/database/DBObjPtr.h>
#include <framework/database/Database.h>
#include <mva/dataobjects/DatabaseRepresentationOfWeightfile.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <vector>
#include <analysis/modules/BelleNbarMVAModule/BelleNbarMVAModule.h>
#define FDEEP_FLOAT_TYPE double
#include <string>
#include <fdeep/fdeep.hpp>

using namespace Belle2;

REG_MODULE(BelleNbarMVA)

BelleNbarMVAModule::BelleNbarMVAModule() : Module(),
  m_model(fdeep::read_model_from_string(((DatabaseRepresentationOfWeightfile*)Database::Instance().getData("nbarMVA", 0, 0))->m_data))
{
  setDescription(R"DOC(Apply nbarMVA for Belle I)DOC");
  addParam("particleList", m_particleList, "ParticleList to apply the MVA", std::string(""));
}

void BelleNbarMVAModule::initialize()
{
  m_plist.isRequired(m_particleList);
}

void BelleNbarMVAModule::event()
{
  const unsigned int n = m_plist->getListSize();
  for (unsigned i = 0; i < n; i++) {
    Particle* particle = m_plist->getParticle(i);
    const ECLCluster* cluster = particle->getECLCluster();
    double transform_clusterE = (cluster->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons) - 1.01092414) / 0.47629791;
    double transform_clusterE9E25 = (cluster->getE9oE21() - 0.86381786) / 0.12698416;
    double transform_clusterHighestE = (cluster->getEnergyHighestCrystal() - 0.48924759) / 0.30376054;
    double transform_clusterNHits = (cluster->getNumberOfCrystals() - 14.32496591) / 3.55654319;
    double transform_clusterLAT = (cluster->getLAT() - 5.59208538) / 1.80608294;
    std::vector<double> v{transform_clusterE, transform_clusterE9E25, transform_clusterHighestE, transform_clusterNHits, transform_clusterLAT};
    particle->addExtraInfo("nbarMVA", double(m_model.predict_single_output({fdeep::tensor(fdeep::tensor_shape(5), v)})));
  }
}

