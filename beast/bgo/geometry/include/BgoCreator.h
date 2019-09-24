/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BGOCREATOR_H_
#define BGOCREATOR_H_

#include <geometry/CreatorBase.h>

class G4LogicalVolume;

namespace Belle2 {
  /** Namespace to encapsulate code needed for the BGO detector */
  namespace bgo {

    class SensitiveDetector;

    /** The creator for the BGO geometry. */
    class BgoCreator : public geometry::CreatorBase {
    public:
      BgoCreator();
      virtual ~BgoCreator();
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);
    protected:
      /**  SensitiveDetector BGO */
      SensitiveDetector* m_sensitive;
    };

  }
}

#endif /* BGOCREATOR_H_ */
