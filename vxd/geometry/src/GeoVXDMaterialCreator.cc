/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Benjamin Schwenker                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vxd/geometry/GeoVXDMaterialCreator.h>
#include <vxd/geometry/GeoVXDComponents.h>
#include <geometry/CreatorFactory.h>
#include <geometry/Materials.h>
#include <simulation/background/BkgSensitiveDetector.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <G4Transform3D.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include "G4Polycone.hh"
#include <G4Tubs.hh>
#include <G4Cons.hh>
#include <G4Box.hh>
#include <string>
#include <vector>
#include <map>

using namespace std;

namespace Belle2 {

  using namespace geometry;

  namespace VXD {

    /** Create factory instance so that the framework can instantiate the VXDMaterialCreator */
    geometry::CreatorFactory<GeoVXDMaterialCreator> GeoVXDMaterialFactory("VXDMaterialCreator");

    //! Create a parameter object from the Gearbox XML parameters.
    VXDMaterialGeometryPar GeoVXDMaterialCreator::createConfiguration(const GearDir& content)
    {
      VXDMaterialGeometryPar vxdMaterialGeometryPar(content.getBool("RecordBackground", false));

      GearDir content1(content, "GapMomVolBack");
      // Read parameters for Backward Gap Mom Volume
      GapMomVolBackPar MomVolBackPar;
      for (const GearDir& GapVol : content1.getNodes("ZBound")) {
        const double rmin = GapVol.getLength("Rmin") / Unit::mm;
        const double rmax = GapVol.getLength("Rmax") / Unit::mm;
        const double z = GapVol.getLength("Z") / Unit::mm;
        MomVolBackPar.appendNode(rmin, rmax, z);
      }
      vxdMaterialGeometryPar.getMomVolBack() = MomVolBackPar;

      GearDir content2(content, "GapMomVolFor");
      // Read parameters for Forward Gap Mom Volume
      GapMomVolForPar MomVolForPar;
      for (const GearDir& GapVol : content2.getNodes("ZBound")) {
        const double rmin = GapVol.getLength("Rmin") / Unit::mm;
        const double rmax = GapVol.getLength("Rmax") / Unit::mm;
        const double z = GapVol.getLength("Z") / Unit::mm;
        MomVolForPar.appendNode(rmin, rmax, z);
      }
      vxdMaterialGeometryPar.getMomVolFor() = MomVolForPar;

      GearDir content3(content, "BEAST2Materials");
      // Read parameters to creates BEAST2 Material
      for (const GearDir& material : content3.getNodes("BEAST2Material")) {
        BeastMaterialsPar MaterialPar(
          material.getString("Name"),
          material.getString("material"),
          material.getInt("@id", 0),
          material.getLength("InnerR"),
          material.getLength("OuterR"),
          material.getLength("BackwardZ"),
          material.getLength("ForwardZ")
        );
        vxdMaterialGeometryPar.getbeastMaterials().push_back(MaterialPar);
      }

      GearDir content4(content, "BEAST2EclMaterials");
      // Read parameters to creates BEAST2 Material
      for (const GearDir& material : content4.getNodes("BEAST2Material")) {
        BeastEclMaterialsPar MaterialPar(
          material.getString("Name"),
          material.getString("material"),
          material.getInt("@id", 0),
          material.getLength("InnerR1"),
          material.getLength("OuterR1"),
          material.getLength("InnerR2"),
          material.getLength("OuterR2"),
          material.getLength("BackwardZ"),
          material.getLength("ForwardZ")
        );
        vxdMaterialGeometryPar.getbeastEclMaterials().push_back(MaterialPar);
      }
      return vxdMaterialGeometryPar;
    }


    void GeoVXDMaterialCreator::createGeometry(const VXDMaterialGeometryPar& parameters, G4LogicalVolume& topVolume, GeometryTypes)
    {
      G4Material* medAir = geometry::Materials::get("Air");
      m_defaultMaterial = topVolume.GetMaterial();

      const auto& MomVolFor = parameters.getMomVolFor();
      std::vector<double> motherforRmin =  MomVolFor.getRmin();
      std::vector<double> motherforRmax =  MomVolFor.getRmax();
      std::vector<double> motherforZ =  MomVolFor.getZ() ;

      G4Polycone* solid_gap_for = new G4Polycone("VXD.GAPFor", 0 * CLHEP::deg, 360.* CLHEP::deg,  MomVolFor.getNNodes(),
                                                 motherforZ.data(), motherforRmin.data(), motherforRmax.data());
      G4LogicalVolume* logical_gap_for = new G4LogicalVolume(solid_gap_for, medAir, "VXD.GAPFor", 0, 0, 0);
      new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, 0.0), logical_gap_for, "VXD.GAPFor", &topVolume, false, 1);

      const auto& MomVolBack = parameters.getMomVolBack();
      std::vector<double> motherbackRmin =  MomVolBack.getRmin();
      std::vector<double> motherbackRmax =  MomVolBack.getRmax();
      std::vector<double> motherbackZ =  MomVolBack.getZ() ;

      G4Polycone* solid_gap_back = new G4Polycone("VXD.GAPBack", 0 * CLHEP::deg, 360.* CLHEP::deg,  MomVolBack.getNNodes(),
                                                  motherbackZ.data(), motherbackRmin.data(), motherbackRmax.data());
      G4LogicalVolume* logical_gap_back = new G4LogicalVolume(solid_gap_back, medAir, "VXD.GAPBack", 0, 0, 0);
      new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, 0.0), logical_gap_back, "VXD.GAPBack", &topVolume, false, 1);


      //      Create Materials from BEAST 2  between CDC and ECL.
      G4Material* medAluminum = geometry::Materials::get("Al");
//      m_VisAttributes.push_back(new G4VisAttributes(true, G4Colour(0., 1., 0.)));
      for (const BeastMaterialsPar& material : parameters.getbeastMaterials()) {
        const int materialID = material.getIdentifier();
        const double materialInnerR = material.getInnerR() / Unit::mm;
        const double materialOuterR = material.getOuterR() / Unit::mm;
        const double materialBackwardZ = material.getBackwardZ() / Unit::mm;
        const double materialForwardZ = material.getForwardZ() / Unit::mm;
        const double cellR = (materialOuterR - materialInnerR) / 140;
        if (materialID < 2) {
          int blockid = 0;
          for (int iR = 0; iR < 140; iR++) {
            const double rmin = materialInnerR + iR * cellR;
            const double rmax = materialInnerR + (iR + 1) * cellR;
            const double materialThick = fabs(materialForwardZ - materialBackwardZ);
            const double materialPosZ = materialBackwardZ;
            for (int iPhi = 0; iPhi < 144; iPhi++) {
              const double SPhi = 360 / 144.*iPhi;
              const double DPhi = 360 / 144.;
              const int blockID = blockid;
              if (materialID < 1) {
                createTube(rmin, rmax, SPhi, DPhi, materialThick, materialPosZ, materialID, medAluminum, blockID, logical_gap_back);
              }
              if (materialID >= 1) {
                createTube(rmin, rmax, SPhi, DPhi, materialThick, materialPosZ, materialID, medAluminum, blockID, logical_gap_for);
              }
              blockid++;
            }
          }
        }
        if (materialID == 2) {
          int blockid = 0;
          for (int iZ = 0; iZ < 3; iZ++) {
            const double rmin = materialInnerR;
            const double rmax = materialOuterR;
            const double materialThick = fabs(materialForwardZ - materialBackwardZ) / 3;
            const double materialPosZ = materialBackwardZ + iZ * materialThick;
            for (int iPhi = 0; iPhi < 144; iPhi++) {
              const double SPhi = 360 / 144.*iPhi;
              const double DPhi = 360 / 144.;
              const int blockID = blockid;
              createTube(rmin, rmax, SPhi, DPhi, materialThick, materialPosZ, materialID, medAluminum,  blockID, logical_gap_for);
              blockid++;
            }
          }
        }
        if (materialID == 3) {
          int blockid = 0;
          const double rmin = materialInnerR;
          const double rmax = materialOuterR;
          const double materialThick = materialForwardZ - materialBackwardZ;
          const double materialPosZ = materialBackwardZ;
          for (int iPhi = 0; iPhi < 144; iPhi++) {
            const double SPhi = 360 / 144.*iPhi;
            const double DPhi = 360 / 144.;
            const int blockID = blockid;
            createTube(rmin, rmax, SPhi, DPhi, materialThick, materialPosZ, materialID, medAluminum,  blockID, logical_gap_for);
            blockid++;
          }
        }
      }
      //      Create Materials from BEAST 2 between barrel and endcap of ECL.
      for (const BeastEclMaterialsPar& material : parameters.getbeastEclMaterials()) {
        const int materialID = material.getIdentifier();
        const double materialInnerR1 = material.getInnerR1() / Unit::mm;
        const double materialOuterR1 = material.getOuterR1() / Unit::mm;
        const double materialInnerR2 = material.getInnerR2() / Unit::mm;
        const double materialOuterR2 = material.getOuterR2() / Unit::mm;
        const double materialBackwardZ = material.getBackwardZ() / Unit::mm;
        const double materialForwardZ = material.getForwardZ() / Unit::mm;
        const double thick = (materialForwardZ - materialBackwardZ) / 3.0;
        const double Hf1 = (materialInnerR2 - materialInnerR1) / 3.0;
        const double Hf2 = (materialOuterR2 - materialOuterR1) / 3.0;
        for (int iZ = 0; iZ < 3; iZ++) {
          const double rmin1 = materialInnerR1 + Hf1 * iZ;
          const double rmin2 = materialInnerR1 + Hf1 * (iZ + 1);
          const double rmax1 = materialOuterR1 + Hf2 * iZ;
          const double rmax2 = materialOuterR1 + Hf2 * (iZ + 1);
          const double posZ = materialBackwardZ + thick * iZ;
          for (int iPhi = 0; iPhi < 144; iPhi++) {
            const double SPhi = (360 / 144.) * iPhi;
            const double DPhi = 360 / 144.;
            G4Material* medAl = geometry::Materials::get("Al");
            const string storageName = "forward storage_" + to_string(iZ) + "_" + to_string(iPhi);
            if (materialID == 0) createCone(rmin1, rmax1, rmin2, rmax2, thick, SPhi, DPhi, posZ,  medAl, storageName, logical_gap_back);
            if (materialID == 1) createCone(rmin1, rmax1, rmin2, rmax2, thick, SPhi, DPhi, posZ,  medAl, storageName, logical_gap_for);
          }
        }
      }

    }

    void GeoVXDMaterialCreator::createTube(const double rmin, const double rmax, const double SPhi, const double DPhi,
                                           const double thick, const double posZ, const int id, G4Material* med, const int blockid, G4LogicalVolume*&  logical_gap)
    {
      const string solidName = "VXD.solid_" + to_string(id) + "_" + to_string(blockid);
      const string logicalName = "VXD.logical_" + to_string(id) + "_" + to_string(blockid);
      const string physicalName = "VXD.physical_" + to_string(id) + "_" + to_string(blockid);
      G4Tubs* solidV = new G4Tubs(solidName.c_str(), rmin * CLHEP::mm, rmax * CLHEP::mm,
                                  thick * CLHEP::mm / 2.0, SPhi * CLHEP::deg, DPhi * CLHEP::deg);
      G4LogicalVolume* logicalV = new G4LogicalVolume(solidV, med,
                                                      logicalName.c_str(), 0, 0, 0);
//      logicalV->SetVisAttributes(m_VisAttributes.back());
      new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, posZ * CLHEP::mm + thick * CLHEP::mm / 2.0), logicalV,
                        physicalName.c_str(), logical_gap, false, id);
    }

    void Belle2::VXD::GeoVXDMaterialCreator::createCone(const double rmin1, const double rmax1, const double rmin2, const double rmax2,
                                                        const double thick, const double SPhi, const double DPhi, const double posZ, G4Material* med, const string& name,
                                                        G4LogicalVolume*& top)
    {
      const string solidName = "solid" + name;
      const string logicalName = "logical" + name;
      const string physicalName = "physical" + name;
      G4Cons* storageConeShape = new G4Cons(solidName.c_str(), rmin1 * CLHEP::mm, rmax1 * CLHEP::mm,     rmin2 * CLHEP::mm,
                                            rmax2 * CLHEP::mm, thick * CLHEP::mm / 2.0, SPhi *  CLHEP::deg, DPhi * CLHEP::deg);
      G4LogicalVolume* storageCone = new G4LogicalVolume(storageConeShape, med, logicalName.c_str(), 0,   0, 0);
      //         storageCone->SetVisAttributes(m_VisAttributes.back());
      new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, posZ * CLHEP::mm + thick * CLHEP::mm / 2.0), storageCone, physicalName.c_str(), top,
                        false, 0);

    }

  } // VXD namespace
} // Belle2 namespace
