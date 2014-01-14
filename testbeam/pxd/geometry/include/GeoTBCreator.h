/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOTBCREATOR_H_
#define GEOTBCREATOR_H_

#include <pxd/simulation/SensitiveDetector.h>
#include <svd/simulation/SensitiveDetector.h>
#include <testbeam/vxd/simulation/SensitiveDetector.h>
#include <geometry/CreatorBase.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <string>
#include <vector>
#include <map>
#include <sstream>

#include <G4Transform3D.hh>
#include <G4LogicalVolume.hh>

class G4LogicalVolume;
class G4AssemblyVolume;

namespace Belle2 {
  /** Namespace to encapsulate code needed for simulation and reconstruction of the PXD */
  namespace PXD {

    /** The creator for the TB geometry of the Belle II detector.   */
    class GeoTBCreator : public geometry::CreatorBase {

    public:

      /** Constructor of the GeoTBCreator class. */
      GeoTBCreator();

      /** The destructor of the GeoTBCreator class. */
      virtual ~GeoTBCreator();

      /**
       * Reads the content of a GearDir node for shape/dimensions/material/name
       * of a logical volume to be created and returns its instance
       * @param content The GearDir with everything inside <Volume>...</Volume>
       * @return G4LogicalVolume* Returns new G4 volume to be placed somewhere
       **/
      G4LogicalVolume* getLogicalVolume(const GearDir& content);

      /**
       * Reads the transformation (displacement+rotation) from GearDir Volume node
       * @param content GearDir with content of <Volume>...</Volume>
       * @return G4Transform3D* Returns new transformation to be used in G4PVPlacement
       **/
      G4Transform3D* getTransform(const GearDir& content);

      /**
       * Reads content of <Active></Active>, decides on type of sensor (TB/PXD)
       * creates appropriate SensitiveDetector, fills it by info from GearDir
       * and sets passed volume as sensitive
       * @param content content of <Active></Active>
       * @param volume The G4 volume to be set as sensitive
       * @param parentContent The GearDir object with content of parent <Volume>...</Volume> node.
       * Required to pass geometry params of the volume to the SensorInfo
       * @return void
       **/
      void setVolumeActive(const GearDir& content, G4LogicalVolume* volume, const GearDir& parentContent);

      /**
       * Goes through all Volume nodes in GearDir recursively and creates the G4 geometry
       *
       * @param content GearDir object with arbitrary number of Volume nodes (tree structure possible)
       * @param parentVolume Typicaly the G4 logical topVolume from argument of create void. Or other G4 logical volume, where the
       * geometry created from params in 'content' should be placed.
       * @param level level of recursion. For internal use. Keep blank or set to 1 when calling setVolumeActive.
       * @return void
       **/
      void readAddVolumes(const GearDir& content, G4LogicalVolume* parentVolume, int level);

      /**
       * Creates the G4 Objects for the TB geometry.
       * @param content A reference to the content part of the parameter
       *                description, which should to be used to create the G4
       *                objects.
       */
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);

    protected:

      /** List of all created PXD sensitive detector instances */
      std::vector<PXD::SensitiveDetector*> m_sensitivePXD;
      /** List of all created SVD sensitive detector instances */
      std::vector<SVD::SensitiveDetector*> m_sensitiveSVD;
      /** List of all created telescope sensitive detector instances */
      std::vector<TEL::SensitiveDetector*> m_sensitiveTEL;

      /** Make also chips sensitive. */
      bool m_activeChips;
      /** Make sensitive detectors also see neutrons. */
      bool m_seeNeutrons;
      /** If true only create TrueHits from primary particles and ignore secondaries */
      bool m_onlyPrimaryTrueHits;
      /** Minimum deposited energy per step in the SensitiveDetector (discard step if less) */
      double m_sensitiveThreshold;
    };

  }
}

#endif /* GEOTBCREATOR_H_ */
