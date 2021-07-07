/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
      /**
       * Constructor
       */
      CsiCreator();
      /**
       * Destructor
       */
      virtual ~CsiCreator();
      /**
       * Creation of the detector geometry from Gearbox (XML).
       * @param[in] content   XML data directory.
       * @param[in] topVolume Geant world volume.
       * @param[in] type      Geometry type.
       */
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
