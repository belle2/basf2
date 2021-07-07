/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <analysis/modules/CurlTagger/Bundle.h>

#include <analysis/variables/VertexVariables.h>

using namespace Belle2;
using namespace CurlTagger;

Bundle::Bundle(bool isTruthBundle)
{
  m_IsTruthBundle = isTruthBundle;
  m_Gamma = 5; //From BN1079 - TODO check this gives best selection (what defines best?)

  if (m_IsTruthBundle) {
    m_CurlLabel = "isTruthCurl";
    m_SizeLabel = "truthBundleSize";
  } else {
    m_CurlLabel = "isCurl";
    m_SizeLabel = "bundleSize";
  }
}

Bundle::~Bundle() = default;

void Bundle::addParticle(Particle* particle)
{
  m_Particles.push_back(particle);
}

Particle* Bundle::getParticle(unsigned int i)
{
  return m_Particles[i];
}

unsigned int Bundle::size()
{
  return m_Particles.size();
}

float Bundle::scaledImpactParam(Particle* particle)
{
  return TMath::Power(m_Gamma * Variable::particleDRho(particle), 2) + TMath::Power(Variable::particleDZ(particle), 2);
}

bool Bundle::compareParticles(Particle* iPart, Particle* jPart)
{
  return scaledImpactParam(iPart) < scaledImpactParam(jPart);
}

void Bundle::tagCurlInfo()
{
  //somewhat slow but should only be comparing 2-3 particles so shouldnt be a problem.
  //std::sort (m_Particles.begin(), m_Particles.end(), compareParticles);
  unsigned int bundleSize = size();
  float lowestVal  = 1e10;
  unsigned int posLowestVal = 0;
  for (unsigned int i = 0; i < bundleSize; i++) {
    if (scaledImpactParam(m_Particles[i]) < lowestVal) {
      lowestVal = scaledImpactParam(m_Particles[i]);
      posLowestVal = i;
    }
  }

  for (unsigned int i = 0; i < bundleSize; i++) {
    Particle* particle = m_Particles[i];
    //std::cout << trackDist(particle) << std::endl;
    if (i == posLowestVal) {continue;}
    particle ->setExtraInfo(m_CurlLabel, 1);
  }
}

void Bundle::tagSizeInfo()
{
  unsigned int bundleSize = size();
  for (Particle* particle : m_Particles) {
    particle -> setExtraInfo(m_SizeLabel, bundleSize);
  }
}

