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
      PlumeCreator();
      virtual ~PlumeCreator();
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);
    protected:
      /**  SensitiveDetector PLUME */
      SensitiveDetector* m_sensitive;
      G4AssemblyVolume* buildSupport1();
      G4AssemblyVolume* buildSupport2();
      G4AssemblyVolume* buildSupport3();
      G4AssemblyVolume* buildSupport4();
    };

  }
}

#endif /* PLUMECREATOR_H_ */
