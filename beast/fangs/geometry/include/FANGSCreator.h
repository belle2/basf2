/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef FANGSCREATOR_H_
#define FANGSCREATOR_H_

#include <geometry/CreatorBase.h>
#include <simulation/kernel/SensitiveDetectorBase.h>

class G4LogicalVolume;
class G4Material;
class G4UserLimits;


namespace Belle2 {
  /** Namespace to encapsulate code needed for the FANGS detector */
  namespace fangs {

    /** The creator for the FANGS geometry. */
    class FANGSCreator : public geometry::CreatorBase {
    public:
      /** Constructor
       * @param sensitive detector instance to use, if null a
       * fangs::SensitiveDetector instance will be created.
       */
      FANGSCreator(Simulation::SensitiveDetectorBase* sensitive = nullptr);
      /**
       * Destructor
       */
      virtual ~FANGSCreator();
      /** create a shape (box or cylinder) from XML description and place all
       * child shapes in it by recursively calling this function for all
       * children.
       * @param prefix prefix of the volume names
       * @param params XML parameters
       * @param parent Logical Volume where to place the shape
       * @param roffset radial offset for the placement
       * @param check if true check for overlaps when placing the volumes
       */
      void createShape(const std::string& prefix, const GearDir& params,
                       G4LogicalVolume* parent, double roffset, bool check);
      /**
       * Creation of the detector geometry from Gearbox (XML).
       * @param[in] content   XML data directory.
       * @param[in] topVolume Geant world volume.
       * @param[in] type      Geometry type.
       */
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume,
                          geometry::GeometryTypes type);
    protected:
      /** pointer to the sensitive detector implementation */
      Simulation::SensitiveDetectorBase* m_sensitive{nullptr};
      /** pointer to the material in the top volume to use as default */
      G4Material* m_topMaterial{nullptr};
      /** pointer to the G4Userlimits to set for sensitive volumes (if any) */
      G4UserLimits* m_stepLength{nullptr};
    };

  }
}

#endif /* FANGSCREATOR_H_ */
