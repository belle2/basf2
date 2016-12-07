/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/bfieldmap/BFieldMap.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;

BFieldMap& BFieldMap::Instance()
{
  static std::unique_ptr<BFieldMap> instance(new BFieldMap());
  return *instance;
}

void BFieldMap::initialize()
{
  if (!m_isMapInitialized) {
    for (BFieldComponentAbs* comp : m_components) {
      comp->initialize();
    }

    m_isMapInitialized = true;
    B2DEBUG(10, "The magnetic field map has been initialized.");
  }
}

void BFieldMap::clear()
{
  m_isMapInitialized = false;
  for (auto ptr : m_components) delete ptr;
  m_components.clear();
}

//====================================================================
//                          Private methods
//====================================================================

BFieldMap::BFieldMap() : m_isMapInitialized(false)
{

}

BFieldMap::~BFieldMap()
{
  if (m_isMapInitialized) {
    //Delete the magnetic field components by calling their terminate() method and freeing their memory.
    for (BFieldComponentAbs* comp : m_components) {
      comp->terminate();
      delete comp;
    }
  }
}
