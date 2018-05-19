/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Makoto Uchida                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/dbobjects/CDCGeometry.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <boost/format.hpp>
using namespace Belle2;
using namespace std;
using namespace boost;

void CDCGeometry::MotherVolume::appendNode(double rmin, double rmax, double z)
{
  m_rmin.push_back(rmin);
  m_rmax.push_back(rmax);
  m_z.push_back(z);
}


CDCGeometry::CDCGeometry(const GearDir& content)
{
  read(content);
}

CDCGeometry::~CDCGeometry()
{
}

void CDCGeometry::read(const GearDir& content)
{
  // Global.
  m_globalOffsetX  = content.getLength("OffsetX");
  m_globalOffsetY  = content.getLength("OffsetY");
  m_globalOffsetZ  = content.getLength("OffsetZ");
  m_globalPhiRotation = content.getAngle("GlobalPhiRotation");
  m_globalOffsetA  = content.getAngle("OffsetA");
  m_globalOffsetB  = content.getAngle("OffsetB");
  m_globalOffsetC  = content.getAngle("OffsetC");

  // Mother volume.
  const int nBound = content.getNumberNodes("MomVol/ZBound");
  for (int iBound = 0; iBound < nBound; iBound++) {
    const double rmin = content.getLength((format("MomVol/ZBound[%1%]/Rmin") % (iBound + 1)).str()) / Unit::mm;
    const double rmax = content.getLength((format("MomVol/ZBound[%1%]/Rmax") % (iBound + 1)).str()) / Unit::mm;
    const double z = content.getLength((format("MomVol/ZBound[%1%]/Z") % (iBound + 1)).str()) / Unit::mm;
    m_mother.appendNode(rmin, rmax, z);
  }


  // Sense layer.
  const int nSLayer = content.getNumberNodes("SLayers/SLayer");

  for (int iSLayer = 0; iSLayer < nSLayer; ++iSLayer) {
    GearDir layerContent(content);
    layerContent.append((format("/SLayers/SLayer[%1%]/") % (iSLayer + 1)).str());
    const double r = layerContent.getLength("Radius");
    const double zfwd = layerContent.getLength("ForwardZ");
    const double zbwd = layerContent.getLength("BackwardZ");
    const double dzfwd = layerContent.getLength("FwdDeltaZ");
    const double dzbwd = layerContent.getLength("BwdDeltaZ");
    const double offset = atof((layerContent.getString("Offset")).c_str());
    const int nWires = atoi((layerContent.getString("NHoles")).c_str()) / 2;
    const double nShifts = atof((layerContent.getString("NShift")).c_str());

    SenseLayer sense(iSLayer, r, zfwd, zbwd,
                     dzfwd, dzbwd, offset, nWires, nShifts);

    m_senseLayers.push_back(sense);
  }

  // Field layer.
  const int nFLayer = content.getNumberNodes("FLayers/FLayer");

  for (int iFLayer = 0; iFLayer < nFLayer; ++iFLayer) {
    GearDir layerContent(content);
    layerContent.append((format("/FLayers/FLayer[%1%]/") % (iFLayer + 1)).str());
    const double r = layerContent.getLength("Radius");
    const double zfwd = layerContent.getLength("ForwardZ");
    const double zbwd = layerContent.getLength("BackwardZ");
    FieldLayer field(iFLayer, r, zfwd, zbwd);
    m_fieldLayers.push_back(field);
  }

  // Endplate.
  const int nEndplates = content.getNumberNodes("Endplates/Endplate");
  for (int i = 0; i < nEndplates; ++i) {

    EndPlate ep(i);

    GearDir epContent(content);
    epContent.append((format("/Endplates/Endplate[%1%]/") % (i + 1)).str());
    const int nEPLayer = epContent.getNumberNodes("EndplateLayer");

    for (int iEPLayer = 0; iEPLayer < nEPLayer; ++iEPLayer) {
      GearDir epLayerContent(epContent);
      epLayerContent.append((format("/EndplateLayer[%1%]/") % (iEPLayer + 1)).str());
      string epName = epLayerContent.getString("Name");
      double rmin = epLayerContent.getLength("InnerR");
      double rmax = epLayerContent.getLength("OuterR");
      double zfwd = epLayerContent.getLength("ForwardZ");
      double zbwd = epLayerContent.getLength("BackwardZ");
      std::string name = "Layer" + to_string(i) + epName + to_string(iEPLayer);

      ep.appendNew(name, iEPLayer, rmin, rmax, zfwd, zbwd);
    }
    m_endplates.push_back(ep);
  }

  // Inner wall.
  const int nInnerWall = content.getNumberNodes("InnerWalls/InnerWall");
  for (int i = 0; i < nInnerWall; ++i) {
    GearDir innerWallContent(content);
    innerWallContent.append((format("/InnerWalls/InnerWall[%1%]/") % (i + 1)).str());

    string sInnerWallID = innerWallContent.getString("@id");
    const string name = "InnerWall_" + sInnerWallID + "_" + innerWallContent.getString("Name");
    const double rmin = innerWallContent.getLength("InnerR");
    const double rmax = innerWallContent.getLength("OuterR");
    const double zfwd = innerWallContent.getLength("ForwardZ");
    const double zbwd = innerWallContent.getLength("BackwardZ");
    const InnerWall wall(name, i, rmin, rmax, zfwd, zbwd);
    m_innerWalls.push_back(wall);
    if (innerWallContent.getString("Name") == "Shield") {
      m_fiducialRmin = rmax;
    }
  }


  // Outer wall.
  const int nOuterWall = content.getNumberNodes("OuterWalls/OuterWall");
  for (int i = 0; i < nOuterWall; ++i) {
    GearDir outerWallContent(content);
    outerWallContent.append((format("/OuterWalls/OuterWall[%1%]/") % (i + 1)).str());

    string sOuterWallID = outerWallContent.getString("@id");
    const string name = "OuterWall_" + sOuterWallID + "_" + outerWallContent.getString("Name");
    const double rmin = outerWallContent.getLength("InnerR");
    const double rmax = outerWallContent.getLength("OuterR");
    const double zfwd = outerWallContent.getLength("ForwardZ");
    const double zbwd = outerWallContent.getLength("BackwardZ");
    const OuterWall wall(name, i, rmin, rmax, zfwd, zbwd);
    m_outerWalls.push_back(wall);
    if (outerWallContent.getString("Name") == "Shield") {
      m_fiducialRmax = rmin;
    }
  }


  // Electronics board.
  const int nEB = content.getNumberNodes("ElectronicsBoards/ElectronicsBoard");
  for (int iEB = 0; iEB < nEB; ++iEB) {
    GearDir ebContent(content);
    ebContent.append((format("/ElectronicsBoards/ElectronicsBoard[%1%]/") % (iEB + 1)).str());

    const double rmin = ebContent.getLength("EBInnerR");
    const double rmax = ebContent.getLength("EBOuterR");
    const double zfwd = ebContent.getLength("EBForwardZ");
    const double zbwd = ebContent.getLength("EBBackwardZ");
    Frontend frontend(iEB, rmin, rmax, zfwd, zbwd);
    m_frontends.push_back(frontend);
  }

  // Neutron shield.
  const int nShields = content.getNumberNodes("Shields/Shield");
  for (int i = 0; i < nShields; ++i) {
    GearDir neuContent(content);
    neuContent.append((format("/Shields/Shield[%1%]/") % (i + 1)).str());

    const double rmin1 = neuContent.getLength("InnerR1");
    const double rmin2 = neuContent.getLength("InnerR2");
    const double rmax1 = neuContent.getLength("OuterR1");
    const double rmax2 = neuContent.getLength("OuterR2");
    const double thick = neuContent.getLength("Thickness");
    const double z = neuContent.getLength("PosZ");
    NeutronShield shield(i, rmin1, rmin2, rmax1, rmax2, thick, z);
    m_neutronShields.push_back(shield);
  }

  // Support structure.
  const int nCovers = content.getNumberNodes("Covers/Cover");
  for (int i = 0; i < nCovers; ++i) {
    GearDir coverContent(content);
    coverContent.append((format("/Covers/Cover[%1%]/") % (i + 1)).str());

    const int coverID = atoi((coverContent.getString("@id")).c_str());
    const double rmin1 = coverContent.getLength("InnerR1");
    const double rmin2 = coverContent.getLength("InnerR2");
    const double rmax1 = coverContent.getLength("OuterR1");
    const double rmax2 = coverContent.getLength("OuterR2");
    const double thick = coverContent.getLength("Thickness");
    const double z = coverContent.getLength("PosZ");
    Cover cover(coverID, rmin1, rmin2, rmax1, rmax2, thick, z);
    m_covers.push_back(cover);
  }

  const int nCover2s = content.getNumberNodes("Covers/Cover2");
  for (int i = 0; i < nCover2s; ++i) {
    GearDir cover2Content(content);
    cover2Content.append((format("/Covers/Cover2[%1%]/") % (i + 1)).str());

    const int cover2ID = atoi((cover2Content.getString("@id")).c_str());
    const double rmin = cover2Content.getLength("InnerR");
    const double rmax = cover2Content.getLength("OuterR");
    const double phis = cover2Content.getLength("StartPhi");
    const double dphi = cover2Content.getLength("DeltaPhi");
    const double thick = cover2Content.getLength("Thickness");
    const double z = cover2Content.getLength("PosZ");
    Cover2 cover2(cover2ID, rmin, rmax, phis, dphi, thick, z);
    m_cover2s.push_back(cover2);
  }

  const int nRibs = content.getNumberNodes("Covers/Rib");
  for (int i = 0; i < nRibs; ++i) {
    GearDir ribContent(content);
    ribContent.append((format("/Covers/Rib[%1%]/") % (i + 1)).str());

    const int ribID = atoi((ribContent.getString("@id")).c_str());
    const double length = ribContent.getLength("Length");
    const double width = ribContent.getLength("Width");
    const double thick = ribContent.getLength("Thickness");
    const double rotx = ribContent.getLength("RotX");
    const double roty = ribContent.getLength("RotY");
    const double rotz = ribContent.getLength("RotZ");
    const double x = ribContent.getLength("PosX");
    const double y = ribContent.getLength("PosY");
    const double z = ribContent.getLength("PosZ");
    const int offset = atoi((ribContent.getString("Offset")).c_str());
    const int ndiv = atoi((ribContent.getString("NDiv")).c_str());
    Rib rib(ribID, length, width, thick, rotx, roty, rotz, x, y, z, offset, ndiv);
    m_ribs.push_back(rib);
  }

  const int nRib2s = content.getNumberNodes("Covers/Rib2");
  for (int i = 0; i < nRib2s; ++i) {
    GearDir rib2Content(content);
    rib2Content.append((format("/Covers/Rib2[%1%]/") % (i + 1)).str());

    const int rib2ID = atoi((rib2Content.getString("@id")).c_str());
    const double length = rib2Content.getLength("Length");
    const double width = rib2Content.getLength("Width");
    const double thick = rib2Content.getLength("Thickness");
    const double width2 = rib2Content.getLength("Width2");
    const double thick2 = rib2Content.getLength("Thickness2");
    const double rotx = rib2Content.getLength("RotX");
    const double roty = rib2Content.getLength("RotY");
    const double rotz = rib2Content.getLength("RotZ");
    const double x = rib2Content.getLength("PosX");
    const double y = rib2Content.getLength("PosY");
    const double z = rib2Content.getLength("PosZ");
    const int ndiv = atoi((rib2Content.getString("NDiv")).c_str());
    Rib2 rib2(rib2ID, length, width, thick, width2, thick2, rotx, roty, rotz,
              x, y, z, ndiv);
    m_rib2s.push_back(rib2);
  }

  const int nRib3s = content.getNumberNodes("Covers/Rib3");
  for (int i = 0; i < nRib3s; ++i) {
    GearDir rib3Content(content);
    rib3Content.append((format("/Covers/Rib3[%1%]/") % (i + 1)).str());

    const int rib3ID = atoi((rib3Content.getString("@id")).c_str());
    const double length = rib3Content.getLength("Length");
    const double width = rib3Content.getLength("Width");
    const double thick = rib3Content.getLength("Thickness");
    const double r = rib3Content.getLength("HoleR");
    const double x = rib3Content.getLength("PosX");
    const double y = rib3Content.getLength("PosY");
    const double z = rib3Content.getLength("PosZ");
    const double rx = rib3Content.getLength("HoleX");
    const double ry = rib3Content.getLength("HoleY");
    const double rz = rib3Content.getLength("HoleZ");
    const int offset = atoi((rib3Content.getString("Offset")).c_str());
    const int ndiv = atoi((rib3Content.getString("NDiv")).c_str());
    Rib3 rib3(rib3ID, length, width, thick, r, x, y, z, rx, ry, rz, offset, ndiv);
    m_rib3s.push_back(rib3);
  }

  const int nRib4s = content.getNumberNodes("Covers/Rib4");
  for (int i = 0; i < nRib4s; ++i) {
    GearDir rib4Content(content);
    rib4Content.append((format("/Covers/Rib4[%1%]/") % (i + 1)).str());

    const int rib4ID = atoi((rib4Content.getString("@id")).c_str());
    const double length = rib4Content.getLength("Length");
    const double width = rib4Content.getLength("Width");
    const double thick = rib4Content.getLength("Thickness");
    const double length2 = rib4Content.getLength("Length2");
    const double width2 = rib4Content.getLength("Width2");
    const double thick2 = rib4Content.getLength("Thickness2");
    const double x = rib4Content.getLength("PosX");
    const double y = rib4Content.getLength("PosY");
    const double z = rib4Content.getLength("PosZ");
    const double x2 = rib4Content.getLength("HoleX");
    const double y2 = rib4Content.getLength("HoleY");
    const double z2 = rib4Content.getLength("HoleZ");
    const int offset = atoi((rib4Content.getString("Offset")).c_str());
    const int ndiv = atoi((rib4Content.getString("NDiv")).c_str());
    Rib4 rib4(rib4ID, length, width, thick, length2, width2, thick2, x, y, z, x2, y2, z2, offset, ndiv);
    m_rib4s.push_back(rib4);
  }


  // Sense wire.
  GearDir senseWire(content);
  senseWire.append("/SenseWire/");
  m_senseWireDiameter = senseWire.getLength("Diameter");
  //  B2INFO("m_senseWireDiameter= " << m_senseWireDiameter);
  m_senseWireNumbers = atoi((senseWire.getString("Number")).c_str());

  // Field wire.
  GearDir fieldWire(content);
  fieldWire.append("/FieldWire/");
  m_fieldWireDiameter = fieldWire.getLength("Diameter");
  //  B2INFO("m_fieldWireDiameter= " << m_fieldWireDiameter);
  m_fieldWireNumbers = atoi((fieldWire.getString("Number")).c_str());


  // Feedthrough.
  m_feedThroughLength = content.getLength("/FeedThrough/Length");
  //  B2INFO("m_feedThroughLength= " << m_feedThroughLength);

  // Get control switch for gas and wire material definition
  m_clockFrequency  = content.getDouble("ClockFrequencyForTDC");
  m_nominalSpaceResolution = content.getLength("SenseWire/SpaceResol");

}
