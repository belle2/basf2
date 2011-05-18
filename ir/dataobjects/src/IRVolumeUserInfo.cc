/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hiroyuki Nakayama                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ir/dataobjects/IRVolumeUserInfo.h>

//#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;

ClassImp(IRVolumeUserInfo)

IRVolumeUserInfo& IRVolumeUserInfo::operator=(const IRVolumeUserInfo & other)
{
  VolumeUserInfoBase::operator=(other);
  return *this;
}

