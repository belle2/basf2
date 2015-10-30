/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/geometry/BFieldManager.h>

using namespace Belle2;

BFieldManager& BFieldManager::getInstance()
{
  static BFieldManager instance;
  return instance;
}

void BFieldManager::clearComponents()
{
  for (BFieldComponent* c : m_components) {
    delete c;
  }
  m_components.clear();
  m_fallbackActive = false;
}
