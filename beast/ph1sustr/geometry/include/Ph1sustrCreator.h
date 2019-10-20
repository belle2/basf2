/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PH1SUSTRCREATOR_H_
#define PH1SUSTRCREATOR_H_

#include <geometry/CreatorBase.h>

class G4LogicalVolume;

namespace Belle2 {
  /** Namespace to encapsulate code needed for the PH1SUSTR detector */
  namespace ph1sustr {

    class SensitiveDetector;

    /** The creator for the PH1SUSTR geometry. */
    class Ph1sustrCreator : public geometry::CreatorBase {
    public:
      Ph1sustrCreator();
      virtual ~Ph1sustrCreator();
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);
    protected:
      /**  SensitiveDetector phase 1 support structure */
      SensitiveDetector* m_sensitive;
    };

  }
}

#endif /* PH1SUSTRCREATOR_H_ */
