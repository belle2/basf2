/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
      /**
       * Constructor
       */
      SddCreator();
      /**
       * Destructor
       */
      virtual ~SddCreator();
      /**
       * Creation of the detector geometry from Gearbox (XML).
       * @param[in] content   XML data directory.
       * @param[in] topVolume Geant world volume.
       * @param[in] type      Geometry type.
       */
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);
    protected:
      /**  SensitiveDetector SDD */
      SensitiveDetector* m_sensitive;
    };

  }
}

#endif /* SDDCREATOR_H_ */
