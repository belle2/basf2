/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sasha Glazov                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <analysis/dataobjects/Particle.h>
#include <framework/datastore/StoreObjPtr.h>
#include <analysis/dataobjects/ParticleList.h>
#include <TRandom.h>

namespace Belle2 {
  /**
  *This set of module is designed for tracking sysetmatics studies
  *
  */


  /**
   * Tracking efficiency systematics
   */
  class TrackingEfficiencyModule : public Module {
  public:

    /**
    * Constructor: Sets the description, the properties and the parameters of the module.
    */
    TrackingEfficiencyModule()
    {
      setDescription(
        R"DOC(Module to remove tracks from the lists at random. Include in your code as
		     
mypath.add_module("TrackingEfficiency",particleLists=['pi+:cut'],prob=0.01)

The module modifies the input particleLists by randomly removing tracks with the probability prob.
		     
		     )DOC");
      // Parameter definitions
      addParam("particleLists", m_ParticleLists, "input particle lists");
      addParam("prob", m_prob, "probability to remove the particle");
    }

    virtual void event() override
    {
      for (auto& iList : m_ParticleLists) {
        StoreObjPtr<ParticleList> particleList(iList);

        //check particle List exists and has particles
        if (!particleList) {
          B2ERROR("ParticleList " << iList << " not found");
          continue;
        }
        std::vector<unsigned int> toRemove;
        size_t nPart = particleList->getListSize();
        for (size_t iPart = 0; iPart < nPart; iPart++) {
          auto particle = particleList->getParticle(iPart);
          const Track* track = particle->getTrack();
          if (track != nullptr) {
            /// got a track
            auto prob = gRandom->Uniform();
            if (prob < m_prob)
              toRemove.push_back(particle->getArrayIndex());
          }
        }
        particleList->removeParticles(toRemove);
      }
    }

  private:
    /** input particle lists */
    std::vector<std::string> m_ParticleLists;
    /** imput probability to remove the particle */
    double m_prob;

  }; //TrackingEfficiencyModule


  /**
  * Tracking momentum systematics
  */
  class TrackingMomentumModule : public Module {

  public:

    /**
    * Constructor: Sets the description, the properties and the parameters of the module.
    */
    TrackingMomentumModule()
    {
      setDescription(
        R"DOC(Module to modify momentum of tracks from the lists. Include in your code as
		     
mypath.add_module("TrackingMomentum",particleLists=['pi+:cut'],scale=0.999)

The module modifies the input particleLists by scaling track momenta as given by the parameter scale
		     
		     )DOC");
      // Parameter definitions
      addParam("particleLists", m_ParticleLists, "input particle lists");
      addParam("scale", m_scale, "scale factor to be applied to 3-momentum");
    }

    virtual void event() override
    {
      for (auto& iList : m_ParticleLists) {
        StoreObjPtr<ParticleList> particleList(iList);

        //check particle List exists and has particles
        if (!particleList) {
          B2ERROR("ParticleList " << iList << " not found");
          continue;
        }
        size_t nPart = particleList->getListSize();
        for (size_t iPart = 0; iPart < nPart; iPart++) {
          auto particle = particleList->getParticle(iPart);
          const Track* track = particle->getTrack();
          if (track != nullptr) {
            /// got a track, scale
            double m = particle->getMass();
            TVector3 p = m_scale * particle->getMomentum();
            double e = sqrt(p.Mag2() + m * m);
            particle->set4Vector(TLorentzVector(p, e));
          }
        }
      }
    }

  private:
    /** input particle lists */
    std::vector<std::string> m_ParticleLists;
    /** imput momentum scale modifier */
    double m_scale;

  }; // TrackingMomentumModule


}; //namespace

