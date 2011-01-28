/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/dataobjects/VolumeUserInfoBase.h>

using namespace std;
using namespace Belle2;

ClassImp(VolumeUserInfoBase)

VolumeUserInfoBase& VolumeUserInfoBase::operator=(const VolumeUserInfoBase & other)
{
  return *this;
}
