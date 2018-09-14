/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald, Phillip Urquijo                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleKlongIDTool.h>
#include <analysis/VariableManager/Variables.h>
#include <analysis/VariableManager/KLMClusterVariables.h>
#include <framework/gearbox/Const.h>
#include <TBranch.h>

using namespace Belle2;
using namespace std;

void NtupleKlongIDTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();
  m_klongID   = new float[nDecayProducts];
  m_isFEKLM   = new float[nDecayProducts];
  m_isBEKLM   = new float[nDecayProducts];
  m_isBKLM    = new float[nDecayProducts];
  m_Time      = new float[nDecayProducts];
  m_innerMost = new float[nDecayProducts];
  m_NLayer    = new float[nDecayProducts];
  m_trackFlag = new float[nDecayProducts];
  m_ECLFlag   = new float[nDecayProducts];
  m_Energy    = new float[nDecayProducts];

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_klongID").c_str()      , &m_klongID[iProduct]      ,
                   (strNames[iProduct] + "_klongID/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_isFEKLM").c_str()   , &m_isFEKLM[iProduct]   , (strNames[iProduct] + "_isFEKLM/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_isBEKLM").c_str()   , &m_isBEKLM[iProduct]   , (strNames[iProduct] + "_isBEKLM/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_isBKLM").c_str()    , &m_isBKLM[iProduct]    , (strNames[iProduct] + "_isBKLM/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Time").c_str()      , &m_Time[iProduct]      , (strNames[iProduct] + "_Time/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_innerMost").c_str() , &m_innerMost[iProduct] ,
                   (strNames[iProduct] + "_innerMost/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_NLayer").c_str()    , &m_NLayer[iProduct]    , (strNames[iProduct] + "_NLayer/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_trackFlag").c_str() , &m_trackFlag[iProduct] ,
                   (strNames[iProduct] + "_trackFlag/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_ECLFlag").c_str()   , &m_ECLFlag[iProduct]   , (strNames[iProduct] + "_ECLFlag/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Energy").c_str()    , &m_Energy[iProduct]    , (strNames[iProduct] + "_Energy/F").c_str());
  }
}

void NtupleKlongIDTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtupleKlongIDTool::eval - no Particle found!");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_klongID[iProduct]      = Variable::klmClusterKlId(selparticles[iProduct]);
    m_isFEKLM[iProduct]   = Variable::klmClusterIsForwardEKLM(selparticles[iProduct]);
    m_isBEKLM[iProduct]   = Variable::klmClusterIsBackwardEKLM(selparticles[iProduct]);
    m_isBKLM[iProduct]    = Variable::klmClusterIsBKLM(selparticles[iProduct]);
    m_Time[iProduct]      = Variable::klmClusterTiming(selparticles[iProduct]);
    m_innerMost[iProduct] = Variable::klmClusterInnermostLayer(selparticles[iProduct]);
    m_NLayer[iProduct]    = Variable::klmClusterLayers(selparticles[iProduct]);
    m_trackFlag[iProduct] = Variable::klmClusterBelleTrackFlag(selparticles[iProduct]);
    m_ECLFlag[iProduct]   = Variable::klmClusterBelleECLFlag(selparticles[iProduct]);
    m_Energy[iProduct]    = Variable::klmClusterEnergy(selparticles[iProduct]);
  }
}
