/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <daq/hlt/HLTProcess.h>

using namespace Belle2;

/* @brief HLTProcess constructor
 * @param nodeInfo NodeInfo object
*/
HLTProcess::HLTProcess(NodeInfo* nodeInfo)
{
  m_nodeInfo = nodeInfo;
  m_signalMan = NULL;
}

/* @brief HLTProcess destructor
*/
HLTProcess::~HLTProcess()
{
  //delete m_nodeInfo;
  //delete m_signalMan;
}

/* @brief Initializing HLTProcess
 * @return c_Success Initializing success
 * @return c_InitFailed Initializing failed
*/
EStatus HLTProcess::init()
{
  if (m_signalMan != NULL) {
    B2ERROR("HLTProcess: SignalMan exists!");
    return c_InitFailed;
  } else {
    if (m_nodeInfo->targetIP().size() == 0) {
      //if (m_dest.size() == 0) {
      B2ERROR("HLTProcess: Destinations are not assigned");
      return c_InitFailed;
    } else {
      B2INFO("HLTProcess: SignalMan initialized with destination");
      m_signalMan = new SignalMan(c_DataInPort, c_DataOutPort, m_nodeInfo->targetIP()[0]);
      m_signalMan->init("B2DataIn", "B2DataOut");

      B2INFO("HLTProcess: Return c_Success");
      return c_Success;
    }
  }
}

EStatus HLTProcess::beginRun()
{
  B2INFO("HLTProcess: Starting to run basf2...");

  m_pidBasf2 = fork();

  if (m_pidBasf2 == 0) {
    system("basf2 -i");
    return c_TermCalled;
  }

  return c_Success;
}
