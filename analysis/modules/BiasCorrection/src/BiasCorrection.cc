// Own include
#include <analysis/modules/BiasCorrection/BiasCorrection.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

#include <framework/datastore/StoreObjPtr.h>

#include <map>
#include <TRandom.h>

using namespace Belle2;

REG_MODULE(EnergyBiasCorrection);

EnergyBiasCorrectionModule::EnergyBiasCorrectionModule() : Module()
{
  setDescription(
    R"DOC(Module to modify energy from the lists. Include in your code as

    .. code:: python

        mypath.add_module("EnergyBiasCorrection", particleLists=['gamma:cut'], scale=0.999)

The module modifies the input particleLists by scaling energy as given by the parameter scale
		     
		     )DOC");
  // Parameter definitions
  addParam("particleLists", m_ParticleLists, "input particle lists");
  addParam("scale", m_scale, "scale factor to be applied to energy", 0.999);
}

void EnergyBiasCorrectionModule::event()
{
  for (auto& iList : m_ParticleLists) {
    StoreObjPtr<ParticleList> particleList(iList);

    //check particle List exists and has particles
    if (!particleList) {
      B2ERROR("ParticleList " << iList << " not found");
      continue;
    }

    // should it be just for photons?

    size_t nPart = particleList->getListSize();
    for (size_t iPart = 0; iPart < nPart; iPart++) {
      auto particle = particleList->getParticle(iPart);

      double m = particle->getMass();
      TVector3 p = particle->getMomentum();
      double e =  m_scale * sqrt(p.Mag2() + m * m);
      particle->set4Vector(TLorentzVector(p, e));
    }
  }
}