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

#include <boost/foreach.hpp>

using namespace std;
using namespace Belle2;

BFieldMap* BFieldMap::m_instance = NULL;


BFieldMap& BFieldMap::Instance()
{
  static SingletonDestroyer siDestroyer;
  if (!m_instance) {
    m_instance = new BFieldMap();
  }
  return *m_instance;
}


const TVector3 BFieldMap::getBField(const TVector3& point) const
{
  TVector3 magFieldVec(0.0, 0.0, 0.0);

  //Loop over all magnetic field components and add their magnetic field vectors
  BOOST_FOREACH(BFieldComponentAbs* comp, m_components) {
    magFieldVec += comp->calculate(point);
  }
  return magFieldVec;
}



//====================================================================
//                          Private methods
//====================================================================

BFieldMap::BFieldMap()
{

}

BFieldMap::~BFieldMap()
{
  //Delete the magnetic field components by calling their terminate() method and freeing their memory.
  BOOST_FOREACH(BFieldComponentAbs* comp, m_components) {
    comp->terminate();
    delete comp;
  }
}
