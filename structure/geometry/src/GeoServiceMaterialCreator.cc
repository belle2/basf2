/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yu Hu and Torben Ferber                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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

      GearDir content1(content, "GapMomVolBack");
      // Read parameters for Backward Gap Mom Volume
      ServiceGapsMomVolPar MomVolBackPar;
      for (const GearDir& GapVol : content1.getNodes("ZBound")) {
        const double rmin = GapVol.getLength("Rmin") / Unit::mm;
        const double rmax = GapVol.getLength("Rmax") / Unit::mm;
        const double z = GapVol.getLength("Z") / Unit::mm;
        MomVolBackPar.appendNode(rmin, rmax, z);
      }
      ServiceMaterialGeometryPar.getMomVolBack() = MomVolBackPar;

      GearDir content2(content, "GapMomVolFor");
      // Read parameters for Forward Gap Mom Volume
      ServiceGapsMomVolPar MomVolForPar;
      for (const GearDir& GapVol : content2.getNodes("ZBound")) {
        const double rmin = GapVol.getLength("Rmin") / Unit::mm;
        const double rmax = GapVol.getLength("Rmax") / Unit::mm;
        const double z = GapVol.getLength("Z") / Unit::mm;
        MomVolForPar.appendNode(rmin, rmax, z);
      }
      ServiceMaterialGeometryPar.getMomVolFor() = MomVolForPar;

      GearDir content3(content, "ServiceGapsMaterials");
      // Read parameters to creates ServiceGaps Material in the gap between CDC and ECL, ARICH and TOP, TOP and ECL.
      for (const GearDir& material : content3.getNodes("ServiceGapsMaterial")) {
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

      GearDir content4(content, "ServiceGapsEclMaterials");
      // Read parameters to creates ServiceGaps Material in the gap between barrel and endcap of ECL.
      for (const GearDir& material : content4.getNodes("ServiceGapsMaterial")) {
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

      GearDir content5(content, "TicknessDensity");
      // Read thickness and density for Gaps Volume
      ThicknessDensityPar ThickPar(
        content5.getInt("IRCDCBack"),
        content5.getInt("IPhiCDCBack"),
        content5.getInt("IRCDCFor"),
        content5.getInt("IPhiCDCFor"),
        content5.getInt("IRECLBack"),
        content5.getInt("IPhiECLBack"),
        content5.getInt("IRECLFor"),
        content5.getInt("IPhiECLFor"),
        content5.getInt("IRARICHFor"),
        content5.getInt("IPhiARICHFor"),
        content5.getInt("IPhiTOPFor"),
        content5.getArray("thicknesses"),
        content5.getArray("density")
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

      const auto& Thick = parameters.getthick();
      std::vector<double> Thickness =  Thick.getthickness();
      std::vector<double> Density =  Thick.getdensity();
      double IRCDCB =  Thick.getIRCDCB();
      double IPhiCDCB =  Thick.getIPhiCDCB();
      double IRCDCF =  Thick.getIRCDCF();
      double IPhiCDCF =  Thick.getIPhiCDCF();
      double IZECLB =  Thick.getIRECLB();
      double IPhiECLB =  Thick.getIPhiECLB();
      double IZECLF =  Thick.getIRECLF();
      double IPhiECLF =  Thick.getIPhiECLF();
      double IZARICHF =  Thick.getIRARICHF();
      double IPhiARICHF =  Thick.getIPhiARICHF();
      double IPhiTOPF =  Thick.getIPhiTOPF();

      for (const ServiceGapsMaterialsCdcArichTopPar& material : parameters.getServiceGapsMaterials()) {
        const int materialID = material.getIdentifier();
        const double materialInnerR = material.getInnerR() / Unit::mm;
        const double materialOuterR = material.getOuterR() / Unit::mm;
        const double materialBackwardZ = material.getBackwardZ() / Unit::mm;
        const double materialForwardZ = material.getForwardZ() / Unit::mm;
        //      Create Materials from BEAST 2 in the gap between CDC and ECL.
        if (materialID < 2) {
          int blockid = 0;
          double IR = 0, IPhi = 0;
          if (materialID < 1) {IR = IRCDCB; IPhi = IPhiCDCB;}
          else {IR = IRCDCF; IPhi = IPhiCDCF;}
          const double cellR = (materialOuterR - materialInnerR) / IRCDCB;
          for (int iR = 0; iR < IR; iR++) {
            const double rmin = materialInnerR + iR * cellR;
            const double rmax = materialInnerR + (iR + 1) * cellR;
            const double materialPosZ = materialBackwardZ;
            for (int iPhi = 0; iPhi < IPhi; iPhi++) {
              const double SPhi = 360. / IPhi * iPhi;
              const double DPhi = 360. / IPhi;
              if (materialID < 1) {
                const string storageName = "Service_CDC_ECL_Bwd_" + to_string(iR) + "_" + to_string(iPhi);
                const double materialThick = Thickness[blockid] / Unit::mm;
                G4Material* mCDCGapback = geometry::Materials::get("CDCGapback");
                createTube(rmin, rmax, SPhi, DPhi, materialThick, materialPosZ, mCDCGapback, storageName, logical_gap_back);
              }
              if (materialID >= 1) {
                const string storageName = "Service_CDC_ARICH_Fwd_" + to_string(iR) + "_" + to_string(iPhi);
                const double materialThick = Thickness[blockid + IRCDCB * IPhiCDCB] / Unit::mm;
                G4Material* mCDCGapfor = geometry::Materials::get("CDCGapfor");
                createTube(rmin, rmax, SPhi, DPhi, materialThick, materialPosZ, mCDCGapfor, storageName, logical_gap_for);
              }
              blockid++;
            }
          }
        }
        //      Create Materials from BEAST 2 in the gap between ARICH and TOP.
        if (materialID == 2) {
          int blockid = 0;
          for (int iZ = 0; iZ < IZARICHF; iZ++) {
            const double rmin = materialInnerR;
            const double rmax = materialOuterR;
            const double materialThick = fabs(materialForwardZ - materialBackwardZ) / IZARICHF;
            const double materialPosZ = materialBackwardZ + iZ * materialThick;
            for (int iPhi = 0; iPhi < IPhiARICHF; iPhi++) {
              const double SPhi = 360. / IPhiARICHF * iPhi;
              const double DPhi = 360. / IPhiARICHF;
              double density = Density[blockid];
              G4Material* ArichAir = geometry::Materials::get("Arich_TopGapback");
              G4Material* medArichGap = new G4Material("ArichGap_" + to_string(iZ) + "_" + to_string(iPhi), density, 1);
              medArichGap->AddMaterial(ArichAir, 1.);
              const string storageName = "Service_ARICH_TOP_Fwd_" + to_string(iZ) + "_" + to_string(iPhi);
              createTube(rmin, rmax, SPhi, DPhi, materialThick, materialPosZ, medArichGap,  storageName, logical_gap_for);
              blockid++;
            }
          }
        }
        //      Create Materials from BEAST 2 in the gap between TOP and ECL.
        if (materialID == 3) {
          int blockid = 0;
          const double rmin = materialInnerR;
          const double rmax = materialOuterR;
          const double materialThick = materialForwardZ - materialBackwardZ;
          const double materialPosZ = materialBackwardZ;
          for (int iPhi = 0; iPhi < IPhiTOPF; iPhi++) {
            const double SPhi = 360. / IPhiTOPF * iPhi;
            const double DPhi = 360. / IPhiTOPF;
            double density = Density[blockid + IZARICHF * IPhiARICHF];
            G4Material* TopAir = geometry::Materials::get("Top_ECLGapback");
            G4Material* medTopGap = new G4Material("TopGap_" + to_string(iPhi), density, 1);
            medTopGap->AddMaterial(TopAir, 1.);
            const string storageName = "Service_TOP_ECL_Fwd_" + to_string(iPhi);
            createTube(rmin, rmax, SPhi, DPhi, materialThick, materialPosZ, medTopGap,  storageName, logical_gap_for);
            blockid++;
          }
        }
      }
      //      Create Materials from BEAST 2 in the gap between barrel and endcap of ECL.
      for (const ServiceGapsMaterialsEclPar& material : parameters.getServiceGapsEclMaterials()) {
        int blockid = 0;
        double IZECL = 0, IPhiECL = 0;
        const int materialID = material.getIdentifier();
        if (materialID < 1) {IZECL = IZECLB; IPhiECL = IPhiECLB;}
        else {IZECL = IZECLF; IPhiECL = IPhiECLF;}
        const double materialInnerR1 = material.getInnerR1() / Unit::mm;
        const double materialOuterR1 = material.getOuterR1() / Unit::mm;
        const double materialInnerR2 = material.getInnerR2() / Unit::mm;
        const double materialOuterR2 = material.getOuterR2() / Unit::mm;
        const double materialBackwardZ = material.getBackwardZ() / Unit::mm;
        const double materialForwardZ = material.getForwardZ() / Unit::mm;
        const double thick = materialForwardZ - materialBackwardZ ;
        const double Hf1 = (materialOuterR1 - materialInnerR1) / IZECL;
        const double Hf2 = (materialOuterR2 - materialInnerR2) / IZECL;
        for (int iZ = 0; iZ < IZECL; iZ++) {
          const double rmin1 = materialInnerR1 + Hf1 * iZ;
          const double rmax1 = materialInnerR1 + Hf1 * (iZ + 1);
          const double rmin2 = materialInnerR2 + Hf2 * iZ;
          const double rmax2 = materialInnerR2 + Hf2 * (iZ + 1);
          const double posZ = materialBackwardZ ;
          for (int iPhi = 0; iPhi < IPhiECL; iPhi++) {
            const double SPhi = (360. / IPhiECL) * iPhi;
            const double DPhi = 360. / IPhiECL;
            if (materialID == 0) {
              double density = Density[blockid + IZARICHF * IPhiARICHF + IPhiTOPF];
              G4Material* ECLbackAir = geometry::Materials::get("ECLGapback");
              G4Material* medECLback = new G4Material("ECLback_" + to_string(iZ) + "_" + to_string(iPhi), density, 1);
              medECLback->AddMaterial(ECLbackAir, 1.);
              const string storageName = "Service_ECLGAPS_Bwd_" + to_string(iZ) + "_" + to_string(iPhi);
              createCone(rmin1, rmax1, rmin2, rmax2, thick, SPhi, DPhi, posZ,  medECLback, storageName, logical_gap_back);
            }
            if (materialID == 1) {
              double density = Density[blockid + IZARICHF * IPhiARICHF + IPhiTOPF + IZECLB * IPhiECLB];
              G4Material* ECLforAir = geometry::Materials::get("ECLGapfor");
              G4Material* medECLfor = new G4Material("ECLfor_" + to_string(iZ) + "_" + to_string(iPhi), density, 1);
              medECLfor->AddMaterial(ECLforAir, 1.);
              const string storageName = "Service_ECLGAPS_Fwd_" + to_string(iZ) + "_" + to_string(iPhi);
              createCone(rmin1, rmax1, rmin2, rmax2, thick, SPhi, DPhi, posZ,  medECLfor, storageName, logical_gap_for);
            }
          }
        }
        blockid++;
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
