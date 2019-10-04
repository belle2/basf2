/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HE3TUBECREATOR_H_
#define HE3TUBECREATOR_H_

#include <geometry/CreatorBase.h>

class G4LogicalVolume;

namespace Belle2 {
  /** Namespace to encapsulate code needed for the HE3TUBE detector */
  namespace he3tube {

    class SensitiveDetector;

    /** The creator for the HE3TUBE geometry. */
    class He3tubeCreator : public geometry::CreatorBase {
    public:
      He3tubeCreator();
      virtual ~He3tubeCreator();
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);
    protected:
      /**  SensitiveDetector He-3 tube */
      SensitiveDetector* m_sensitive;
    };

  }
}

#endif /* HE3TUBECREATOR_H_ */
