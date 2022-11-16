/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <generators/utilities/InitialParticleGeneration.h>
#include <framework/logging/Logger.h>
#include <generators/utilities/beamHelpers.h>
#include <cmath>
#include <Eigen/Dense>


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


  TMatrixDSym InitialParticleGeneration::adjustCovMatrix(TMatrixDSym cov) const
  {
    // no smearing
    if (!m_beamParams->hasGenerationFlags(BeamParameters::c_smearBeam))
      return TMatrixDSym(3);

    // don't smear energy
    if (!m_beamParams->hasGenerationFlags(BeamParameters::c_smearBeamEnergy)) {
      for (int i = 0; i < 3; ++i)
        cov(i, 0) = cov(0, i) = 0;
    }

    // don't smear angles
    if (!m_beamParams->hasGenerationFlags(BeamParameters::c_smearBeamDirection)) {
      for (int i = 1; i < 3; ++i)
        for (int j = 1; j < 3; ++j)
          cov(i, j) = cov(j, i) = 0;
    }

    return cov;

  }


  /** transform matrix from ROOT to Eigen format */
  static Eigen::MatrixXd toEigen(TMatrixDSym m)
  {
    Eigen::MatrixXd mEig(m.GetNrows(), m.GetNcols());
    for (int i = 0; i < m.GetNrows(); ++i)
      for (int j = 0; j < m.GetNcols(); ++j)
        mEig(i, j) = m(i, j);

    return mEig;
  }


  // init random number generator which generates random boost based on the beam parameters
  ConditionalGaussGenerator InitialParticleGeneration::initConditionalGenerator(const ROOT::Math::PxPyPzEVector& pHER,
      const ROOT::Math::PxPyPzEVector& pLER, const TMatrixDSym& covHER, const TMatrixDSym& covLER)
  {
    //Calculate initial parameters of the HER beam before smearing
    double E0her   = pHER.E();
    double thX0her = std::atan(pHER.Px() / pHER.Pz());
    double thY0her = std::atan(pHER.Py() / pHER.Pz());

    //Calculate initial parameters of the LER beam before smearing
    double E0ler   = pLER.E();
    double thX0ler = std::atan(pLER.Px() / pLER.Pz());
    double thY0ler = std::atan(pLER.Py() / pLER.Pz());

    // calculate gradient of transformation (EH, thXH, thYH, EL, thXL, thYL) ->  (Ecms, bX, bY, bZ, angleCmsX, angleCmsY)
    Eigen::MatrixXd grad = getGradientMatrix(E0her, thX0her, thY0her, E0ler, thX0ler, thY0ler);
    Eigen::VectorXd mu   = getCentralValues(E0her, thX0her, thY0her, E0ler, thX0ler, thY0ler);

    // calculate covariance smearing matrix in the (Ecms, bX, bY, bZ, angleCmsX, angleCmsY) coordinate system
    Eigen::MatrixXd covT = transformCov(toEigen(adjustCovMatrix(covHER)), toEigen(adjustCovMatrix(covLER)), grad);

    // return random number generator which allows to generate Lorentz transformation parameters based on the cmsEnergy
    return ConditionalGaussGenerator(mu, covT);

  }


  ROOT::Math::PxPyPzEVector InitialParticleGeneration::generateBeam(const ROOT::Math::PxPyPzEVector& initial, const TMatrixDSym& cov,
      MultivariateNormalGenerator& gen) const
  {
    //no smearing, nothing to do
    if (!(m_event->getGenerationFlags() & BeamParameters::c_smearBeam))
      return initial;


    //Calculate initial parameters of the beam before smearing
    double E0   = initial.E();
    double thX0 = std::atan(initial.Px() / initial.Pz());
    double thY0 = std::atan(initial.Py() / initial.Pz());

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


  // it generates random Lorentz transformation based on the HER/LER smearing covariance matrices
  ROOT::Math::XYZVector InitialParticleGeneration::getVertexConditional()
  {
    if (!m_event) {
      m_event.create();
    }
    if (!m_beamParams.isValid()) {
      B2FATAL("Cannot generate beam without valid BeamParameters");
    }
    if (m_beamParams.hasChanged()) {
      m_generateLorentzTransformation = initConditionalGenerator(m_beamParams->getHER(),  m_beamParams->getLER(),
                                                                 m_beamParams->getCovHER(), m_beamParams->getCovLER());
      m_generateVertex.reset();
    }
    m_event->setGenerationFlags(m_beamParams->getGenerationFlags() & m_allowedFlags);
    ROOT::Math::XYZVector vtx = generateVertex(m_beamParams->getVertex(), m_beamParams->getCovVertex(), m_generateVertex);

    //TODO is m_event of type MCInitialParticles important?
    //Eigen::VectorXd collSys = m_generateLorentzTransformation.generate(Ecms);
    //m_event->set(her, ler, vtx);
    //m_event->setByLorentzTransformation(collSys[0], collSys[1], collSys[2], collSys[3], collSys[4], collSys[5], vtx);

    return vtx;
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
