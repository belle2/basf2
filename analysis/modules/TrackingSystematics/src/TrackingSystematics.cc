#include <analysis/modules/TrackingSystematics/TrackingSystematics.h>

using namespace Belle2;

REG_MODULE(TrackingEfficiency);
REG_MODULE(TrackingMomentum);

TrackingEfficiencyModule::TrackingEfficiencyModule():
  m_frac(0)
{
  setDescription(
    R"DOC(Module to remove tracks from the lists at random. Include in your code as
		     
.. code:: python

mypath.add_module("TrackingEfficiency",particleLists=['pi+:cut'],frac=0.01)

The module modifies the input particleLists by randomly removing tracks with the probability frac.
		     
		     )DOC");
  // Parameter definitions
  addParam("particleLists", m_ParticleLists, "input particle lists");
  addParam("frac", m_frac, "probability to remove the particle");
}

void TrackingEfficiencyModule::event()
{
  for (auto& iList : m_ParticleLists) {
    StoreObjPtr<ParticleList> particleList(iList);
    //check particle List exists and has particles
    if (!particleList) {
      B2ERROR("ParticleList " << iList << " not found");
      continue;
    }

    if (!Const::chargedStableSet.contains(Const::ParticleType(abs(particleList->getPDGCode())))) {
      B2ERROR("The provided particlelist " << iList << " does not contain track-based particles.");
    }

    std::vector<unsigned int> toRemove;
    size_t nPart = particleList->getListSize();
    for (size_t iPart = 0; iPart < nPart; iPart++) {
      auto particle = particleList->getParticle(iPart);
      auto prob = gRandom->Uniform();
      if (prob < m_frac)
        toRemove.push_back(particle->getArrayIndex());
    }
    particleList->removeParticles(toRemove);
  }
}

TrackingMomentumModule::TrackingMomentumModule()
{
  setDescription(
    R"DOC(Module to modify momentum of tracks from the lists. Include in your code as
		     
.. code:: python

mypath.add_module("TrackingMomentum",particleLists=['pi+:cut'],scale=0.999)

The module modifies the input particleLists by scaling track momenta as given by the parameter scale
		     
		     )DOC");
  // Parameter definitions
  addParam("particleLists", m_ParticleLists, "input particle lists");
  addParam("scale", m_scale, "scale factor to be applied to 3-momentum");
}

void TrackingMomentumModule::event()
{
  for (auto& iList : m_ParticleLists) {
    StoreObjPtr<ParticleList> particleList(iList);

    //check particle List exists and has particles
    if (!particleList) {
      B2ERROR("ParticleList " << iList << " not found");
      continue;
    }

    if (!Const::chargedStableSet.contains(Const::ParticleType(abs(particleList->getPDGCode())))) {
      B2ERROR("The provided particlelist " << iList << " does not contain track-based particles.");
    }

    size_t nPart = particleList->getListSize();
    for (size_t iPart = 0; iPart < nPart; iPart++) {
      auto particle = particleList->getParticle(iPart);

      double m = particle->getMass();
      TVector3 p = m_scale * particle->getMomentum();
      double e = sqrt(p.Mag2() + m * m);
      particle->set4Vector(TLorentzVector(p, e));
    }
  }
}
