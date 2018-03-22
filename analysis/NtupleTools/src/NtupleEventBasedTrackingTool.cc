/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sam Cunlffe, Martin Heck                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/NtupleTools/NtupleEventBasedTrackingTool.h>
#include <analysis/VariableManager/TrackVariables.h>
#include <TBranch.h>

using namespace Belle2;

void NtupleEventBasedTrackingTool::setupTree()
{
  m_tree->Branch("nExtraCDCHits", &m_nExtraCDCHits, "nExtraCDCHits/I");
  m_tree->Branch("nExtraCDCHitsPostCleaning", &m_nExtraCDCHitsPostCleaning, "nExtraCDCHitsPostCleaning/I");
  m_tree->Branch("nExtraCDCSegments", &m_nExtraCDCSegments, "nExtraCDCSegments/I");
  m_tree->Branch("nExtraVXDHits", &m_nExtraVXDHits, "nExtraVXDHits/I");
  m_tree->Branch("svdFirstSampleTime", &m_svdFirstSampleTime, "svdFirstSampleTime/D");
  m_tree->Branch("trackFindingFailureFlag", &m_trackFindingFailureFlag, "trackFindingFailureFlag/B");
}

void NtupleEventBasedTrackingTool::eval(const Particle* p)
{
  m_nExtraCDCHits = int(Variable::nExtraCDCHits(p));
  m_nExtraCDCHitsPostCleaning = int(Variable::nExtraCDCHitsPostCleaning(p));
  m_nExtraCDCSegments = int(Variable::nExtraCDCSegments(p));
  m_nExtraVXDHits = int(Variable::nExtraVXDHits(p));
  m_svdFirstSampleTime = Variable::svdFirstSampleTime(p);
  m_trackFindingFailureFlag = bool(Variable::trackFindingFailureFlag(p));
}
