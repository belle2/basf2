/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <arich/dbobjects/ARICHAerogelMap.h>

using namespace Belle2;
using namespace std;



void ARICHAerogelMap::setAerogelLayer(unsigned int i, unsigned int layer)
{
  if (i < c_layers) m_layer[i] = layer;
}

