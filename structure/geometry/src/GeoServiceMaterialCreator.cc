/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <structure/geometry/GeoServiceMaterialCreator.h>
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

  namespace structure {

    /** Create factory instance so that the framework can instantiate the ServiceMaterialCreator */
    geometry::CreatorFactory<GeoServiceMaterialCreator> GeoServiceMaterialFactory("ServiceMaterialCreator");

    //! Create a parameter object from the Gearbox XML parameters.
    ServiceGapsMaterialsPar GeoServiceMaterialCreator::createConfiguration(const GearDir& content)
    {
      ServiceGapsMaterialsPar ServiceMaterialGeometryPar(content.getBool("RecordBackground", false));

      GearDir content0(content, "GapMomVolTopBack");
      // Read parameters for Backward Gap Mom Volume
      ServiceGapsMomVolPar MomVolTopBackPar;
      for (const GearDir& GapVol : content0.getNodes("ZBound")) {
        const double rmin = GapVol.getLength("Rmin") / Unit::mm;
        const double rmax = GapVol.getLength("Rmax") / Unit::mm;
        const double z = GapVol.getLength("Z") / Unit::mm;
        MomVolTopBackPar.appendNode(rmin, rmax, z);
      }
      ServiceMaterialGeometryPar.getMomVolTopBack() = MomVolTopBackPar;

      GearDir content1(content, "GapMomVolEclCoilBarrel");
      // Read parameters for Backward Gap Mom Volume
      ServiceGapsMomVolPar MomVolEclCoilBarrelPar;
      for (const GearDir& GapVol : content1.getNodes("ZBound")) {
        const double rmin = GapVol.getLength("Rmin") / Unit::mm;
        const double rmax = GapVol.getLength("Rmax") / Unit::mm;
        const double z = GapVol.getLength("Z") / Unit::mm;
        MomVolEclCoilBarrelPar.appendNode(rmin, rmax, z);
      }
      ServiceMaterialGeometryPar.getMomVolEclCoilBarrel() = MomVolEclCoilBarrelPar;

      GearDir content2(content, "GapMomVolBack");
      // Read parameters for Backward Gap Mom Volume
      ServiceGapsMomVolPar MomVolBackPar;
      for (const GearDir& GapVol : content2.getNodes("ZBound")) {
        const double rmin = GapVol.getLength("Rmin") / Unit::mm;
        const double rmax = GapVol.getLength("Rmax") / Unit::mm;
        const double z = GapVol.getLength("Z") / Unit::mm;
        MomVolBackPar.appendNode(rmin, rmax, z);
      }
      ServiceMaterialGeometryPar.getMomVolBack() = MomVolBackPar;

      GearDir content3(content, "GapMomVolFor");
      // Read parameters for Forward Gap Mom Volume
      ServiceGapsMomVolPar MomVolForPar;
      for (const GearDir& GapVol : content3.getNodes("ZBound")) {
        const double rmin = GapVol.getLength("Rmin") / Unit::mm;
        const double rmax = GapVol.getLength("Rmax") / Unit::mm;
        const double z = GapVol.getLength("Z") / Unit::mm;
        MomVolForPar.appendNode(rmin, rmax, z);
      }
      ServiceMaterialGeometryPar.getMomVolFor() = MomVolForPar;

      GearDir content4(content, "ServiceGapsMaterials");
      // Read parameters to creates ServiceGaps Material in the gap between CDC and ECL, ARICH and TOP, TOP and ECL.
      for (const GearDir& material : content4.getNodes("ServiceGapsMaterial")) {
        ServiceGapsMaterialsCdcArichTopPar MaterialPar(
          material.getString("Name"),
          material.getString("material"),
          material.getInt("@id", 0),
          material.getLength("InnerR"),
          material.getLength("OuterR"),
          material.getLength("BackwardZ"),
          material.getLength("ForwardZ")
        );
        ServiceMaterialGeometryPar.getServiceGapsMaterials().push_back(MaterialPar);
      }

      GearDir content5(content, "ServiceGapsEclMaterials");
      // Read parameters to creates ServiceGaps Material in the gap between barrel and endcap of ECL.
      for (const GearDir& material : content5.getNodes("ServiceGapsMaterial")) {
        ServiceGapsMaterialsEclPar MaterialPar(
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
        ServiceMaterialGeometryPar.getServiceGapsEclMaterials().push_back(MaterialPar);
      }

      GearDir content6(content, "TicknessDensity");
      // Read thickness and density for Gaps Volume
      ThicknessDensityPar ThickPar(
        content6.getInt("IRCDCBack"),
        content6.getInt("IPhiCDCBack"),
        content6.getInt("IRCDCFor"),
        content6.getInt("IPhiCDCFor"),
        content6.getInt("IRECLBack"),
        content6.getInt("IZECLBack"),
        content6.getInt("IPhiECLBack"),
        content6.getInt("IRECLFor"),
        content6.getInt("IZECLFor"),
        content6.getInt("IPhiECLFor"),
        content6.getInt("IZARICHFor"),
        content6.getInt("IPhiARICHFor"),
        content6.getInt("IPhiTOPBack"),
        content6.getInt("IPhiTOPFor"),
        content6.getInt("IZECLCOILBar"),
        content6.getInt("IPhiECLCOILBar"),
        content6.getArray("thicknesses")
      );
      ServiceMaterialGeometryPar.getthick() = ThickPar;


      return ServiceMaterialGeometryPar;
    }


    void GeoServiceMaterialCreator::createGeometry(const ServiceGapsMaterialsPar& parameters, G4LogicalVolume& topVolume, GeometryTypes)
    {
      G4Material* medAir = geometry::Materials::get("Air");

      const auto& MomVolFor = parameters.getMomVolFor();
      std::vector<double> motherforRmin =  MomVolFor.getRmin();
      std::vector<double> motherforRmax =  MomVolFor.getRmax();
      std::vector<double> motherforZ =  MomVolFor.getZ() ;

      G4Polycone* solid_gap_for = new G4Polycone("ServiceMaterial.GAPFor", 0 * CLHEP::deg, 360.* CLHEP::deg,  MomVolFor.getNNodes(),
                                                 motherforZ.data(), motherforRmin.data(), motherforRmax.data());
      G4LogicalVolume* logical_gap_for = new G4LogicalVolume(solid_gap_for, medAir, "ServiceMaterial.GAPFor", 0, 0, 0);
      new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, 0.0), logical_gap_for, "ServiceMaterial.GAPFor", &topVolume, false, 1);

      const auto& MomVolBack = parameters.getMomVolBack();
      std::vector<double> motherbackRmin =  MomVolBack.getRmin();
      std::vector<double> motherbackRmax =  MomVolBack.getRmax();
      std::vector<double> motherbackZ =  MomVolBack.getZ();

      G4Polycone* solid_gap_back = new G4Polycone("ServiceMaterial.GAPBack", 0 * CLHEP::deg, 360.* CLHEP::deg,  MomVolBack.getNNodes(),
                                                  motherbackZ.data(), motherbackRmin.data(), motherbackRmax.data());
      G4LogicalVolume* logical_gap_back = new G4LogicalVolume(solid_gap_back, medAir, "ServiceMaterial.GAPBack", 0, 0, 0);
      new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, 0.0), logical_gap_back, "ServiceMaterial.GAPBack", &topVolume, false, 1);

      const auto& MomVolTopBack = parameters.getMomVolTopBack();
      std::vector<double> mothertopbackRmin =  MomVolTopBack.getRmin();
      std::vector<double> mothertopbackRmax =  MomVolTopBack.getRmax();
      std::vector<double> mothertopbackZ =  MomVolTopBack.getZ();

      G4Polycone* solid_gap_topback = new G4Polycone("ServiceMaterial.GAPTopBack", 0 * CLHEP::deg, 360.* CLHEP::deg,
                                                     MomVolTopBack.getNNodes(),
                                                     mothertopbackZ.data(), mothertopbackRmin.data(), mothertopbackRmax.data());
      G4LogicalVolume* logical_gap_topback = new G4LogicalVolume(solid_gap_topback, medAir, "ServiceMaterial.GAPTopBack", 0, 0, 0);
      new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, 0.0), logical_gap_topback, "ServiceMaterial.GAPTopBack", &topVolume, false, 1);

      const auto& MomVolEclCoilBarrel = parameters.getMomVolEclCoilBarrel();
      std::vector<double> mothereclcoilbarrelRmin =  MomVolEclCoilBarrel.getRmin();
      std::vector<double> mothereclcoilbarrelRmax =  MomVolEclCoilBarrel.getRmax();
      std::vector<double> mothereclcoilbarrelZ =  MomVolEclCoilBarrel.getZ();

      G4Polycone* solid_gap_eclcoilbarrel = new G4Polycone("ServiceMaterial.GAPEclCoilBarrel", 0 * CLHEP::deg, 360.* CLHEP::deg,
                                                           MomVolEclCoilBarrel.getNNodes(),
                                                           mothereclcoilbarrelZ.data(), mothereclcoilbarrelRmin.data(), mothereclcoilbarrelRmax.data());
      G4LogicalVolume* logical_gap_eclcoilbarrel = new G4LogicalVolume(solid_gap_eclcoilbarrel, medAir,
          "ServiceMaterial.GAPEclCoilBarrel", 0, 0, 0);
      new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, 0.0), logical_gap_eclcoilbarrel, "ServiceMaterial.GAPEclCoilBarrel", &topVolume, false,
                        1);

      const auto& Thick = parameters.getthick();
      std::vector<double> Thickness =  Thick.getthickness();
      int IRCDCB =  Thick.getIRCDCB();
      int IPhiCDCB =  Thick.getIPhiCDCB();
      int IRCDCF =  Thick.getIRCDCF();
      int IPhiCDCF =  Thick.getIPhiCDCF();
      int IRECLB =  Thick.getIRECLB();
      int IZECLB = Thick.getIZECLB();
      int IPhiECLB =  Thick.getIPhiECLB();
      int IRECLF =  Thick.getIRECLF();
      int IZECLF = Thick.getIZECLF();
      int IPhiECLF =  Thick.getIPhiECLF();
      int IZARICHF =  Thick.getIZARICHF();
      int IPhiARICHF =  Thick.getIPhiARICHF();
      int IPhiTOPB =  Thick.getIPhiTOPB();
      int IPhiTOPF =  Thick.getIPhiTOPF();
      int IPhiECLCOILB =  Thick.getIPhiECLCOILB();
      int IZECLCOILB = Thick.getIZECLCOILB();

      for (const ServiceGapsMaterialsCdcArichTopPar& material : parameters.getServiceGapsMaterials()) {
        const int materialID = material.getIdentifier();
        const double materialInnerR = material.getInnerR() / Unit::mm;
        const double materialOuterR = material.getOuterR() / Unit::mm;
        const double materialBackwardZ = material.getBackwardZ() / Unit::mm;
        const double materialForwardZ = material.getForwardZ() / Unit::mm;
        //      Create Service Materials in the gap between CDC and ECL.
        if (materialID < 2) {
          int blockid = 0;
          double IR = 0, IPhi = 0;
          if (materialID < 1) {IR = IRCDCB; IPhi = IPhiCDCB;}
          else {IR = IRCDCF; IPhi = IPhiCDCF;}
          const double cellR = (materialOuterR - materialInnerR) / IRCDCB;
          for (int iR = 0; iR < IR; iR++) {
            const double rmin = materialInnerR + iR * cellR;
            const double rmax = materialInnerR + (iR + 1) * cellR;
            for (int iPhi = 0; iPhi < IPhi; iPhi++) {
              const double SPhi = 360. / IPhi * iPhi;
              const double DPhi = 360. / IPhi;
              if (materialID < 1) {
                const string storageName = "Service_CDC_ECL_Bwd_" + to_string(iR) + "_" + to_string(iPhi);
                const double materialThick = Thickness[blockid] / Unit::mm;
                const double materialPosZ = materialForwardZ - materialThick;
                G4Material* mCDCGapback = geometry::Materials::get("CDCGapback");
                createTube(rmin, rmax, SPhi, DPhi, materialThick, materialPosZ, mCDCGapback, storageName, logical_gap_back);
              }
              if (materialID >= 1) {
                const string storageName = "Service_CDC_ARICH_Fwd_" + to_string(iR) + "_" + to_string(iPhi);
                const double materialThick = Thickness[blockid + IRCDCB * IPhiCDCB] / Unit::mm;
                const double materialPosZ = materialBackwardZ;
                G4Material* mCDCGapfor = geometry::Materials::get("CDCGapfor");
                createTube(rmin, rmax, SPhi, DPhi, materialThick, materialPosZ, mCDCGapfor, storageName, logical_gap_for);
              }
              blockid++;
            }
          }
        }
        //      Create Service Materials in the gap between ARICH and TOP.
        if (materialID == 2) {
          int blockid = 0;
          const double materialThick = fabs(materialForwardZ - materialBackwardZ) / IZARICHF;
          for (int iZ = 0; iZ < IZARICHF; iZ++) {
            const double rmax = materialOuterR;
            const double materialPosZ = materialBackwardZ + iZ * materialThick;
            for (int iPhi = 0; iPhi < IPhiARICHF; iPhi++) {
              const double SPhi = 360. / IPhiARICHF * iPhi;
              const double DPhi = 360. / IPhiARICHF;
              const double materialRThick = Thickness[blockid + IRCDCB * IPhiCDCB + IRCDCF * IPhiCDCF] / Unit::mm;
              const double rmin = rmax - materialRThick;
              G4Material* ArichAir = geometry::Materials::get("Arich_TopGapfor");
              const string storageName = "Service_ARICH_TOP_Fwd_" + to_string(iZ) + "_" + to_string(iPhi);
              createTube(rmin, rmax, SPhi, DPhi, materialThick, materialPosZ, ArichAir,  storageName, logical_gap_for);
              blockid++;
            }
          }
        }
        //      Create Service Materials in the gap between TOP and ECL.
        if (materialID >= 3 && materialID < 5) {
          int blockid = 0;
          int IPhiTOP = 0;
          if (materialID == 3) {IPhiTOP = IPhiTOPB;}
          else {IPhiTOP = IPhiTOPF;}
          const double rmin = materialInnerR;
          const double rmax = materialOuterR;
          for (int iPhi = 0; iPhi < IPhiTOP; iPhi++) {
            const double SPhi = 360. / IPhiTOP * iPhi;
            const double DPhi = 360. / IPhiTOP;
            if (materialID == 3) {
              const double materialThick = Thickness[blockid + IRCDCB * IPhiCDCB + IRCDCF * IPhiCDCF + IZARICHF * IPhiARICHF] / Unit::mm;
              const double materialPosZ = materialForwardZ - materialThick;
              G4Material* TopbackAir = geometry::Materials::get("Top_ECLGapback");
              const string storageName = "Service_TOP_ECL_Bwd_" + to_string(iPhi);
              createTube(rmin, rmax, SPhi, DPhi, materialThick, materialPosZ, TopbackAir,  storageName, logical_gap_topback);
            }
            if (materialID == 4) {
              const double materialThick = Thickness[blockid + IRCDCB * IPhiCDCB + IRCDCF * IPhiCDCF + IZARICHF * IPhiARICHF + IPhiTOPB] /
                                           Unit::mm;
              const double materialPosZ = materialBackwardZ;
              G4Material* TopforAir = geometry::Materials::get("Top_ECLGapfor");
              const string storageName = "Service_TOP_ECL_Fwd_" + to_string(iPhi);
              createTube(rmin, rmax, SPhi, DPhi, materialThick, materialPosZ, TopforAir,  storageName, logical_gap_for);
            }
            blockid++;
          }
        }
        //      Create Service Materials in the gap between barrel ECL and Coil.
        if (materialID == 5) {
          int blockid = 0;
          const double materialThick = fabs(materialForwardZ - materialBackwardZ) / IZECLCOILB;
          for (int iZ = 0; iZ < IZECLCOILB; iZ++) {
            const double rmin = materialInnerR;
            const double materialPosZ = materialBackwardZ + iZ * materialThick;
            for (int iPhi = 0; iPhi < IPhiECLCOILB; iPhi++) {
              const double SPhi = 360. / IPhiECLCOILB * iPhi;
              const double DPhi = 360. / IPhiECLCOILB;
              const double materialRThick = Thickness[blockid + IRCDCB * IPhiCDCB + IRCDCF * IPhiCDCF + IZARICHF * IPhiARICHF + IPhiTOPB +
                                                      IPhiTOPF] / Unit::mm;
              const double rmax = rmin + materialRThick;
              G4Material* EclCoilAir = geometry::Materials::get("ECL_COILbarrel");
              const string storageName = "Service_ECL_COIL_Barrel_" + to_string(iZ) + "_" + to_string(iPhi);
              createTube(rmin, rmax, SPhi, DPhi, materialThick, materialPosZ, EclCoilAir,  storageName, logical_gap_eclcoilbarrel);
              blockid++;
            }
          }
        }
      }

      //      Create Service Materials in the gap between barrel and endcap of ECL.
      for (const ServiceGapsMaterialsEclPar& material : parameters.getServiceGapsEclMaterials()) {
        int blockid = 0;
        int IRECL = 0, IZECL = 0, IPhiECL = 0;
        const int materialID = material.getIdentifier();
        if (materialID < 1) {IRECL = IRECLB; IZECL = IZECLB; IPhiECL = IPhiECLB;}
        else {IRECL = IRECLF; IZECL = IZECLF; IPhiECL = IPhiECLF;}
        const double materialInnerR1 = material.getInnerR1() / Unit::mm;
        const double materialOuterR1 = material.getOuterR1() / Unit::mm;
        const double materialInnerR2 = material.getInnerR2() / Unit::mm;
        const double materialOuterR2 = material.getOuterR2() / Unit::mm;
        const double materialBackwardZ = material.getBackwardZ() / Unit::mm;
        const double materialForwardZ = material.getForwardZ() / Unit::mm;
        const double interval = (materialForwardZ - materialBackwardZ) / IZECL ;
        const double Hf1 = (materialOuterR1 - materialInnerR1) / IRECL;
        const double Hf2 = (materialOuterR2 - materialInnerR2) / IRECL;
        for (int iR = 0; iR < IRECL; iR++) {
          const double Rmin1 = materialInnerR1 + Hf1 * iR;
          const double Rmax1 = materialInnerR1 + Hf1 * (iR + 1);
          const double Rmin2 = materialInnerR2 + Hf2 * iR;
          const double Rmax2 = materialInnerR2 + Hf2 * (iR + 1);
          const double Hrmax = (Rmax2 - Rmax1) / IZECL;
          const double Hrmin = (Rmin2 - Rmin1) / IZECL;
          for (int iZ = 0; iZ < IZECL; iZ++) {
            const double BackwardposZ = materialBackwardZ + interval * iZ;
            const double ForwardposZ = materialBackwardZ + interval * (iZ + 1);
            for (int iPhi = 0; iPhi < IPhiECL; iPhi++) {
              const double SPhi = (360. / IPhiECL) * iPhi;
              const double DPhi = 360. / IPhiECL;
              if (materialID == 0) {
                const double thick = Thickness[blockid + IRCDCB * IPhiCDCB + IRCDCF * IPhiCDCF + IZARICHF * IPhiARICHF + IPhiTOPB + IPhiTOPF +
                                               IPhiECLCOILB * IZECLCOILB] / Unit::mm;
                const double rmin2 = Rmin1 + Hrmin * (iZ + 1);
                const double rmax2 = Rmax1 + Hrmax * (iZ + 1);
                const double rmin1 = rmin2 - Hrmin * thick / interval ;
                const double rmax1 = rmax2 - Hrmax * thick / interval ;
                const double posZ = ForwardposZ - thick;
                G4Material* ECLbackAir = geometry::Materials::get("ECLGapback");
                const string storageName = "Service_ECLGAPS_Bwd_" + to_string(iR) + "_" + to_string(iZ) + "_" + to_string(iPhi);
                createCone(rmin1, rmax1, rmin2, rmax2, thick, SPhi, DPhi, posZ,  ECLbackAir, storageName, logical_gap_back);
              }
              if (materialID == 1) {
                const double thick = Thickness[blockid + IRCDCB * IPhiCDCB + IRCDCF * IPhiCDCF + IZARICHF * IPhiARICHF + IPhiTOPB + IPhiTOPF +
                                               IPhiECLCOILB * IZECLCOILB + IZECLB * IRECLB * IPhiECLB ] / Unit::mm;
                const double rmin1 = Rmin1 + Hrmin * iZ;
                const double rmax1 = Rmax1 + Hrmax * iZ;
                const double rmin2 = rmin1 + Hrmin * thick / interval;
                const double rmax2 = rmax1 + Hrmax * thick / interval;
                const double posZ = BackwardposZ;
                G4Material* ECLforAir = geometry::Materials::get("ECLGapfor");
                const string storageName = "Service_ECLGAPS_Fwd_" + to_string(iR) + "_" + to_string(iZ) + "_" + to_string(iPhi);
                createCone(rmin1, rmax1, rmin2, rmax2, thick, SPhi, DPhi, posZ,  ECLforAir, storageName, logical_gap_for);
              }
              blockid++;
            }
          }
        }
      }
    }

    void GeoServiceMaterialCreator::createTube(const double rmin, const double rmax, const double SPhi, const double DPhi,
                                               const double thick, const double posZ, G4Material* med, const string& name, G4LogicalVolume*&  logical_gap)
    {
      const string solidName = "solid_" + name;
      const string logicalName = "logical_" + name;
      const string physicalName = "physical_" + name;
      G4Tubs* solidV = new G4Tubs(solidName.c_str(), rmin * CLHEP::mm, rmax * CLHEP::mm,
                                  thick * CLHEP::mm / 2.0, SPhi * CLHEP::deg, DPhi * CLHEP::deg);
      G4LogicalVolume* logicalV = new G4LogicalVolume(solidV, med, logicalName.c_str(), 0, 0, 0);
      new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, posZ * CLHEP::mm + thick * CLHEP::mm / 2.0), logicalV,
                        physicalName.c_str(), logical_gap, false, 0);
    }

    void GeoServiceMaterialCreator::createCone(const double rmin1, const double rmax1, const double rmin2,
                                               const double rmax2,
                                               const double thick, const double SPhi, const double DPhi, const double posZ, G4Material* med,
                                               const string& name, G4LogicalVolume*& top)
    {
      const string solidName = "solid" + name;
      const string logicalName = "logical" + name;
      const string physicalName = "physical" + name;
      G4Cons* storageConeShape = new G4Cons(solidName.c_str(), rmin1 * CLHEP::mm, rmax1 * CLHEP::mm,     rmin2 * CLHEP::mm,
                                            rmax2 * CLHEP::mm, thick * CLHEP::mm / 2.0, SPhi *  CLHEP::deg, DPhi * CLHEP::deg);
      G4LogicalVolume* storageCone = new G4LogicalVolume(storageConeShape, med, logicalName.c_str(), 0,   0, 0);
      new G4PVPlacement(0, G4ThreeVector(0.0, 0.0, posZ * CLHEP::mm + thick * CLHEP::mm / 2.0), storageCone, physicalName.c_str(), top,
                        false, 0);

    }

  } // structure namespace
} // Belle2 namespace
