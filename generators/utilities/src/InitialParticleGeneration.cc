/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <generators/utilities/InitialParticleGeneration.h>
#include <framework/logging/Logger.h>


namespace Belle2 {

  void InitialParticleGeneration::initialize()
  {
    m_event.registerInDataStore();
  }

  ROOT::Math::XYZVector InitialParticleGeneration::generateVertex(const ROOT::Math::XYZVector& initial, const TMatrixDSym& cov,
      MultivariateNormalGenerator& gen) const
  {
    if (m_event->hasGenerationFlags(BeamParameters::c_smearVertex)) {
      if (!gen.size()) gen.setMeanCov(initial, cov);
      return ROOT::Math::XYZVector(gen.generateVec3());
    }
    return initial;
  }

  ROOT::Math::PxPyPzEVector InitialParticleGeneration::generateBeam(const ROOT::Math::PxPyPzEVector& initial, const TMatrixDSym& cov,
      MultivariateNormalGenerator& gen) const
  {
    //no smearing, nothing to do
    if (!(m_event->getGenerationFlags() & BeamParameters::c_smearBeam))
      return initial;


    //Calculate initial parameters of the beam before smearing
    double E0   = initial.E();
    double thX0 = atan(initial.X() / initial.Z());
    double thY0 = atan(initial.Y() / initial.Z());

    //init random generator if there is a change in beam parameters or at the beginning
    if (gen.size() != 3) gen.setMeanCov(ROOT::Math::XYZVector(E0, thX0, thY0), cov);

    //generate the actual smeared vector (E, thX, thY)
    Eigen::VectorXd p = gen.generate();

    //if we don't smear beam energy set the E to initial value
    if (!m_event->hasGenerationFlags(BeamParameters::c_smearBeamEnergy))
      p[0] = E0;

    //if we don't smear beam direction set thX and thY to initial values
    if (!m_event->hasGenerationFlags(BeamParameters::c_smearBeamDirection)) {
      p[1] = thX0;
      p[2] = thY0;
    }

    //store smeared values
    double E   = p[0];
    double thX = p[1];
    double thY = p[2];

    //Convert values back to 4-vector
    bool isHER = initial.Z() > 0;
    return BeamParameters::getFourVector(E, thX, thY, isHER);
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
    ROOT::Math::XYZVector vtx = generateVertex(m_beamParams->getVertex(), m_beamParams->getCovVertex(), m_generateVertex);
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

  ROOT::Math::XYZVector InitialParticleGeneration::updateVertex(bool force)
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
