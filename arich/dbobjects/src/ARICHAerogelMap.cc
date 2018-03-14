/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Manca Mrvar (2017)                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arich/dbobjects/ARICHAerogelMap.h>

using namespace Belle2;
using namespace std;



void ARICHAerogelMap::setAerogelLayer(unsigned int i, unsigned int layer)
{
  if (i < c_layers) m_layer[i] = layer;
}

