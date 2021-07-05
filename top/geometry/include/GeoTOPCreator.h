/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <top/dbobjects/TOPGeometry.h>
#include <top/geometry/TOPGeometryPar.h>
#include <geometry/CreatorBase.h>
#include <framework/gearbox/GearDir.h>
#include <framework/database/IntervalOfValidity.h>

class G4LogicalVolume;
class G4AssemblyVolume;
class G4Box;
class G4UnionSolid;

namespace Belle2 {

  class BkgSensitiveDetector;

  namespace TOP {

    class SensitivePMT;
    class SensitiveBar;

    typedef std::vector<std::pair<double, double> > Polygon;

    /**
     * Geometry creator for TOP counter.
     */
    class GeoTOPCreator : public geometry::CreatorBase {

    public:

      /**
       * Constructor
       */
      GeoTOPCreator();

      /**
       * Destructor
       */
      virtual ~GeoTOPCreator();

      /**
       * Creation of the detector geometry from Gearbox (XML).
       * @param[in] content   XML data directory.
       * @param[in] topVolume Geant world volume.
       * @param[in] type      Geometry type.
       */
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume,
                          geometry::GeometryTypes type) override;

      /**
       * Creation of the detector geometry from database.
       * @param[in] name      Name of the component in the database.
       * @param[in] topVolume Geant world volume.
       * @param[in] type      Geometry type.
       */
      virtual void createFromDB(const std::string& name, G4LogicalVolume& topVolume,
                                geometry::GeometryTypes type) override;

      /**
       * Creation of payloads.
       * @param content XML data directory.
       * @param iov     Interval of validity.
       */
      virtual void createPayloads(const GearDir& content,
                                  const IntervalOfValidity& iov) override;


    private:

      /**
       * Honeycomb panel types
       */
      enum EPanelType {c_Inner, c_Outer};

      /**
       * Side rail types
       */
      enum ESideRailType {c_Left, c_Right};

      /**
       * Create the geometry from a parameter object.
       */
      void createGeometry(const TOPGeometry& parameters,
                          G4LogicalVolume& topVolume,
                          geometry::GeometryTypes type);

      /**
       * Creates single module
       * @param geo geometry description
       * @param moduleID module ID (slot number)
       * @return logical volume
       */
      G4LogicalVolume* createModule(const TOPGeometry& geo, int moduleID);

      /**
       * Creates module envelope
       * @param geo geometry description
       * @param moduleID module ID (slot number)
       * @return logical volume
       */
      G4LogicalVolume* createModuleEnvelope(const TOPGeoQBB& geo, int moduleID);

      /**
       * Assembles QBB
       * @param geo geometry description
       * @return assembly volume
       */
      G4AssemblyVolume* assembleQBB(const TOPGeoQBB& geo);

      /**
       * Creates honeycomb panel
       * @param geo geometry description
       * @param type panel type
       * @return logical volume
       */
      G4LogicalVolume* createHoneycombPanel(const TOPGeoQBB& geo, EPanelType type);

      /**
       * Creates side rail
       * @param geo geometry description
       * @param type side rail type
       * @param translate translation to return
       * @return logical volume
       */
      G4LogicalVolume* createSideRail(const TOPGeoQBB& geo, ESideRailType type);

      /**
       * Assembles front-end electronics
       * @param geo geometry description
       * @param N number of board stacks per module
       * @return assembly volume
       */
      G4AssemblyVolume* assembleFrontEnd(const TOPGeoFrontEnd& geo, int N);

      /**
       * Creates board stack
       * @param geo geometry description
       * @param N number of board stacks per module
       * @return logical volume
       */
      G4LogicalVolume* createBoardStack(const TOPGeoFrontEnd& geo, int N);

      /**
       * Assembles optical components (PMT array, prism and bar segments) along z
       * @param geo geometry description
       * @return assembly volume
       */
      G4AssemblyVolume* assembleOptics(const TOPGeoModule& geo);

      /**
       * Creates quartz bar segment
       * @param geo geometry description
       * @param moduleID module ID (e.g. slot number)
       * @return logical volume of bar segment
       */
      G4LogicalVolume* createBarSegment(const TOPGeoBarSegment& geo, int moduleID);

      /**
       * Creates quartz prism
       * @param geo geometry description
       * @param moduleID module ID (e.g. slot number)
       * @return logical volume of prism
       */
      G4LogicalVolume* createPrism(const TOPGeoPrism& geo, int moduleID);

      /**
       * Creates quartz segment with spherical mirror
       * @param geo geometry description
       * @param moduleID module ID (e.g. slot number)
       * @return logical volume of mirror segment
       */
      G4LogicalVolume* createMirrorSegment(const TOPGeoMirrorSegment& geo, int moduleID);

      /**
       * Creates PMT array
       * @param geo geometry description
       * @param moduleID module ID (e.g. slot number)
       * @return logical volume of PMT array
       */
      G4LogicalVolume* createPMTArray(const TOPGeoPMTArray& geo, int moduleID);

      /**
       * Creates single PMT
       * @param geo geometry description
       * @return logical volume of one PMT
       */
      G4LogicalVolume* createPMT(const TOPGeoPMT& geo);

      /**
       * Creates material box
       * @param name volume name
       * @param A box x size
       * @param B box y size
       * @param C box z size
       * @param materialName material name
       * @return logical volume
       */
      G4LogicalVolume* createBox(const std::string& name,
                                 double A, double B, double C,
                                 const std::string& materialName);

      /**
       * Creates material volume that is intersection of box and half-sphere shell (z > 0)
       * @param name volume name
       * @param box box shape
       * @param Rmin minimal radius of sphere shell
       * @param Rmax maximal radius of sphere shell
       * @param xc center of a sphere in x
       * @param yc center of a sphere in y
       * @param zc center of a sphere in z
       * @param materialName material name
       * @return logical volume
       */
      G4LogicalVolume* createBoxSphereIntersection(const std::string& name,
                                                   G4Box* box,
                                                   double Rmin, double Rmax,
                                                   double xc, double yc, double zc,
                                                   const std::string& materialName);

      /**
       * Creates material extruded solid
       * @param name volume name
       * @param shape x-y shape of extruded solid
       * @param length length in z
       * @param materialName material name
       * @return logical volume
       */
      G4LogicalVolume* createExtrudedSolid(const std::string& name,
                                           const Polygon& shape,
                                           double length,
                                           const std::string& materialName);

      /**
       * Adds number to string
       * @param str string
       * @param number number to be added
       * @return string with a number
       */
      std::string addNumber(const std::string& str, unsigned number);


      SensitivePMT* m_sensitivePMT = 0;  /**< Sensitive vol. to register PMT hits */
      SensitiveBar* m_sensitiveBar = 0;  /**< Sensitive vol. to register particles */
      BkgSensitiveDetector* m_sensitivePCB1 = 0;  /**< PCB sensitive for BG studies */
      BkgSensitiveDetector* m_sensitivePCB2 = 0;  /**< PCB sensitive for BG studies */
      TOPGeometryPar* m_topgp = TOPGeometryPar::Instance(); /**< singleton class */
      int m_isBeamBkgStudy = 0; /**< flag for beam backgound simulation */

      G4UnionSolid* m_moduleEnvelope = 0; /**< module envelope solid */
      G4AssemblyVolume* m_qbb = 0; /**< QBB assembly volume */
      G4AssemblyVolume* m_frontEnd = 0; /**< front-end electronics assembly volume */

      int m_numDecoupledPMTs = 0;  /**< number of optically decoupled PMT's */
      int m_numBrokenGlues = 0;    /**< number of broken glues */
      int m_numPeelOffRegions = 0; /**< number of peel-off regions */

    };

  } // namespace TOP
} // namespace Belle2

