/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
      /**
       * Constructor
       */
      Ph1sustrCreator();
      /**
       * Destructor
       */
      virtual ~Ph1sustrCreator();
      /**
       * Creation of the detector geometry from Gearbox (XML).
       * @param[in] content   XML data directory.
       * @param[in] topVolume Geant world volume.
       * @param[in] type      Geometry type.
       */
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);
    protected:
      /**  SensitiveDetector phase 1 support structure */
      SensitiveDetector* m_sensitive;
    };

  }
}

#endif /* PH1SUSTRCREATOR_H_ */
