/*************************************************************************
*  BASF2 (Belle Analysis Framework 2)                                    *
*  Copyright(C) 2010 - Belle II Collaboration                            *
*                                                                        *
*  Author: The Belle II Collaboration                                    *
*  Contributors: Kirill Chilikin                                         *
*                                                                        *
*  This software is provided "as is" without any warranty.               *
* ***********************************************************************/

#include <eklm/geoeklm/G4PVPlacementGT.h>

namespace Belle2 {

  class G4PVPlacementGT;

  G4PVPlacementGT::G4PVPlacementGT(const G4Transform3D &globalTransform,
                                   const G4Transform3D &Transform3D,
                                   G4LogicalVolume *pCurrentLogical,
                                   const G4String &pName,
                                   G4LogicalVolume *pMotherLogical,
                                   int id, int mode) :
      G4PVPlacement(Transform3D, pCurrentLogical, pName, pMotherLogical, false,
                    1, false)
  {
    m_transform = globalTransform;
    m_id = id;
    m_mode = mode;
    m_mother = NULL;
  }

  G4PVPlacementGT::G4PVPlacementGT(G4PVPlacementGT *motherPVPlacementGT,
                                   const G4Transform3D &Transform3D,
                                   G4LogicalVolume *pCurrentLogical,
                                   const G4String &pName,
                                   int id, int mode) :
      G4PVPlacement(Transform3D, pCurrentLogical, pName,
                    motherPVPlacementGT->GetLogicalVolume(), false, 1, false)
  {
    m_transform = motherPVPlacementGT->getTransform() * Transform3D;
    m_id = id;
    m_mode = mode;
    m_mother = motherPVPlacementGT;
  }

  G4Transform3D G4PVPlacementGT::getTransform()
  {
    return m_transform;
  }

  int G4PVPlacementGT::getID()
  {
    return m_id;
  }

  int G4PVPlacementGT::getMode()
  {
    return m_mode;
  }

  G4PVPlacementGT* G4PVPlacementGT::getMother()
  {
    return m_mother;
  }

}

