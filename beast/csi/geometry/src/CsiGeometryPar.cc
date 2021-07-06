/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <geometry/Materials.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <beast/csi/geometry/CsiGeometryPar.h>
#include <cmath>
#include <fstream>


using namespace std;
using namespace boost;
using namespace Belle2;
using namespace csi;

#define PI 3.14159265358979323846


CsiGeometryPar* CsiGeometryPar::m_B4CsiGeometryParDB = 0;

CsiGeometryPar* CsiGeometryPar::Instance()
{
  if (!m_B4CsiGeometryParDB) m_B4CsiGeometryParDB = new CsiGeometryPar();
  return m_B4CsiGeometryParDB;
}

CsiGeometryPar::CsiGeometryPar()
{
  clear();
  read();

  PrintAll();
}

CsiGeometryPar::~CsiGeometryPar()
{
  if (m_B4CsiGeometryParDB) {
    delete m_B4CsiGeometryParDB;
    B2INFO("m_B4CsiGeometryParDB deleted ");
  }
}

void CsiGeometryPar::clear()
{
  m_cellID = 0;

  m_Position.clear();
  m_Orientation.clear();
  m_BoxID.clear();
  m_SlotID.clear();
  m_thetaID.clear();
  m_phiID.clear();

}

void CsiGeometryPar::read()
{

  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"CSI\"]/Content/");
  string gearPath = "Enclosures/Enclosure";
  int nEnc = content.getNumberNodes(gearPath);

  int iCell = 0;

  for (int iEnc = 1; iEnc <= nEnc; iEnc++) {
    // Build the box (same for all)
    //double length = content.getLength("Enclosures/Length") * CLHEP::cm;
    //double thk    = content.getLength("Enclosures/Thickness") * CLHEP::cm;
    //double halflength = 15.0 * CLHEP::cm;
    //double zshift = 0.5 * length - thk - halflength; /*< Shift of the box along z-axis (cry touches panel) **/

    string enclosurePath = (boost::format("/%1%[%2%]") % gearPath % iEnc).str();

    // Connect the appropriate Gearbox path
    GearDir enclosureContent(content);
    enclosureContent.append(enclosurePath);

    // Read position
    double PosZ  = enclosureContent.getLength("PosZ") * CLHEP::cm;
    double PosR  = enclosureContent.getLength("PosR") * CLHEP::cm;
    double PosT  = enclosureContent.getAngle("PosT") ;

    // Read Orientation
    double Phi1  = enclosureContent.getAngle("AngPhi1") ;
    double Theta = enclosureContent.getAngle("AngTheta") ;
    double Phi2  = enclosureContent.getAngle("AngPhi2") ;

    //Transform3D zsh = Translate3D(0, 0, zshift);
    Transform3D m1 = RotateZ3D(Phi1);
    Transform3D m2 = RotateY3D(Theta);
    Transform3D m3 = RotateZ3D(Phi2);
    Transform3D position = Translate3D(PosR * cos(PosT), PosR * sin(PosT), PosZ);

    /** Position of the nominal centre of crystals in the box **/
    Transform3D Tr    = position * m3 * m2 * m1;

    int nSlots = enclosureContent.getNumberNodes("CrystalInSlot");
    for (int iSlot = 1; iSlot <= nSlots; iSlot++) {
      iCell++;

      //Thread the strings
      string slotPath = (boost::format("/Enclosures/Slot[%1%]") % iSlot).str();

      GearDir slotContent(content);
      slotContent.append(slotPath);

      double SlotX = slotContent.getLength("PosX") * CLHEP::cm;
      double SlotY = slotContent.getLength("PosY") * CLHEP::cm;
      double SlotZ = slotContent.getLength("PosZ") * CLHEP::cm;
      Transform3D Pos = Translate3D(SlotX, SlotY, SlotZ);

      Transform3D CrystalPos = Tr * Pos;
      RotationMatrix CrystalRot = CrystalPos.getRotation();

      m_Position.push_back(CrystalPos.getTranslation() * 1.0 / CLHEP::cm);
      m_Orientation.push_back(CrystalRot.colZ());

      m_thetaID.push_back(CrystalPos.getTranslation().z() > 0 ? 0 : 1);
      m_phiID.push_back(iCell - 9 * m_thetaID.back());

      m_BoxID.push_back(iEnc - 1);
      m_SlotID.push_back(iSlot - 1);
    }
    //
  }

  //comnvert all that to tvector3's for speed

  vector<ThreeVector>::iterator it;
  for (it = m_Position.begin(); it != m_Position.end(); ++it) {
    m_PositionTV3.push_back(ConvertToTVector3(*it));
  }
  for (it = m_Orientation.begin(); it != m_Orientation.end(); ++it) {
    m_OrientationTV3.push_back(ConvertToTVector3(*it));
  }


}


int CsiGeometryPar::CsiVolNameToCellID(const G4String VolumeName)
{
  int cellID = 0;

  vector< string > partName;
  split(partName, VolumeName, is_any_of("_"));

  int iEnclosure = -1;
  int iCrystal   = -1;
  for (std::vector<string>::iterator it = partName.begin() ; it != partName.end(); ++it) {
    if (equals(*it, "Enclosure")) iEnclosure = boost::lexical_cast<int>(*(it + 1)) - 1;
    else if (equals(*it, "Crystal")) iCrystal = boost::lexical_cast<int>(*(it + 1)) - 1;
  }

  cellID = 3 * iEnclosure + iCrystal;

  if (cellID < 0) B2WARNING("CsiGeometryPar: volume " << VolumeName << " is not a crystal");

  return cellID;
}


G4Material* CsiGeometryPar::GetMaterial(int cid)
{
  int iEnclosure = GetEnclosureID(cid) + 1;
  int iSlot      = GetSlotID(cid) + 1;

  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"CSI\"]/Content/");

  GearDir enclosureContent(content);
  string gearPath = "Enclosures/Enclosure";
  string enclosurePath = (format("/%1%[%2%]") % gearPath % iEnclosure).str();
  enclosureContent.append(enclosurePath);

  string slotName = (format("CrystalInSlot[%1%]") % iSlot).str();
  int iCry = enclosureContent.getInt(slotName);


  GearDir crystalContent(content);
  crystalContent.append((format("/EndCapCrystals/EndCapCrystal[%1%]/") % (iCry)).str());
  string strMatCrystal = crystalContent.getString("Material", "Air");

  return geometry::Materials::get(strMatCrystal);

}


double CsiGeometryPar::GetMaterialProperty(int cid, const char* propertyname)
{
  G4Material* mat = GetMaterial(cid);
  G4MaterialPropertiesTable* properties = mat->GetMaterialPropertiesTable();
  G4MaterialPropertyVector* property = properties->GetProperty(propertyname);

  return property->Value(0);
}

void CsiGeometryPar::Print(int cid, int debuglevel)
{
  B2DEBUG(debuglevel, "Cell ID : " << cid);

  B2DEBUG(debuglevel, "   Position  x : " << GetPosition(cid).x());
  B2DEBUG(debuglevel, "   Position  y : " << GetPosition(cid).y());
  B2DEBUG(debuglevel, "   Position  z : " << GetPosition(cid).z());

  B2DEBUG(debuglevel, "   Orientation  x : " << GetOrientation(cid).x());
  B2DEBUG(debuglevel, "   Orientation  y : " << GetOrientation(cid).y());
  B2DEBUG(debuglevel, "   Orientation  z : " << GetOrientation(cid).z());

  B2DEBUG(debuglevel, "   Material : " << GetMaterial(cid)->GetName());

  B2DEBUG(debuglevel, "   Slow time constatnt : " << GetMaterialProperty(cid, "SLOWTIMECONSTANT"));
  B2DEBUG(debuglevel, "   Fast time constatnt : " << GetMaterialProperty(cid, "FASTTIMECONSTANT"));
  B2DEBUG(debuglevel, "   Light yield : "         << GetMaterialProperty(cid, "SCINTILLATIONYIELD"));

  //GetMaterial(cid)->GetMaterialPropertiesTable()->DumpTable();
}

void CsiGeometryPar::PrintAll(int debuglevel)
{
  for (uint i = 0; i < m_thetaID.size(); i++)
    Print(i, debuglevel);
}
