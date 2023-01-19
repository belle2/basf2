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

BelleNbarMVAModule::BelleNbarMVAModule() : Module(), m_model()
{
  setDescription(
    R"DOC(Apply nbarMVA for Belle I, a DNN trained with ECLCluster variables for discrimination of anti-neutrons against photons. Anti-neutron-like particles have scores closer to 1; photon-like particles have scores closer to 0.)DOC");
  addParam("identifier", m_identifier, "Identifier of the MVA", std::string(""));
  addParam("particleList", m_particleList, "ParticleList to apply the MVA", std::string(""));
}

void BelleNbarMVAModule::initialize()
{
  m_model = std::make_unique<fdeep::model>(fdeep::read_model_from_string(((DatabaseRepresentationOfWeightfile*)
                                           Database::Instance().getData(m_identifier, 0, 0))->m_data));
  m_plist.isRequired(m_particleList);
}

void BelleNbarMVAModule::event()
{
  for (Particle& particle : *m_plist) {
    const ECLCluster* cluster = particle.getECLCluster();
    std::vector<double> v{
      cluster->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons),
      cluster->getE9oE21(),
      cluster->getEnergyHighestCrystal(),
      cluster->getNumberOfCrystals(),
      cluster->getLAT()
    };
    particle.addExtraInfo("nbarMVA", double(m_model->predict_single_output({fdeep::tensor(fdeep::tensor_shape(5), v)})));
  }
}

