/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/utilities/InitialParticleGeneration.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

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

  TLorentzVector InitialParticleGeneration::generateBeam(const TLorentzVector& initial, const TMatrixDSym& cov,
                                                         MultivariateNormalGenerator& gen)
  {
    if (!(m_event->getGenerationFlags() & BeamParameters::c_smearBeam)) {
      //no smearing, nothing to do
      return initial;
    }
    if (gen.size() != 3) gen.setMeanCov(TVector3(initial.E(), 0, 0), cov);
    // generate the beam parameters: energy and horizontal angle and vertical
    // angle with respect to nominal direction
    // p[0] = energy
    // p[1] = horizontal angle
    // p[2] = vertical angle
    Eigen::VectorXd p = gen.generate();
    // if we don't smear beam energy set it to nominal values
    if (!m_event->hasGenerationFlags(BeamParameters::c_smearBeamEnergy)) {
      p[0] = initial.E();
    }
    // if we don't smear beam direction set angles to 0
    if (!m_event->hasGenerationFlags(BeamParameters::c_smearBeamDirection)) {
      p[1] = 0;
      p[2] = 0;
    }
    // calculate TLorentzvector from p
    const double pz = std::sqrt(p[0] * p[0] - Const::electronMass * Const::electronMass);
    // Rotate around theta_x and theta_y. Same as result.RotateX(p[1]);
    // result.RotateY(p[2]) but as we know it's a 0,0,pz vector this can be
    // optimized.
    const double sx = sin(p[1]);
    const double cx = cos(p[1]);
    const double sy = sin(p[2]);
    const double cy = cos(p[2]);
    const double px = sy * cx * pz;
    const double py = -sx * pz;
    TLorentzVector result(px, py, cx * cy * pz, p[0]);
    // And rotate into the direction of the nominal beam
    result.RotateY(initial.Theta());
    return result;
  }

  MCInitialParticles& InitialParticleGeneration::generate(bool forceGeneration)
  {
    if (!m_event) {
      m_event.create();
    } else {
      if (m_event->getValidFlag() && !forceGeneration) {
        return *m_event;
      }
    }
    if (!m_beamParams.isValid()) {
      B2FATAL("Cannot generate beam without valid BeamParameters");
    }
    if (m_beamParams.hasChanged()) {
      m_generateHER.reset();
      m_generateLER.reset();
      m_generateVertex.reset();
    }
    m_event->setGenerationFlags(m_beamParams->getGenerationFlags() & m_allowedFlags);
    TLorentzVector her = generateBeam(m_beamParams->getHER(), m_beamParams->getCovHER(), m_generateHER);
    TLorentzVector ler = generateBeam(m_beamParams->getLER(), m_beamParams->getCovLER(), m_generateLER);
    TVector3 vtx = generateVertex(m_beamParams->getVertex(), m_beamParams->getCovVertex(), m_generateVertex);
    m_event->set(her, ler, vtx);
    //Check if we want to go to CMS, if so boost both
    if (m_beamParams->hasGenerationFlags(BeamParameters::c_generateCMS)) {
      m_event->setGenerationFlags(0);
      her = m_event->getLabToCMS() * her;
      ler = m_event->getLabToCMS() * ler;
      m_event->set(her, ler, vtx);
      m_event->setGenerationFlags(m_beamParams->getGenerationFlags() & m_allowedFlags);
    }
    return *m_event;
  }

} //Belle2 namespace
