/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MICROTPCCREATOR_H_
#define MICROTPCCREATOR_H_

#include <geometry/CreatorBase.h>

class G4LogicalVolume;

namespace Belle2 {
  /** Namespace to encapsulate code needed for the MICROTPC detector */
  namespace microtpc {

    class SensitiveDetector;

    /** The creator for the MICROTPC geometry. */
    class MicrotpcCreator : public geometry::CreatorBase {
    public:
      /**
       * Constructor
       */
      MicrotpcCreator();
      /**
       * Destructor
       */
      virtual ~MicrotpcCreator();
      /**
       * Creation of the detector geometry from Gearbox (XML).
       * @param[in] content   XML data directory.
       * @param[in] topVolume Geant world volume.
       * @param[in] type      Geometry type.
       */
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);
    protected:
      /**  SensitiveDetector micro-tpc */
      SensitiveDetector* m_sensitive;
    };

  }
}

#endif /* MICROTPCCREATOR_H_ */
