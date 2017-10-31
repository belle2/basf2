/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//Implementation of Decay Tree Fitter based on arXiv:physics/0503191
//Main module implementation

#include <analysis/modules/TreeFitter/TreeFitterModule.h>
#include <analysis/modules/TreeFitter/Fitter.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/Particle.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// Magnetic field
#include <framework/geometry/BFieldManager.h>

using namespace Belle2;

// Register Module
REG_MODULE(TreeFitter)

// Constructor
TreeFitterModule::TreeFitterModule() : Module()
{
  setDescription("Tree Fitter module. Performs simultaneous fit of all vertices in a decay chain.");
  addParam("particleList", m_particleList, "Input mother of the decay tree to fit");
  addParam("confidenceLevel", m_confidenceLevel, "Confidence level to accept fitted decay tree. -1.0 for failed fits.", 0.0);
  addParam("convergencePrecision", m_precision, "Upper limit for chi2 fluctuations to accept result.", 1.); //large value for now
  addParam("verbose", m_verbose, "BaBar verbosity (to be phased out in the future)", 5);
  addParam("massConstraintList", m_massConstraintList, "Type::[int]. List of particles to mass constrain with int = pdg code.");
  addParam("ipConstraintDimension", m_ipConstraintDimension,
           "Type::Int. Use the x-y-z-beamspot or x-y-beamtube constraint. Zero means no cosntraint which is the default. The Beamspot will be treated as the mother of the particlelist you feed.",
           0);
}


// Initializer
void TreeFitterModule::initialize()
{
  StoreObjPtr<ParticleList>::required(m_particleList);
  StoreArray<Belle2::Particle> particles;
  particles.isRequired();
}

// Start of run
void TreeFitterModule::beginRun()
{
}

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
    Belle2::Particle* particle = plist->getParticle(i);

    bool ok = doTreeFit(particle);

    if (!ok) {
      particle->setPValue(-1);
    }

    if (particle->getPValue() < m_confidenceLevel) {
      toRemove.push_back(particle->getArrayIndex());
    }
  }
  plist->removeParticles(toRemove);
}

bool TreeFitterModule::doTreeFit(Belle2::Particle* head)
{
  TreeFitter::Fitter* TreeFitObject = new TreeFitter::Fitter(head, m_precision, m_ipConstraintDimension);
  TreeFitObject->setVerbose(m_verbose);
  TreeFitObject->setMassConstraintList(m_massConstraintList); // JFK: move to constrcutor Fri 08 Sep 2017 04:48:21 AM CEST

  bool rc = TreeFitObject->fit();

  delete TreeFitObject; //clean up statement, consider using unique_ptr<TreeFitter::Fitter> in the future
  return rc;
}
