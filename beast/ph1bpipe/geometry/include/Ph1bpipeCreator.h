/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PH1BPIPECREATOR_H_
#define PH1BPIPECREATOR_H_

#include <geometry/CreatorBase.h>

class G4LogicalVolume;

namespace Belle2 {
  /** Namespace to encapsulate code needed for the PH1BPIPE detector */
  namespace ph1bpipe {

    class SensitiveDetector;

    /** The creator for the PH1BPIPE geometry. */
    class Ph1bpipeCreator : public geometry::CreatorBase {
    public:
      Ph1bpipeCreator();
      virtual ~Ph1bpipeCreator();
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);
    protected:
      /**  SensitiveDetector phase 1 beam pipe */
      SensitiveDetector* m_sensitive;
    };

  }
}

#endif /* PH1BPIPECREATOR_H_ */
