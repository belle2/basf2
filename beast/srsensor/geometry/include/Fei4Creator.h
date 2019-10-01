/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FEI4CREATOR_H_
#define FEI4CREATOR_H_

#include <geometry/CreatorBase.h>

class G4LogicalVolume;

namespace Belle2 {
  /** Namespace to encapsulate code needed for the FEI4 detector */
  namespace srsensor {

    class SensitiveDetector;

    /** The creator for the FEI4 geometry. */
    class Fei4Creator : public geometry::CreatorBase {
    public:
      Fei4Creator();
      virtual ~Fei4Creator();
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);
    protected:
      /**  SensitiveDetector FEI4 */
      SensitiveDetector* m_sensitive;
    };

  }
}

#endif /* FEI4CREATOR_H_ */
