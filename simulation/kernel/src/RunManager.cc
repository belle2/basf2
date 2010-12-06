/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Guofu Cao                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/kernel/RunManager.h>

using namespace Belle2;
using namespace std;

RunManager* RunManager::m_instance = NULL;

RunManager& RunManager::Instance()
{
  static SingletonDestroyer siDestroyer;
  if (!m_instance) m_instance = new RunManager();
  return *m_instance;
}


//============================================================================
//                              Private methods
//============================================================================

RunManager::RunManager() : G4RunManager()
{

}


RunManager::~RunManager()
{

}
