/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <generators/utilities/InitialParticleGeneration.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <Math/RotationY.h>

namespace Belle2 {

  void InitialParticleGeneration::initialize()
  {
    m_event.registerInDataStore();
  }

  TVector3 InitialParticleGeneration::generateVertex(const TVector3& initial, const TMatrixDSym& cov,
                                                     MultivariateNormalGenerator& gen)
  {
    if (m_event->hasGenerationFlags(BeamParameters::c_smearVertex)) {
      if (!gen.size()) gen.setMeanCov(initial, cov);
      return gen.generateVec3();
    }
    return initial;
  }

  ROOT::Math::PxPyPzEVector InitialParticleGeneration::generateBeam(const ROOT::Math::PxPyPzEVector& initial, const TMatrixDSym& cov,
      MultivariateNormalGenerator& gen)
  {
    //no smearing, nothing to do
    if (!(m_event->getGenerationFlags() & BeamParameters::c_smearBeam))
      return initial;

    //init random generator if there is a change in beam parameters or at the beginning
    if (gen.size() != 3) gen.setMeanCov(TVector3(0, 0, 0), cov);

    //generate the actuall smearing delta vector (deltaE, deltaThX, deltaThY)
    Eigen::VectorXd p = gen.generate();

    // if we don't smear beam energy set the deltaE to zero
    if (!m_event->hasGenerationFlags(BeamParameters::c_smearBeamEnergy))
      p[0] = 0;

    // if we don't smear beam direction set deltaThX and deltaThY to 0
    if (!m_event->hasGenerationFlags(BeamParameters::c_smearBeamDirection)) {
      p[1] = 0;
      p[2] = 0;
    }

    //Calculate initial parameters of the beam before smearing
    double E0   = initial.E();
    double thX0 = atan(initial.Px() / initial.Pz());
    double thY0 = atan(initial.Py() / initial.Pz());

    //Apply smearing
    double E   = E0   + p[0];
    double thX = thX0 + p[1];
    double thY = thY0 + p[2];

    //Convert values back to 4-vector
    int dir = (initial.Pz() > 0) ? 1 : -1;
    double pz =  dir * sqrt(E * E - Const::electronMass * Const::electronMass) / sqrt(1 + pow(tan(thX), 2) + pow(tan(thY), 2));
    return ROOT::Math::PxPyPzEVector(pz * tan(thX), pz * tan(thY), pz, E);

  }

  MCInitialParticles& InitialParticleGeneration::generate()
  {
    return generate(m_allowedFlags);
  }

  MCInitialParticles& InitialParticleGeneration::generate(int allowedFlags)
  {
    if (!m_event) {
      m_event.create();
    }
    if (!m_beamParams.isValid()) {
      B2FATAL("Cannot generate beam without valid BeamParameters");
    }
    if (m_beamParams.hasChanged()) {
      m_generateHER.reset();
      m_generateLER.reset();
      m_generateVertex.reset();
    }
    m_event->setGenerationFlags(m_beamParams->getGenerationFlags() & allowedFlags);
    ROOT::Math::PxPyPzEVector her = generateBeam(m_beamParams->getHER(), m_beamParams->getCovHER(), m_generateHER);
    ROOT::Math::PxPyPzEVector ler = generateBeam(m_beamParams->getLER(), m_beamParams->getCovLER(), m_generateLER);
    TVector3 vtx = generateVertex(m_beamParams->getVertex(), m_beamParams->getCovVertex(), m_generateVertex);
    m_event->set(her, ler, vtx);
    //Check if we want to go to CMS, if so boost both
    if (m_beamParams->hasGenerationFlags(BeamParameters::c_generateCMS)) {
      m_event->setGenerationFlags(0);
      her = m_event->getLabToCMS() * her;
      ler = m_event->getLabToCMS() * ler;
      m_event->set(her, ler, vtx);
      m_event->setGenerationFlags(m_beamParams->getGenerationFlags() & allowedFlags);
    }
    return *m_event;
  }

  TVector3 InitialParticleGeneration::updateVertex(bool force)
  {
    if (!m_beamParams.isValid()) {
      B2FATAL("Cannot generate beam without valid BeamParameters");
    }
    if (!m_event) {
      // generate a new mc initial particle without smearing except for the vertex
      generate(BeamParameters::c_smearVertex);
      return m_event->getVertex();
    }
    if (!m_beamParams->hasGenerationFlags(BeamParameters::c_smearVertex) or
        (m_event->hasGenerationFlags(BeamParameters::c_smearVertex) and not force)) {
      // already has a smeared vertex or smearing disallowed. nothing to do
      return {0, 0, 0};
    }
    // Ok, now we need to just update the vertex, make sure the parameters are up to date ...
    if (m_beamParams.hasChanged()) {
      m_generateHER.reset();
      m_generateLER.reset();
      m_generateVertex.reset();
    }
    // Add the smear vertex flag
    m_event->setGenerationFlags(m_event->getGenerationFlags() | BeamParameters::c_smearVertex);
    // And generate a vertex ...
    auto previous = m_event->getVertex();
    auto vtx = generateVertex(m_beamParams->getVertex(), m_beamParams->getCovVertex(), m_generateVertex);
    m_event->setVertex(vtx);
    // Everything done
    return vtx - previous;
  }
} //Belle2 namespace
