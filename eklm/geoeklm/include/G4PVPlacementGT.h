/*************************************************************************
*  BASF2 (Belle Analysis Framework 2)                                    *
*  Copyright(C) 2010 - Belle II Collaboration                            *
*                                                                        *
*  Author: The Belle II Collaboration                                    *
*  Contributors: Kirill Chilikin                                         *
*                                                                        *
*  This software is provided "as is" without any warranty.               *
* ***********************************************************************/

#ifndef G4PVPLACEMENTGT_H_
#define G4PVPLACEMENTGT_H_

#include <G4PVPlacement.hh>
#include <G4LogicalVolume.hh>
#include <G4Transform3D.hh>

namespace Belle2 {

  /**
   * G4PVPlacementGT - G4PVPlacement with Global Transformation information
   */
  class G4PVPlacementGT : public G4PVPlacement {

  public:

    /**
     * Constructor.
     * This constructor is used when mother physical volume is not a
     * G4PVPlacementGT.
     */
    G4PVPlacementGT(const G4Transform3D &globalTransform,
                    const G4Transform3D &Transform3D,
                    G4LogicalVolume *pCurrentLogical,
                    const G4String &pName,
                    G4LogicalVolume *pMotherLogical);

    /**
     * Constructor.
     * This constructor is used when mother physical volume is
     * a G4PVPlacementGT.
     */
    G4PVPlacementGT(G4PVPlacementGT *motherPVPlacementGT,
                    const G4Transform3D &Transform3D,
                    G4LogicalVolume *pCurrentLogical,
                    const G4String &pName);

    /**
     * getTransform - get global transformation.
     */
    inline G4Transform3D getTransform() const
    {return m_transform;}

  private:

    /**
     * Transformation from global to local coordinates.
     */
    G4Transform3D m_transform;

  };

}

#endif

