/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CAVECREATOR_H_
#define CAVECREATOR_H_

#include <geometry/CreatorBase.h>

class G4LogicalVolume;

namespace Belle2 {
  /** Namespace to encapsulate code needed for the CAVE detector */
  namespace cave {

    class SensitiveDetector;

    /** The creator for the CAVE geometry. */
    class CaveCreator : public geometry::CreatorBase {
    public:
      CaveCreator();
      virtual ~CaveCreator();
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);
    protected:
      /**  SensitiveDetector cave */
      SensitiveDetector* m_sensitive;
    };

  }
}

#endif /* CAVECREATOR_H_ */
