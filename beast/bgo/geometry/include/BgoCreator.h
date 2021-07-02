/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
      /**
       * Constructor
       */
      BgoCreator();
      /**
       * Destructor
       */
      virtual ~BgoCreator();
      /**
       * Creation of the detector geometry from Gearbox (XML).
       * @param[in] content   XML data directory.
       * @param[in] topVolume Geant world volume.
       * @param[in] type      Geometry type.
       */
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);
    protected:
      /**  SensitiveDetector BGO */
      SensitiveDetector* m_sensitive;
    };

  }
}

#endif /* BGOCREATOR_H_ */
