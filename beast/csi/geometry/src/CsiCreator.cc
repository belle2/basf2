/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <beast/csi/geometry/CsiCreator.h>
#include <beast/csi/simulation/SensitiveDetector.h>
#include <beast/csi/geometry/CsiGeometryPar.h>
#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <cmath>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <G4AssemblyVolume.hh>
#include <G4LogicalVolume.hh>

//Shapes
#include <G4Trap.hh>
#include <G4Box.hh>
#include <G4SubtractionSolid.hh>

//Visualization Attributes
#include <G4VisAttributes.hh>

#define PI 3.14159265358979323846

using namespace std;
using namespace boost;


namespace Belle2 {

  /** Namespace to encapsulate code needed for simulation and reconstrucion of the CSI detector */
  namespace csi {

    // Register the creator
    /** Creator creates the CSI geometry */
    geometry::CreatorFactory<CsiCreator> CsiFactory("CSICreator");

    // add foil thickness //
    const double avoidov = 1 + 1E-6; /**< foil inside is a little bit lager than crystal to avoid overlap */
    ///////////////////////


    CsiCreator::CsiCreator(): m_sensitive(0)
    {
      //m_sensitive = new SensitiveDetector();
    }

    CsiCreator::~CsiCreator()
    {
      if (m_sensitive) delete m_sensitive;
    }

    void CsiCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type)
    {

      m_sensitive = new SensitiveDetector();

      //Print the type (not used for now)
      B2DEBUG(200, "CsI Geometry Type: " << type);

      // Print list of defined materials
      /*
      G4NistManager* nistManager = G4NistManager::Instance();
      cout << *(G4Material::GetMaterialTable()) << endl;
      */

      G4Transform3D BrR = G4RotateZ3D(0.0);

      int nEnc = content.getNumberNodes("/Enclosures/Enclosure");

      G4AssemblyVolume* assemblyEnclosures = new G4AssemblyVolume();
      for (int iEnc = 1; iEnc <= nEnc; iEnc++) {
        BuildEnclosure(content, assemblyEnclosures, "side", iEnc);
        //
      }

      assemblyEnclosures->MakeImprint(&topVolume, BrR);

      // Show cell IDs and volume names
      B2INFO("Positions of the individual CsI crystals");
      CsiGeometryPar* eclp = CsiGeometryPar::Instance();
      unsigned int i = 0;
      for (std::vector<G4VPhysicalVolume*>::iterator it = assemblyEnclosures->GetVolumesIterator();
           i != assemblyEnclosures->TotalImprintedVolumes();
           ++it, ++i) {

        G4VPhysicalVolume* volume = *it;
        string VolumeName = volume->GetName();
        if (VolumeName.find("Crystal") < string::npos) {
          B2INFO("Crystal Number " << eclp->CsiVolNameToCellID(VolumeName) <<
                 " placed at (r[cm],[deg],z[cm]) = (" << setprecision(1) <<  fixed <<
                 volume->GetTranslation().perp() / CLHEP::cm << "," <<
                 volume->GetTranslation().phi() * 180.0 / PI << "," <<
                 volume->GetTranslation().z() / CLHEP::cm << ")");
        }


      }// for all physical volumes in the assembly
    }// create

    void CsiCreator::PutCrystal(const GearDir& content,
                                G4AssemblyVolume* assembly,
                                G4Transform3D position,
                                int iEnclosure,
                                int iCry)
    {
      if (iCry <= 0) return;

      GearDir counter(content);
      double foilthickness = counter.getLength("/Wrapping/Thickness") * CLHEP::cm;
      G4Material* foilMaterial = geometry::Materials::get(counter.getString("/Wrapping/Material"));


      int nCry = content.getNumberNodes("/EndCapCrystals/EndCapCrystal");
      if (iCry > nCry) {
        B2ERROR("CsiCreator: Crystal index too high");
        return ;
      }

      counter.append((format("/EndCapCrystals/EndCapCrystal[%1%]/") % (iCry)).str());
      double h1 = counter.getLength("K_h1") * CLHEP::cm;
      double h2 = counter.getLength("K_h2") * CLHEP::cm;
      double bl1 = counter.getLength("K_bl1") * CLHEP::cm;
      double bl2 = counter.getLength("K_bl2") * CLHEP::cm;
      double tl1 = counter.getLength("K_tl1") * CLHEP::cm;
      double tl2 = counter.getLength("K_tl2") * CLHEP::cm;
      double alpha1 = counter.getAngle("K_alpha1");
      double alpha2 = counter.getAngle("K_alpha2");
      double halflength = counter.getLength("k_HalfLength") * CLHEP::cm;

      // Read and create material
      string strMatCrystal = counter.getString("Material", "Air");
      G4Material* crystalMaterial = geometry::Materials::get(strMatCrystal);


      G4VisAttributes* CrystalVisAtt = new G4VisAttributes(G4Colour(1.0, 1.0, 0.0, 1.0));

      if (strMatCrystal.compare("CsI") == 0) {
        CrystalVisAtt->SetColour(18.0 / 256, 230.0 / 256, 3.0 / 256);
      } else if (strMatCrystal.compare("CsI-Tl") == 0) {
        CrystalVisAtt->SetColour(0.0, 0.5, 1.0);
      } else if (strMatCrystal.compare("LYSO") == 0) {
        CrystalVisAtt->SetColour(0.820, 0.148, 0.1875);
      }

      ////////// add foil dimensions ////////////////////////////
      double fwtrapangle1 = atan(2 * h1 / (bl1 - tl1)); // the smaller angle of the trap
      double fwtrapangle2 = atan(2 * h2 / (bl2 - tl2));
      double foilh1 = h1 + foilthickness;
      double foilh2 = h2 + foilthickness;
      double foiltl1 = tl1 + foilthickness * tan(fwtrapangle1 / 2);
      double foilbl1 = bl1 + foilthickness / tan(fwtrapangle1 / 2);
      double foiltl2 = tl2 + foilthickness * tan(fwtrapangle2 / 2);
      double foilbl2 = foiltl2 + (foilbl1 - foiltl1) * foilh2 / foilh1;

      double foilhalflength = halflength + foilthickness;
      ///////////////////////////////////////////////////////////

      string cryLogiVolName = (format("Enclosure_%1%_Crystal_%2%") % iEnclosure % iCry).str();
      G4Trap* CrystalShape = new G4Trap((format("sCrystal_%1%") % iCry).str().c_str(),
                                        halflength , 0 , 0, h1 ,   bl1, tl1 , alpha1 , h2   , bl2, tl2, alpha2);
      G4LogicalVolume* Crystal = new G4LogicalVolume(CrystalShape, crystalMaterial,
                                                     cryLogiVolName.c_str(),
                                                     0, 0, 0);

      Crystal->SetVisAttributes(CrystalVisAtt);
      Crystal->SetSensitiveDetector(m_sensitive);

      //cout << "CSI volume " << CrystalShape->GetCubicVolume() / CLHEP::cm / CLHEP::cm / CLHEP::cm
      //<< " density " << crystalMaterial->GetDensity() / CLHEP::g * CLHEP::cm * CLHEP::cm * CLHEP::cm << endl;

      /////////////////  add actual foil ///////////////////////////////////////
      G4Trap* Foilout = new G4Trap((format("Foilout_%1%") % iCry).str().c_str(),
                                   foilhalflength , 0 , 0, foilh1,  foilbl1,
                                   foiltl1, alpha1 , foilh2, foilbl2,
                                   foiltl2, alpha2);

      G4Trap* Foilin = new G4Trap((format("solidEclCrystal_%1%") % iCry).str().c_str(),
                                  halflength * avoidov , 0 , 0, h1 * avoidov ,
                                  bl1 * avoidov, tl1 * avoidov , alpha1 , h2 * avoidov,
                                  bl2 * avoidov, tl2 * avoidov, alpha2);
      G4SubtractionSolid* FoilShape = new G4SubtractionSolid((format("sFoil_%1%") % iCry).str().c_str(),
                                                             Foilout, Foilin);

      G4LogicalVolume* Foil = new G4LogicalVolume(FoilShape, foilMaterial,
                                                  (format("Foil_%1%") % iCry).str().c_str(),
                                                  0, 0, 0);

      G4VisAttributes* FoilVisAtt = new G4VisAttributes(G4Colour(0.1, 0.1, 0.1, 0.5));
      Foil->SetVisAttributes(FoilVisAtt);

      //Hide the foils for now...
      Foil->SetVisAttributes(G4VisAttributes::GetInvisible());

      assembly->AddPlacedVolume(Crystal, position);
      assembly->AddPlacedVolume(Foil, position);

      return ;

    }

    void CsiCreator::BuildEnclosure(const GearDir& content, G4AssemblyVolume* assembly, const string side, int iEnclosure)
    {

      string gearPath = "Enclosures/Enclosure";
      int nEnclosures = content.getNumberNodes(gearPath);

      if (iEnclosure > nEnclosures) {
        B2ERROR("Enclosure index too high");
        return ;
      }

      // Build the box (same for all)
      double width  = content.getLength("Enclosures/Width") * CLHEP::cm;
      double length = content.getLength("Enclosures/Length") * CLHEP::cm;
      double depth  = content.getLength("Enclosures/Depth") * CLHEP::cm;
      double thk    = content.getLength("Enclosures/Thickness") * CLHEP::cm;
      double fold   = content.getLength("Enclosures/Fold") * CLHEP::cm;
      double lidthk = content.getLength("Enclosures/LidThickness") * CLHEP::cm;
      double halflength = 15.0 * CLHEP::cm;
      double zshift = 0.5 * length - thk - halflength; /*< Shift of the box along z-axis to make crystal touch the panel **/

      string strMatEnclosure = content.getString("Enclosures/Material", "5052-Alloy");
      G4Material* EnclosureMat = geometry::Materials::get(strMatEnclosure);

      string strMatEncloLid = content.getString("Enclosures/LidMaterial", "5052-Alloy");
      G4Material* EncloLidMat = geometry::Materials::get(strMatEncloLid);

      G4Box* outer   = new G4Box("Outer", 0.5 * width, 0.5 * depth, 0.5 * length);
      G4Box* inner   = new G4Box("Inner", 0.5 * width - thk, 0.5 * depth - thk, 0.5 * length - thk);
      G4Box* opening = new G4Box("Opening", 0.5 * width - fold, 0.5 * depth, 0.5 * length - fold);
      G4Box* lid     = new G4Box("Lid", 0.5 * width, 0.5 * lidthk, 0.5 * length);

      G4ThreeVector translation(0, thk, 0);
      G4Translate3D transform(translation);
      G4SubtractionSolid* enclosureShapeT =  new G4SubtractionSolid("EnclosureShapeT", outer, inner);
      G4SubtractionSolid* enclosureShape =  new G4SubtractionSolid("EnclosureShape",
          enclosureShapeT, opening, transform);

      //Thread the strings
      string enclosurePath = (format("/%1%[%2%]") % gearPath % iEnclosure).str();
      string logiVolName   = (format("%1%Enclosure_%2%") % side % iEnclosure).str();
      string logiLidVolName = (format("%1%EnclosureLid_%2%") % side % iEnclosure).str();

      // Connect the appropriate Gearbox path
      GearDir enclosureContent(content);
      enclosureContent.append(enclosurePath);

      // Create logical volumes
      G4LogicalVolume* logiEnclosure = new G4LogicalVolume(enclosureShape, EnclosureMat, logiVolName, 0, 0, 0);
      G4LogicalVolume* logiEncloLid  = new G4LogicalVolume(lid,  EncloLidMat,  logiLidVolName, 0, 0, 0);

      // Read position
      double PosZ  = enclosureContent.getLength("PosZ") * CLHEP::cm;
      double PosR  = enclosureContent.getLength("PosR") * CLHEP::cm;
      double PosT  = enclosureContent.getAngle("PosT") ;

      // Read Orientation
      double Phi1  = enclosureContent.getAngle("AngPhi1") ;
      double Theta = enclosureContent.getAngle("AngTheta") ;
      double Phi2  = enclosureContent.getAngle("AngPhi2") ;

      //Read position adjustments from nominal
      double AdjX = enclosureContent.getLength("ShiftX") * CLHEP::cm;
      double AdjY = enclosureContent.getLength("ShiftY") * CLHEP::cm;
      double AdjZ = enclosureContent.getLength("ShiftZ") * CLHEP::cm;

      G4Transform3D zsh = G4Translate3D(0, 0, zshift);
      //G4Transform3D invzsh = G4Translate3D(0, 0, -zshift);
      G4Transform3D m1 = G4RotateZ3D(Phi1);
      G4Transform3D m2 = G4RotateY3D(Theta);
      G4Transform3D m3 = G4RotateZ3D(Phi2);
      G4Transform3D position = G4Translate3D(PosR * cos(PosT), PosR * sin(PosT), PosZ);
      G4Transform3D adjust   = G4Translate3D(AdjX, AdjY, AdjZ);
      G4Transform3D lidpos   = G4Translate3D(0, 0.5 * (depth + lidthk), 0);

      G4Transform3D Tr    = position * m3 * m2 * m1; /**< Position of the nominal centre of crystals in the box **/
      G4Transform3D ZshTr    = Tr * zsh; /** < Nominal position of the centre of the box **/
      G4Transform3D ZshTrAdj = adjust * ZshTr;
      G4Transform3D LidTr    = ZshTr * lidpos;
      G4Transform3D LidTrAdj = adjust * LidTr;

      G4VisAttributes* VisAtt = new G4VisAttributes(G4Colour(1.0, 0.5, 0.0, 0.5));
      logiEnclosure->SetVisAttributes(VisAtt);

      G4VisAttributes* LidVisAtt = new G4VisAttributes(G4Colour(0.8, 1.0, 0.4, 0.5));
      logiEncloLid->SetVisAttributes(LidVisAtt);
      //logiEncloLid->SetVisAttributes(G4VisAttributes::GetInvisible());

      B2INFO("CsIBox No. " << iEnclosure << " Nominal pos.   (mm): " << ZshTr.getTranslation());
      B2INFO("             Installed pos. (mm): " << ZshTrAdj.getTranslation());
      B2INFO("             Rotation matrix    : " << ZshTrAdj.getRotation());
      B2INFO(" ");

      assembly->AddPlacedVolume(logiEnclosure, ZshTrAdj);
      assembly->AddPlacedVolume(logiEncloLid, LidTrAdj);


      int nSlots = enclosureContent.getNumberNodes("CrystalInSlot");

      for (int iSlot = 1; iSlot <= nSlots; iSlot++) {
        //Thread the strings
        string slotPath = (format("/Enclosures/Slot[%1%]") % iSlot).str();

        GearDir slotContent(content);
        slotContent.append(slotPath);

        double SlotX = slotContent.getLength("PosX") * CLHEP::cm;
        double SlotY = slotContent.getLength("PosY") * CLHEP::cm;
        double SlotZ = slotContent.getLength("PosZ") * CLHEP::cm;


        G4Transform3D Pos = G4Translate3D(SlotX, SlotY, SlotZ);

        int    CryID  = enclosureContent.getInt((format("/CrystalInSlot[%1%]") % iSlot).str());

        PutCrystal(content, assembly, adjust * Tr * Pos,  iEnclosure, CryID);
      }

      return;

    }

  } // csi namespace
} // Belle2 namespace
