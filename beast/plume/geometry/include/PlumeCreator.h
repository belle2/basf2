/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PLUMECREATOR_H_
#define PLUMECREATOR_H_

#include <geometry/CreatorBase.h>

class G4LogicalVolume;
class G4AssemblyVolume;

namespace Belle2 {
  /** Namespace to encapsulate code needed for the PLUME detector */
  namespace plume {

    class SensitiveDetector;

    /** The creator for the PLUME geometry. */
    class PlumeCreator : public geometry::CreatorBase {
    public:
      /**
       * Constructor
       */
      PlumeCreator();
      /**
       * Destructor
       */
      virtual ~PlumeCreator();
      /**
       * Creation of the detector geometry from Gearbox (XML).
       * @param[in] content   XML data directory.
       * @param[in] topVolume Geant world volume.
       * @param[in] type      Geometry type.
       */
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);
    protected:
      /**  SensitiveDetector PLUME */
      SensitiveDetector* m_sensitive;
      /**  Volume of support1 */
      G4AssemblyVolume* buildSupport1();
      /**  Volume of support2 */
      G4AssemblyVolume* buildSupport2();
      /**  Volume of support3 */
      G4AssemblyVolume* buildSupport3();
      /**  Volume of support4 */
      G4AssemblyVolume* buildSupport4();
    };

  }
}

#endif /* PLUMECREATOR_H_ */
