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
                    G4LogicalVolume *pMotherLogical,
                    int id = 0);

    /**
     * Constructor.
     * This constructor is used when mother physical volume is
     * a G4PVPlacementGT.
     */
    G4PVPlacementGT(G4PVPlacementGT *motherPVPlacementGT,
                    const G4Transform3D &Transform3D,
                    G4LogicalVolume *pCurrentLogical,
                    const G4String &pName,
                    int id = 0);

    /**
     * getTransform - get transformation.
     */
    inline G4Transform3D getTransform() {return m_transform;}

    /**
     * getID - get identifier.
     */
    inline int getID() {return m_id;}

    /**
     * getMother - get mother G4PVPlacementGT;
     */
    inline G4PVPlacementGT *getMother() {return m_mother;}

  private:

    /**
     * Transformation from local to global coordinate.
     * This is the transformation of point (not coordinate system).
     */
    G4Transform3D m_transform;

    /**
     * Identifier.
     */
    int m_id;

    /**
     * Mother G4PVPlacementGT.
     */
    G4PVPlacementGT *m_mother;

  };

}

#endif

