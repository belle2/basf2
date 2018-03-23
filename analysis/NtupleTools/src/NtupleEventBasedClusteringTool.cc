/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
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
  m_tree->Branch("nECLCrystalDepositsOutOfTime", &m_nECLCrystalDepositsOutOfTime, "nECLCrystalDepositsOutOfTime/I");
  m_tree->Branch("nECLCrystalDepositsOutOfTimeFWDEndcap", &m_nECLCrystalDepositsOutOfTimeFWD,
                 "nECLCrystalDepositsOutOfTimeFWDEndcap/I");
  m_tree->Branch("nECLCrystalDepositsOutOfTimeBarrel", &m_nECLCrystalDepositsOutOfTimeBarrel, "nECLCrystalDepositsOutOfTimeBarrel/I");
  m_tree->Branch("nECLCrystalDepositsOutOfTimeBWDEndcap", &m_nECLCrystalDepositsOutOfTimeBWD,
                 "nECLCrystalDepositsOutOfTimeBWDEndcap/I");
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
    B2WARNING("No EventBasedClustering information found. This is only available files produced with release-02 or newer");
    m_nECLCrystalDepositsOutOfTime = -1;
    m_nECLCrystalDepositsOutOfTimeFWD = -1;
    m_nECLCrystalDepositsOutOfTimeBarrel = -1;
    m_nECLCrystalDepositsOutOfTimeBWD = -1;
    m_nRejectedECLShowers = -1;
    m_nRejectedECLShowersFWD = -1;
    m_nRejectedECLShowersBarrel = -1;
    m_nRejectedECLShowersBWD = -1;
    return;
  }
  //
  m_nECLCrystalDepositsOutOfTime = int(Variable::nECLCrystalDepositsOutOfTime(p));
  m_nECLCrystalDepositsOutOfTimeFWD = int(Variable::nECLCrystalDepositsOutOfTimeFWDEndcap(p));
  m_nECLCrystalDepositsOutOfTimeBarrel = int(Variable::nECLCrystalDepositsOutOfTimeBarrel(p));
  m_nECLCrystalDepositsOutOfTimeBWD = int(Variable::nECLCrystalDepositsOutOfTimeBWDEndcap(p));
  m_nRejectedECLShowers = int(nRejectedShowers);
  m_nRejectedECLShowersFWD = int(Variable::nRejectedECLShowersFWDEndcap(p));
  m_nRejectedECLShowersBarrel = int(Variable::nRejectedECLShowersBarrel(p));
  m_nRejectedECLShowersBWD = int(Variable::nRejectedECLShowersBWDEndcap(p));
  return;
}
