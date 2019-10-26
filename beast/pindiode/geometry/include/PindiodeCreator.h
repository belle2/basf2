/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PINDIODECREATOR_H_
#define PINDIODECREATOR_H_

#include <geometry/CreatorBase.h>

class G4LogicalVolume;

namespace Belle2 {
  /** Namespace to encapsulate code needed for the PINDIODE detector */
  namespace pindiode {

    class SensitiveDetector;

    /** The creator for the PINDIODE geometry. */
    class PindiodeCreator : public geometry::CreatorBase {
    public:
      PindiodeCreator();
      virtual ~PindiodeCreator();
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);
    protected:
      /**  SensitiveDetector PINDIODE */
      SensitiveDetector* m_sensitive;
    };

  }
}

#endif /* PINDIODECREATOR_H_ */
