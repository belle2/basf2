/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sam Cunlffe, Alon Hershenhorn                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/NtupleTools/NtupleEventBasedClusteringTool.h>
#include <analysis/VariableManager/ECLVariables.h>
#include <framework/logging/Logger.h>
#include <TBranch.h>

using namespace Belle2;

void NtupleEventBasedClusteringTool::setupTree()
{
  m_tree->Branch("nECLOutOfTimeCrystals", &m_nECLOutOfTimeCrystals, "nECLOutOfTimeCrystals/I");
  m_tree->Branch("nECLOutOfTimeCrystalsFWDEndcap", &m_nECLOutOfTimeCrystalsFWD,
                 "nECLOutOfTimeCrystalsFWDEndcap/I");
  m_tree->Branch("nECLOutOfTimeCrystalsBarrel", &m_nECLOutOfTimeCrystalsBarrel, "nECLOutOfTimeCrystalsBarrel/I");
  m_tree->Branch("nECLOutOfTimeCrystalsBWDEndcap", &m_nECLOutOfTimeCrystalsBWD,
                 "nECLOutOfTimeCrystalsBWDEndcap/I");
  m_tree->Branch("nRejectedECLShowers", &m_nRejectedECLShowers, "nRejectedECLShowers/I");
  m_tree->Branch("nRejectedECLShowersFWDEndcap", &m_nRejectedECLShowersFWD, "nRejectedECLShowersFWDEndcap/I");
  m_tree->Branch("nRejectedECLShowersBarrel", &m_nRejectedECLShowersBarrel, "nRejectedECLShowersBarrel/I");
  m_tree->Branch("nRejectedECLShowersBWDEndcap", &m_nRejectedECLShowersBWD, "nRejectedECLShowersBWDEndcap/I");
}

void NtupleEventBasedClusteringTool::eval(const Particle* p)
{
  // check for NAN (== no EventBasedClusteringInfo found by the VariableManager)
  double nRejectedShowers = Variable::nRejectedECLShowers(p);
  if (std::isnan(nRejectedShowers)) {
    B2WARNING("No EventLevelClusteringInfo found. This is only available in files produced with release-02 or newer.");
    m_nECLOutOfTimeCrystals = -1;
    m_nECLOutOfTimeCrystalsFWD = -1;
    m_nECLOutOfTimeCrystalsBarrel = -1;
    m_nECLOutOfTimeCrystalsBWD = -1;
    m_nRejectedECLShowers = -1;
    m_nRejectedECLShowersFWD = -1;
    m_nRejectedECLShowersBarrel = -1;
    m_nRejectedECLShowersBWD = -1;
    return;
  }
  //
  m_nECLOutOfTimeCrystals = int(Variable::nECLOutOfTimeCrystals(p));
  m_nECLOutOfTimeCrystalsFWD = int(Variable::nECLOutOfTimeCrystalsFWDEndcap(p));
  m_nECLOutOfTimeCrystalsBarrel = int(Variable::nECLOutOfTimeCrystalsBarrel(p));
  m_nECLOutOfTimeCrystalsBWD = int(Variable::nECLOutOfTimeCrystalsBWDEndcap(p));
  m_nRejectedECLShowers = int(nRejectedShowers);
  m_nRejectedECLShowersFWD = int(Variable::nRejectedECLShowersFWDEndcap(p));
  m_nRejectedECLShowersBarrel = int(Variable::nRejectedECLShowersBarrel(p));
  m_nRejectedECLShowersBWD = int(Variable::nRejectedECLShowersBWDEndcap(p));
  return;
}
