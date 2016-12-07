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

#include <boost/foreach.hpp>

using namespace std;
using namespace Belle2;

BFieldMap& BFieldMap::Instance()
{
  static std::unique_ptr<BFieldMap> instance(new BFieldMap());
  return *instance;
}

const B2Vector3D BFieldMap::getBField(const B2Vector3D& point)
{
  B2Vector3D magFieldVec(0.0, 0.0, 0.0);

  //Check that the point makes sense
  if (std::isnan(point.X()) || std::isnan(point.Y()) || std::isnan(point.Z())) {
    B2ERROR("Bfield requested for a position containing NaN, returning field 0");
    return magFieldVec;
  }

  //Check if the map has been initialized yet. If not, initialize all components of the map.
  if (!m_isMapInitialized) {
    BOOST_FOREACH(BFieldComponentAbs * comp, m_components) {
      comp->initialize();
    }

    m_isMapInitialized = true;
    B2DEBUG(10, "The magnetic field map has been initialized.");
  }

  //Loop over all magnetic field components and add their magnetic field vectors
  BOOST_FOREACH(BFieldComponentAbs * comp, m_components) {
    magFieldVec += comp->calculate(point);
  }
  return magFieldVec;
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
    BOOST_FOREACH(BFieldComponentAbs * comp, m_components) {
      comp->terminate();
      delete comp;
    }
  }
}
