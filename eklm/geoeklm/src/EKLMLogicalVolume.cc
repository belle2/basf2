/*************************************************************************
*  BASF2 (Belle Analysis Framework 2)                                    *
*  Copyright(C) 2010 - Belle II Collaboration                            *
*                                                                        *
*  Author: The Belle II Collaboration                                    *
*  Contributors: Kirill Chilikin                                         *
*                                                                        *
*  This software is provided "as is" without any warranty.               *
* ***********************************************************************/

/* Belle2 headers. */
#include <eklm/geoeklm/EKLMLogicalVolume.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

EKLMLogicalVolume::
EKLMLogicalVolume(EKLMLogicalVolume* mlv,
                  G4VSolid* pSolid,
                  G4Material* pMaterial,
                  const G4String& name,
                  int id,
                  G4VSensitiveDetector* pSDetector,
                  enum EKLMDetectorMode mode,
                  enum EKLMSensitiveType type) :
  G4LogicalVolume(pSolid, pMaterial, name, NULL, pSDetector, NULL, true)
{
  m_mother = mlv;
  m_id = id;
  m_mode = mode;
  m_type = type;
}

int EKLMLogicalVolume::getID() const
{
  return m_id;
}

EKLMLogicalVolume* EKLMLogicalVolume::getMother() const
{
  return m_mother;
}

enum EKLMDetectorMode EKLMLogicalVolume::getMode() const
{
  return m_mode;
}

enum EKLMSensitiveType EKLMLogicalVolume::getVolumeType() const
{
  return m_type;
}

