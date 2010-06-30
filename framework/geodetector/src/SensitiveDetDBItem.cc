/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/geodetector/SensitiveDetDBItem.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;


SensitiveDetDBItem::SensitiveDetDBItem(const string& geoPathName)
{
  m_geoPathName = geoPathName;
}


SensitiveDetDBItem::~SensitiveDetDBItem()
{

}


void SensitiveDetDBItem::addSensitiveVolume(TGeoVolume* volume)
{
  if (volume != 0) {
    m_volumeList.push_back(volume);
  } else {
    ERROR("Could not add the sensitive volume to the sensitive volume database: " << m_geoPathName << " !")
  }
}
