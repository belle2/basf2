/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CLAWCREATOR_H_
#define CLAWCREATOR_H_

#include <geometry/CreatorBase.h>

class G4LogicalVolume;

namespace Belle2 {
  /** Namespace to encapsulate code needed for the CLAW detector */
  namespace claw {

    class SensitiveDetector;

    /** The creator for the CLAW geometry. */
    class ClawCreator : public geometry::CreatorBase {
    public:
      /**
       * Constructor
       */
      ClawCreator();
      /**
       * Destructor
       */
      virtual ~ClawCreator();
      /**
       * Creation of the detector geometry from Gearbox (XML).
       * @param[in] content   XML data directory.
       * @param[in] topVolume Geant world volume.
       * @param[in] type      Geometry type.
       */
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);
    protected:
      /**  SensitiveDetector CLAW */
      SensitiveDetector* m_sensitive;
    };

  }
}

#endif /* CLAWCREATOR_H_ */
