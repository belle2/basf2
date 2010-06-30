/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/geodetector/CreatorManager.h>
#include <framework/geodetector/CreatorBase.h>
#include <framework/logging/Logger.h>

#include <TGeoManager.h>

using namespace std;
using namespace Belle2;


CreatorBase::CreatorBase(const string& name) throw(GDetExcCreatorNameEmpty)
{
  if (name.empty()) throw GDetExcCreatorNameEmpty();
  m_name = name;
  m_senVolPrefix = "SD_";
  m_hasSensitiveVolumes = false;
  m_hasGroupName = false;

  CreatorManager::Instance().registerCreator(this);
}


CreatorBase::~CreatorBase()
{

}


void CreatorBase::setDescription(const std::string& description)
{
  m_description = description;
}


void CreatorBase::activateAutoSensitiveVolumes(const std::string prefix)
{
  m_hasSensitiveVolumes = true;
  m_senVolPrefix = prefix;
}


TGeoVolumeAssembly* CreatorBase::addSubdetectorGroup(const std::string& groupName, TGeoMatrix* groupTrafo)
{
  if (m_hasGroupName) {
    ERROR("The Creator " << m_name << " has already a subdetector group name !")
    return 0;
  }

  if (groupName.empty()) {
    ERROR("Empty group name specified in creator " << m_name << " !")
    return 0;
  }

  TGeoVolume* topVolume = gGeoManager->GetTopVolume();
  TGeoVolumeAssembly* subDetGrp = new TGeoVolumeAssembly(groupName.c_str());
  topVolume->AddNode(subDetGrp, 1, groupTrafo);
  m_geoGroupName = groupName;
  return subDetGrp;
}
