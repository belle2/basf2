/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle, Alexandre Beaulieu           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CSICREATOR_H_
#define CSICREATOR_H_

#include <geometry/CreatorBase.h>
#include <G4AssemblyVolume.hh>
#include <G4Transform3D.hh>
#include <string>

class G4LogicalVolume;

namespace Belle2 {
  /** Namespace to encapsulate code needed for the CSI detector */
  namespace csi {

    class SensitiveDetector;

    /** The creator for the CSI geometry. */
    class CsiCreator : public geometry::CreatorBase {
    public:
      CsiCreator();
      virtual ~CsiCreator();
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);

    protected:
      SensitiveDetector* m_sensitive;  /**<  SensitiveDetector CSI */

      /** Builds the crystals enclosures **/
      void BuildEnclosure(const GearDir& content, G4AssemblyVolume* assembly, std::string side, int iEnclosure);

      /** Builds the crystals and their wrapping (foil) **/
      void PutCrystal(const GearDir& content,  G4AssemblyVolume* assembly, G4Transform3D position,  int iEnclosure, int iCry);
    };
  }
}

#endif /* CSICREATOR_H_ */
