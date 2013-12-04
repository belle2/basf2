/**************************************************************************
 * Belle II detector background library                                   *
 * Copyright(C) 2011  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/gfbfield/GFGeant4Field.h>

using namespace std;
using namespace Belle2;

TVector3 GFGeant4Field::get(const TVector3& pos) const
{

  //Convert Tesla to kGauss
  return 10.0 * m_bField.getBField(pos);
}

