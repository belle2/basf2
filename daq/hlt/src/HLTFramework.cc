/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <daq/hlt/HLTFramework.h>

using namespace Belle2;

/// @brief HLTFramework constructor
/// @param type Type of the node
HLTFramework::HLTFramework(int type)
{
  B2INFO("\x1b[33m####################################################\x1b[0m");
  B2INFO("\x1b[33m#              Starting HLTFramework               #\x1b[0m");
  B2INFO("\x1b[33m####################################################\x1b[0m");
  if (type == 1)
    m_type = c_ManagerNode;
  else if (type == 2)
    m_type = c_ProcessNode;
  else {
    B2WARNING("Unknown node type assigned!");
    m_type = c_Undefined;
  }
}

/// @brief HLTFramework destructor
HLTFramework::~HLTFramework()
{
  if (m_hltManager != NULL)
    delete m_hltManager;
  if (m_hltProcess != NULL)
    delete m_hltProcess;
}

/// @brief Initialize the HLTFramework
/// @param xmlFileName XML file name that contains information of entire HLT farm
/// @return c_Success Initialization succeeded
/// @return c_InitFailed Initialization failed
EHLTStatus HLTFramework::init(char* xmlFileName)
{
  if (m_type == c_ManagerNode) {
    m_hltManager = new HLTManager();
    m_hltProcess = NULL;

    XMLParser* xml = new XMLParser(xmlFileName);
    xml->init();
    xml->parsing();
    m_hltManager->storeNodeInfo(xml);

    m_hltManager->initSenders();

    if (!m_hltManager->isChild())
      m_hltManager->checkChildren();

    delete xml;

    return c_Success;
  } else if (m_type == c_ProcessNode) {
    m_hltManager = NULL;
    m_hltProcess = new HLTProcess();

    EHLTStatus status;

    if ((status = m_hltProcess->initControl()) != c_Success)
      return status;
    if ((status = m_hltProcess->initSenders()) != c_Success)
      return status;
    if ((status = m_hltProcess->initReceivers()) != c_Success)
      return status;
    if ((status = m_hltProcess->process()) != c_Success)
      return status;

    if (!m_hltProcess->isChild())
      m_hltProcess->checkChildren();

    return c_Success;
  } else {
    B2ERROR("No role assigned!");
    return c_InitFailed;
  }
}
