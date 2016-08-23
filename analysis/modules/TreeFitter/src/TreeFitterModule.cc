/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Francesco Tenchini                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//Implementation of Decay Tree Fitter based on arXiv:physics/0503191

#include <analysis/modules/TreeFitter/TreeFitterModule.h>
#include <analysis/modules/TreeFitter/Fitter.h>
//
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/Particle.h>
//
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
//
// Magnetic field
#include <framework/geometry/BFieldManager.h>

using namespace Belle2;

// Register Module
REG_MODULE(TreeFitter)

// Constructor
TreeFitterModule::TreeFitterModule() : Module(),
  m_Bfield(0)
{
  setDescription("Tree Fitter module. Performs simultaneous fit of all vertices in a decay chain.");
  //module parameters
  addParam("particleList", m_particleList, "Input mother of the decay tree to fit");
  addParam("confidenceLevel", m_confidenceLevel, "Confidence level to accept fitted decay tree. -1.0 for failed fits.", 0.0);
  addParam("convergencePrecision", m_precision, "Upper limit for chi2 fluctuations to accept result.", 1.0); //large value for now
  addParam("verbose", m_verbose, "BaBar verbosity (to be phased out in the future)", 5);
}

// Destructor
TreeFitterModule::~TreeFitterModule() {}

// Initializer
void TreeFitterModule::initialize()
{
  // get magnetic field
  m_Bfield = BFieldManager::getField(TVector3(0, 0, 0)).Z() / Unit::T;
  //Make sure we have a particle list
  StoreObjPtr<ParticleList>::required(m_particleList);
  //Also make sure we have actual particles
  StoreArray<Particle> particles;
  particles.isRequired();
}

// Start of Run Execution
void TreeFitterModule::beginRun() {}

// Event Loop
void TreeFitterModule::event()
{
  // input Particle
  StoreObjPtr<ParticleList> plist(m_particleList);
  if (!plist) {
    B2ERROR("ParticleList " << m_particleList << " not found");
    return;
  }
  std::vector<unsigned int> toRemove;
  unsigned int n = plist->getListSize();
  for (unsigned i = 0; i < n; i++) {
    Particle* particle = plist->getParticle(i);
    bool ok = doTreeFit(particle);
    if (!ok) particle->setPValue(-1);
    if (particle->getPValue() < m_confidenceLevel) {
      toRemove.push_back(particle->getArrayIndex());
    }
  }
  plist->removeParticles(toRemove);

}

// End of Run Execution
void TreeFitterModule::endRun() {}
//
void TreeFitterModule::terminate() {}

//Actual Fit Call
bool TreeFitterModule::doTreeFit(Particle* head)
{
  TreeFitter::Fitter* TreeFitObject = new TreeFitter::Fitter(head, m_precision);
  TreeFitObject->setVerbose(m_verbose);
  bool rc = TreeFitObject->fit();
  return rc;
}
