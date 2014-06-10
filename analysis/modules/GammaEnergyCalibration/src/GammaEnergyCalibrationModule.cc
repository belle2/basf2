/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/GammaEnergyCalibration/GammaEnergyCalibrationModule.h>
#include <analysis/utility/PSelectorFunctions.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// dataobjects
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/utility/VariableManager.h>


using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(GammaEnergyCalibration)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

GammaEnergyCalibrationModule::GammaEnergyCalibrationModule() : Module()
{
  // Set module properties
  setDescription("Calibrates energy of the photons");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Parameter definitions
  addParam("gammaListName", m_gammaListName, "Input ParticleList name", std::string(""));

}

void GammaEnergyCalibrationModule::initialize()
{
  if (!m_gammaListName.empty()) {
    // obtain the input and output particle lists from the decay string
    bool valid = m_decaydescriptor.init(m_gammaListName);
    if (!valid)
      B2ERROR("GammaEnergyCalibrationModule::initialize Invalid input DecayString: " << m_gammaListName);

    if (m_decaydescriptor.getMother()->getPDGCode() != 22)
      B2ERROR("GammaEnergyCalibrationModule::initialize Invalid input DecayString: " << m_gammaListName << ". This module works only with photons.");

    int nProducts = m_decaydescriptor.getNDaughters();
    if (nProducts > 0)
      B2ERROR("GammaEnergyCalibrationModule::initialize Invalid input DecayString " << m_gammaListName
              << ". DecayString should not contain any daughters, only the mother particle.");
  } else
    B2ERROR("GammaEnergyCalibrationModule::initialize Empty list name!");

}

void GammaEnergyCalibrationModule::event()
{
  StoreObjPtr<ParticleList> plist(m_gammaListName);
  if (!plist) {
    B2ERROR("ParticleList " << m_gammaListName << " not found");
    return;
  }

  for (unsigned i = 0; i < plist->getListSize(); i++) {
    Particle* particle = plist->getParticle(i);

    calibrateEnergy(particle);
  }

}

void GammaEnergyCalibrationModule::calibrateEnergy(Particle* particle) const
{
  double oldE = particle->getEnergy();
  double newE = oldE - 0.041;

  if (newE < 0)
    return;

  // TODO: can be optimized for speed
  double theta = analysis::eclClusterTheta(particle);
  double phi   = analysis::eclClusterPhi(particle);

  TLorentzVector newLorentzVector(float(newE * sin(theta) * cos(phi)), float(newE * sin(theta) * sin(phi)), float(newE * cos(theta)), newE);

  particle->set4Vector(newLorentzVector);
}
