/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marcel Hohmann                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/CurlTagger/Bundle.h>

using namespace Belle2;
using namespace CurlTagger;

Bundle::Bundle(bool isTruthBundle)
{
  m_IsTruthBundle = isTruthBundle;
  m_Gamma = 5; //From BN1079 - TODO check this is the best selection

  if (m_IsTruthBundle) {
    m_CurlLabel = "isCurl";
    m_SizeLabel = "bundleSize";
  } else {
    m_CurlLabel = "isTruthCurl";
    m_SizeLabel = "truthBundleSize";
  }
}

Bundle::~Bundle() {}

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

float Bundle::trackDist(Particle* particle)
{
  return TMath::Power(m_Gamma * Variable::particleDRho(particle), 2) + TMath::Power(Variable::particleDZ(particle), 2);
}

bool Bundle::compareParticles(Particle* iPart, Particle* jPart)
{
  return trackDist(iPart) < trackDist(jPart);
}

void Bundle::tagCurlInfo()
{
  //std::sort (m_Particles.begin(), m_Particles.end(), compareParticles);
  //somewhat slow but should only be comparing 2-3 particles so shouldnt be a problem.
  unsigned int bundleSize = size();
  float lowestVal  = 1e10;
  unsigned int posLowestVal = 0;
  for (unsigned int i = 0; i < bundleSize; i++) {
    if (trackDist(m_Particles[i]) < lowestVal) {
      lowestVal = trackDist(m_Particles[i]);
      posLowestVal = i;
    }
  }

  for (unsigned int i = 0; i < bundleSize; i++) {
    Particle* particle = m_Particles[i];
    std::cout << trackDist(particle) << std::endl;
    if (i == posLowestVal) {continue;}
    particle ->setExtraInfo(m_SizeLabel, 1);
  }
}

void Bundle::tagSizeInfo()
{
  unsigned int bundleSize = size();
  for (Particle* particle : m_Particles) {
    particle -> setExtraInfo(m_SizeLabel, bundleSize);
  }
}

