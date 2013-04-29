/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald                                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleEventMetaDataTool.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <TBranch.h>

void NtupleEventMetaDataTool::setupTree()
{
  m_iExperiment = -1;
  m_iRun = -1;
  m_iEvent = -1;
  m_tree->Branch("exp_no", &m_iExperiment, "exp_no/I");
  m_tree->Branch("run_no", &m_iRun, "run_no/I");
  m_tree->Branch("evt_no", &m_iEvent, "evt_no/I");
}

void NtupleEventMetaDataTool::eval(const  Particle*)
{
  StoreObjPtr<EventMetaData> eventmetadata;
  if (eventmetadata) {
    m_iExperiment = eventmetadata->getExperiment();
    m_iRun = eventmetadata->getRun();
    m_iEvent = eventmetadata->getEvent();
  } else {
    m_iExperiment = -1;
    m_iRun = -1;
    m_iEvent = -1;
  }
}
