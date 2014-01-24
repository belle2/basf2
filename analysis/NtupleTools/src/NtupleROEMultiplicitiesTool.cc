/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Anze Zupanc                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleROEMultiplicitiesTool.h>

#include <analysis/dataobjects/RestOfEvent.h>

#include <TBranch.h>

void NtupleROEMultiplicitiesTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();

  m_nTracks     = new int[nDecayProducts];
  m_nECLShowers = new int[nDecayProducts];
  m_nECLGammas  = new int[nDecayProducts];
  m_nECLPi0s    = new int[nDecayProducts];

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_nROETracks").c_str(),  &m_nTracks[iProduct], (strNames[iProduct] + "_nROETracks/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_nROEShowers").c_str(), &m_nECLShowers[iProduct], (strNames[iProduct] + "_nROEShowers/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_nROEGammas").c_str(),  &m_nECLGammas[iProduct], (strNames[iProduct] + "_nROEGammas/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_nROEPi0s").c_str(),    &m_nECLPi0s[iProduct], (strNames[iProduct] + "_nROEPi0s/I").c_str());
  }
}

void NtupleROEMultiplicitiesTool::eval(const Particle* particle)
{
  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);

  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_nTracks[iProduct]     = -1;
    m_nECLShowers[iProduct] = -1;
    m_nECLGammas[iProduct]  = -1;
    m_nECLPi0s[iProduct]    = -1;

    const RestOfEvent* roe = DataStore::getRelated<RestOfEvent>(selparticles[iProduct]);

    if (roe) {
      m_nTracks[iProduct]      = roe->getNTracks();
      m_nECLShowers[iProduct]  = roe->getNECLShowers();
      m_nECLGammas[iProduct]   = roe->getNECLGammas();
      m_nECLPi0s[iProduct]     = roe->getNECLPi0s();
    }
  }
}
