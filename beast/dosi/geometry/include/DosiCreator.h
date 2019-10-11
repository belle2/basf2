/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef DOSICREATOR_H_
#define DOSICREATOR_H_

#include <geometry/CreatorBase.h>

class G4LogicalVolume;

namespace Belle2 {
  /** Namespace to encapsulate code needed for the DOSI detector */
  namespace dosi {

    class SensitiveDetector;

    /** The creator for the DOSI geometry. */
    class DosiCreator : public geometry::CreatorBase {
    public:
      DosiCreator();
      virtual ~DosiCreator();
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);
    protected:
      /**  SensitiveDetector DOSI */
      SensitiveDetector* m_sensitive;
    };

  }
}

#endif /* DOSICREATOR_H_ */
