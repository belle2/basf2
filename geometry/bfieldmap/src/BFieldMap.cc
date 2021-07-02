/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
