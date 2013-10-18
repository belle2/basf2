/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arpa/inet.h>
#include <tracking/pxdDataReductionClasses/ROIrawID.h>
using namespace Belle2;

ROIrawID::baseType ROIrawID::getBigEndian() const
{
  return htobe64(m_rawID.id);
}
