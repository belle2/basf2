/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef QCSMONITORCREATOR_H_
#define QCSMONITORCREATOR_H_

#include <geometry/CreatorBase.h>

class G4LogicalVolume;

namespace Belle2 {
  /** Namespace to encapsulate code needed for the QCSMONITOR detector */
  namespace qcsmonitor {

    class SensitiveDetector;

    /** The creator for the QCSMONITOR geometry. */
    class QcsmonitorCreator : public geometry::CreatorBase {
    public:
      /**
       * Constructor
       */
      QcsmonitorCreator();
      /**
       * Destructor
       */
      virtual ~QcsmonitorCreator();
      /**
       * Creation of the detector geometry from Gearbox (XML).
       * @param[in] content   XML data directory.
       * @param[in] topVolume Geant world volume.
       * @param[in] type      Geometry type.
       */
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);
    protected:
      /**  SensitiveDetector QCSMONITOR */
      SensitiveDetector* m_sensitive;
    };

  }
}

#endif /* QCSMONITORCREATOR_H_ */
