/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SDDCREATOR_H_
#define SDDCREATOR_H_

#include <geometry/CreatorBase.h>

class G4LogicalVolume;

namespace Belle2 {
  /** Namespace to encapsulate code needed for the SDD detector */
  namespace srsensor {

    class SensitiveDetector;

    /** The creator for the SDD geometry. */
    class SddCreator : public geometry::CreatorBase {
    public:
      SddCreator();
      virtual ~SddCreator();
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);
    protected:
      /**  SensitiveDetector SDD */
      SensitiveDetector* m_sensitive;
    };

  }
}

#endif /* SDDCREATOR_H_ */
