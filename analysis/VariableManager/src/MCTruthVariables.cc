/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sam Cunliffe                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/VariableManager/MCTruthVariables.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/Particle.h>

#include <mdst/dataobjects/MCParticle.h>

#include <framework/datastore/RelationsObject.h>
#include <framework/gearbox/Const.h>

namespace Belle2 {
  namespace Variable {

    double isReconstructible(const Particle* p)
    {
      if (p->getParticleType() == Particle::EParticleType::c_Composite)
        return -1.0;
      const MCParticle* mcp = p->getRelated<MCParticle>();
      if (!mcp)
        return std::numeric_limits<float>::quiet_NaN();

      // If charged: make sure it was seen in the SVD.
      // If neutral: make sure it was seen in the ECL.
      if (abs(mcp->getCharge()) > 0)
        return seenInSVD(p);
      else
        return seenInECL(p);
    }

    double seenInPXD(const Particle* p)
    {
      if (p->getParticleType() == Particle::EParticleType::c_Composite)
        return -1.0;
      const MCParticle* mcp = p->getRelated<MCParticle>();
      if (!mcp)
        return std::numeric_limits<float>::quiet_NaN();
      return (double)mcp->hasSeenInDetector(Const::PXD);
    }

    double seenInSVD(const Particle* p)
    {
      if (p->getParticleType() == Particle::EParticleType::c_Composite)
        return -1.0;
      const MCParticle* mcp = p->getRelated<MCParticle>();
      if (!mcp)
        return std::numeric_limits<float>::quiet_NaN();
      return (double)mcp->hasSeenInDetector(Const::SVD);
    }

    double seenInCDC(const Particle* p)
    {
      if (p->getParticleType() == Particle::EParticleType::c_Composite)
        return -1.0;
      const MCParticle* mcp = p->getRelated<MCParticle>();
      if (!mcp)
        return std::numeric_limits<float>::quiet_NaN();
      return (double)mcp->hasSeenInDetector(Const::CDC);
    }

    double seenInTOP(const Particle* p)
    {
      if (p->getParticleType() == Particle::EParticleType::c_Composite)
        return -1.0;
      const MCParticle* mcp = p->getRelated<MCParticle>();
      if (!mcp)
        return std::numeric_limits<float>::quiet_NaN();
      return (double)mcp->hasSeenInDetector(Const::TOP);
    }

    double seenInECL(const Particle* p)
    {
      if (p->getParticleType() == Particle::EParticleType::c_Composite)
        return -1.0;
      const MCParticle* mcp = p->getRelated<MCParticle>();
      if (!mcp)
        return std::numeric_limits<float>::quiet_NaN();
      return (double)mcp->hasSeenInDetector(Const::ECL);
    }

    double seenInARICH(const Particle* p)
    {
      if (p->getParticleType() == Particle::EParticleType::c_Composite)
        return -1.0;
      const MCParticle* mcp = p->getRelated<MCParticle>();
      if (!mcp)
        return std::numeric_limits<float>::quiet_NaN();
      return (double)mcp->hasSeenInDetector(Const::ARICH);
    }

    double seenInKLM(const Particle* p)
    {
      if (p->getParticleType() == Particle::EParticleType::c_Composite)
        return -1.0;
      const MCParticle* mcp = p->getRelated<MCParticle>();
      if (!mcp)
        return std::numeric_limits<float>::quiet_NaN();
      return (double)mcp->hasSeenInDetector(Const::KLM);
    }

    VARIABLE_GROUP("MC particle seen in subdetectors");
    REGISTER_VARIABLE("isReconstructible", isReconstructible,
                      "checks charged particles were seen in the SVD and neutrals in the ECL, returns 1.0 if so, 0.0 if not, -1.0 for composite particles.");
    REGISTER_VARIABLE("seenInPXD", seenInPXD,
                      "returns 1.0 if the MC particle was seen in the PXD, 0.0 if not, -1.0 for composite particles.");
    REGISTER_VARIABLE("seenInSVD", seenInSVD,
                      "returns 1.0 if the MC particle was seen in the SVD, 0.0 if not, -1.0 for composite particles.");
    REGISTER_VARIABLE("seenInCDC", seenInCDC,
                      "returns 1.0 if the MC particle was seen in the CDC, 0.0 if not, -1.0 for composite particles.");
    REGISTER_VARIABLE("seenInTOP", seenInTOP,
                      "returns 1.0 if the MC particle was seen in the TOP, 0.0 if not, -1.0 for composite particles.");
    REGISTER_VARIABLE("seenInECL", seenInECL,
                      "returns 1.0 if the MC particle was seen in the ECL, 0.0 if not, -1.0 for composite particles.");
    REGISTER_VARIABLE("seenInARICH", seenInARICH,
                      "returns 1.0 if the MC particle was seen in the ARICH, 0.0 if not, -1.0 for composite particles.");
    REGISTER_VARIABLE("seenInKLM", seenInKLM,
                      "returns 1.0 if the MC particle was seen in the KLM, 0.0 if not, -1.0 for composite particles.");

  }
}


