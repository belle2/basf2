/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Thomas Kuhr                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/core/Environment.h>
#include <framework/core/ModuleManager.h>

using namespace Belle2;
using namespace std;

Environment* Environment::m_instance = NULL;

Environment& Environment::Instance()
{
  static SingletonDestroyer siDestroyer;
  if (!m_instance) m_instance = new Environment();
  return *m_instance;
}


const list<string>& Environment::getModuleSearchPaths() const
{
  return ModuleManager::Instance().getModuleSearchPaths();
}


//============================================================================
//                              Private methods
//============================================================================

Environment::Environment() : m_dataSearchPath(""), m_numberProcesses(0), m_steering("")
{

}


Environment::~Environment()
{
}
