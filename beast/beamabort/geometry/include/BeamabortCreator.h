/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BEAMABORTCREATOR_H_
#define BEAMABORTCREATOR_H_

#include <geometry/CreatorBase.h>

class G4LogicalVolume;

namespace Belle2 {
  /** Namespace to encapsulate code needed for the BEAMABORT detector */
  namespace beamabort {

    class SensitiveDetector;

    /** The creator for the BEAMABORT geometry. */
    class BeamabortCreator : public geometry::CreatorBase {
    public:
      BeamabortCreator();
      virtual ~BeamabortCreator();
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);
    protected:
      /**  SensitiveDetector BEAMABORT */
      SensitiveDetector* m_sensitive;
    };

  }
}

#endif /* BEAMABORTCREATOR_H_ */
