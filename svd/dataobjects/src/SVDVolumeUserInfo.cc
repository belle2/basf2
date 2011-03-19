/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/dataobjects/SVDVolumeUserInfo.h>

#include <framework/logging/Logger.h>

#include <G4VPhysicalVolume.hh>
#include <TG4RootDetectorConstruction.h>

using namespace std;
using namespace Belle2;

ClassImp(SVDVolumeUserInfo)

SVDVolumeUserInfo& SVDVolumeUserInfo::operator=(const SVDVolumeUserInfo & other)
{
  VolumeUserInfoBase::operator=(other);
  this->m_layerID = other.getLayerID();
  this->m_ladderID = other.getLadderID();
  this->m_sensorID = other.getSensorID();
  return *this;
}

