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
  //const 1.5 Tesla magnetic field in z direction. Hard coded. Sorry.
  return TVector3(0.0, 0.0, 1.5);
}



//====================================================================
//                          Private methods
//====================================================================

BFieldMap::BFieldMap()
{

}

BFieldMap::~BFieldMap()
{

}
