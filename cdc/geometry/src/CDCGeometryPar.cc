/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Martin Heck                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <cdc/geometry/CDCGeometryPar.h>

#include <cmath>
#include <boost/format.hpp>
#include <iostream>
#include <fstream>

using namespace std;
using namespace boost;
using namespace Belle2;
using namespace CDC;

CDCGeometryPar* CDCGeometryPar::m_B4CDCGeometryParDB = 0;

CDCGeometryPar& CDCGeometryPar::Instance()
{
  if (!m_B4CDCGeometryParDB) m_B4CDCGeometryParDB = new CDCGeometryPar();
  return *m_B4CDCGeometryParDB;
}

CDCGeometryPar::CDCGeometryPar()
{
  clear();
  read();
}

CDCGeometryPar::~CDCGeometryPar()
{
}

void CDCGeometryPar::clear()
{
  m_motherInnerR = 0.;
  m_motherOuterR = 0.;
  m_motherLength = 0.;
  // T.Hara added to define the CDC mother volume (temporal)
  for (unsigned i = 0; i < 7; i++) {
    m_momZ[i] = 0.;
    m_momRmin[i] = 0.;
  }
  //
  m_version = "unknown";
  m_nSLayer = 0;
  m_nFLayer = 0;
  m_senseWireDiameter = 0.0;
  m_fieldWireDiameter = 0.0;
  for (unsigned i = 0; i < 4; i++) {
    m_rWall[i] = 0;
    for (unsigned j = 0; j < 2; j++)
      m_zWall[i][j] = 0;
  }
  for (unsigned i = 0; i < MAX_N_SLAYERS; i++) {
    m_rSLayer[i] = 0;
    m_zSForwardLayer[i] = 0;
    m_zSBackwardLayer[i] = 0;
    m_cellSize[i] = 0;
    m_nWires[i] = 0;
    m_offSet[i] = 0;
    m_nShifts[i] = 0;
  }
  for (unsigned i = 0; i < MAX_N_FLAYERS; i++) {
    m_rFLayer[i] = 0;
    m_zFForwardLayer[i] = 0;
    m_zFBackwardLayer[i] = 0;
  }
}

void CDCGeometryPar::read()
{
  // Get the version of cdc geometry parameters
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"CDC\"]/Content/");
  //m_version = gbxParams.getString("Version");

  // Get Gearbox parameters
  //gbxParams = Gearbox::Instance().getContent("CDC");

  //------------------------------
  // Get CDC geometry parameters
  //------------------------------
  GearDir innerWallParams(content, "InnerWalls/");
  m_motherInnerR = innerWallParams.getLength("InnerWall[3]/InnerR");

  GearDir outerWallParams(content, "OuterWalls/");
  m_motherOuterR = outerWallParams.getLength("OuterWall[6]/OuterR");

  int nBound = content.getNumberNodes("MomVol/ZBound");
  // Loop over to get the parameters of each boundary
  for (int iBound = 0; iBound < nBound; iBound++) {
    m_momZ[iBound] = content.getLength((format("MomVol/ZBound[%1%]/Z") % (iBound + 1)).str()) / Unit::mm;
    m_momRmin[iBound] = content.getLength((format("MomVol/ZBound[%1%]/Rmin") % (iBound + 1)).str()) / Unit::mm;
  }

  GearDir coverParams(content, "Covers/");
  double R1 = coverParams.getLength("Cover[2]/InnerR1");
  double R2 = coverParams.getLength("Cover[2]/InnerR2");
  double angle = coverParams.getLength("Cover[2]/Angle");
  double thick = coverParams.getLength("Cover[2]/Thickness");
  double zpos = coverParams.getLength("Cover[2]/PosZ");
  //  Commented by M. U. June 2nd, 2013
  //  double length1;
  //  if (angle != 0) length1 = fabs(zpos - (R2 - R1) / tan(angle));
  //  else length1 = fabs(zpos);
  R1 = coverParams.getLength("Cover[4]/InnerR1");
  R2 = coverParams.getLength("Cover[4]/InnerR2");
  angle = coverParams.getLength("Cover[4]/Angle");
  zpos = coverParams.getLength("Cover[4]/PosZ");
  double length2;
  if (angle != 0) length2 = fabs(zpos + (R2 - R1) / tan(angle));
  else length2 = fabs(zpos);
  m_motherLength = (length2 + thick) * 2.;
  //m_motherLength = length1 + length2 + 2 * thick;

  // Get inner wall parameters
  m_rWall[0]    = innerWallParams.getLength("InnerWall[3]/InnerR");
  m_zWall[0][0] = innerWallParams.getLength("InnerWall[1]/BackwardZ");
  m_zWall[0][1] = innerWallParams.getLength("InnerWall[1]/ForwardZ");

  m_rWall[1] = innerWallParams.getLength("InnerWall[1]/OuterR");
  m_zWall[1][0] = innerWallParams.getLength("InnerWall[1]/BackwardZ");
  m_zWall[1][1] = innerWallParams.getLength("InnerWall[1]/ForwardZ");

  // Get outer wall parameters
  m_rWall[2] = outerWallParams.getLength("OuterWall[1]/InnerR");
  m_zWall[2][0] = outerWallParams.getLength("OuterWall[1]/BackwardZ");
  m_zWall[2][1] = outerWallParams.getLength("OuterWall[1]/ForwardZ");

  m_rWall[3] = outerWallParams.getLength("OuterWall[2]/OuterR");
  m_zWall[3][0] = outerWallParams.getLength("OuterWall[1]/BackwardZ");
  m_zWall[3][1] = outerWallParams.getLength("OuterWall[1]/ForwardZ");

  // Get sense layers parameters
  GearDir gbxParams(content);
  int nSLayer = gbxParams.getNumberNodes("SLayers/SLayer");
  m_nSLayer = nSLayer;

  // Loop over all sense layers
  for (int iSLayer = 0; iSLayer < nSLayer; iSLayer++) {
    int layerId = atoi((gbxParams.getString((format("SLayers/SLayer[%1%]/@id") % (iSLayer + 1)).str())).c_str());
    m_rSLayer[layerId] = gbxParams.getLength((format("SLayers/SLayer[%1%]/Radius") % (iSLayer + 1)).str());
    m_zSBackwardLayer[layerId] = gbxParams.getLength((format("SLayers/SLayer[%1%]/BackwardZ") % (iSLayer + 1)).str());
    m_zSForwardLayer[layerId] = gbxParams.getLength((format("SLayers/SLayer[%1%]/ForwardZ") % (iSLayer + 1)).str());
    m_nWires[layerId] = atoi((gbxParams.getString((format("SLayers/SLayer[%1%]/NHoles") % (iSLayer + 1)).str())).c_str()) / 2;
    m_nShifts[layerId] = atoi((gbxParams.getString((format("SLayers/SLayer[%1%]/NShift") % (iSLayer + 1)).str())).c_str());
    m_offSet[layerId] = atof((gbxParams.getString((format("SLayers/SLayer[%1%]/Offset") % (iSLayer + 1)).str())).c_str());
    m_cellSize[layerId] = 2 * M_PI * m_rSLayer[layerId] / (double) m_nWires[layerId];
  }

  // Get field layers parameters
  int nFLayer = gbxParams.getNumberNodes("FLayers/FLayer");
  m_nFLayer = nFLayer;

  // Loop over all field layers
  for (int iFLayer = 0; iFLayer < nFLayer; iFLayer++) {
    int layerId = atoi((gbxParams.getString((format("FLayers/FLayer[%1%]/@id") % (iFLayer + 1)).str())).c_str());
    m_rFLayer[layerId] = gbxParams.getLength((format("FLayers/FLayer[%1%]/Radius") % (iFLayer + 1)).str());
    m_zFBackwardLayer[layerId] = gbxParams.getLength((format("FLayers/FLayer[%1%]/BackwardZ") % (iFLayer + 1)).str());
    m_zFForwardLayer[layerId] = gbxParams.getLength((format("FLayers/FLayer[%1%]/ForwardZ") % (iFLayer + 1)).str());
  }

  // Get sense wire diameter
  m_senseWireDiameter = gbxParams.getLength("SenseWire/Diameter");

  // Get field wire diameter
  m_fieldWireDiameter = gbxParams.getLength("FieldWire/Diameter");

  //Print();

}


void CDCGeometryPar::Print() const
{}

const TVector3 CDCGeometryPar::wireForwardPosition(int layerID, int cellID) const
{
  const unsigned nWires = m_nWires[layerID];
  double offset = m_offSet[layerID];
  //...Offset modification to be aligned to axial at z=0...
  const double phiSize = 2 * M_PI / double(nWires);
  /*{
    const double phiF = phiSize * offset
                        + phiSize * 0.5 * double(m_nShifts[layerID]);
    const double phiB = phiSize * offset;
    const TVector3 f(m_rSLayer[layerID] * cos(phiF), m_rSLayer[layerID] * sin(phiF), m_zSForwardLayer[layerID]);
    const TVector3 b(m_rSLayer[layerID] * cos(phiB), m_rSLayer[layerID] * sin(phiB), m_zSBackwardLayer[layerID]);

    const TVector3 v = f - b;
    const TVector3 u = v.Unit();
    const double beta = (0 - b.z()) / u.z();
    const TVector3 p = b + beta * u;
    double phi0 = - atan2(p.y(), p.x());
    offset += phi0 / (2 * M_PI / double(nWires));
  }*/

  const double phiF = phiSize * (double(cellID) + offset)
                      + phiSize * 0.5 * double(m_nShifts[layerID]);
  const double phiB = phiSize * (double(cellID) + offset);
  const TVector3 f(m_rSLayer[layerID] * cos(phiF), m_rSLayer[layerID] * sin(phiF), m_zSForwardLayer[layerID]);
  const TVector3 b(m_rSLayer[layerID] * cos(phiB), m_rSLayer[layerID] * sin(phiB), m_zSBackwardLayer[layerID]);

  return f;
}

const TVector3 CDCGeometryPar::wireBackwardPosition(int layerID, int cellID) const
{
  const unsigned nWires = m_nWires[layerID];

  double offset = m_offSet[layerID];
  //...Offset modification to be aligned to axial at z=0...
  const double phiSize = 2 * M_PI / double(nWires);
  /*{
    const double phiF = phiSize * offset
                        + phiSize * 0.5 * double(m_nShifts[layerID]);
    const double phiB = phiSize * offset;
    const TVector3 f(m_rSLayer[layerID] * cos(phiF), m_rSLayer[layerID] * sin(phiF), m_zSForwardLayer[layerID]);
    const TVector3 b(m_rSLayer[layerID] * cos(phiB), m_rSLayer[layerID] * sin(phiB), m_zSBackwardLayer[layerID]);

    const TVector3 v = f - b;
    const TVector3 u = v.Unit();
    const double beta = (0 - b.z()) / u.z();
    const TVector3 p = b + beta * u;
    double phi0 = - atan2(p.y(), p.x());
    offset += phi0 / (2 * M_PI / double(nWires));
  }*/

  const double phiF = phiSize * (double(cellID) + offset)
                      + phiSize * 0.5 * double(m_nShifts[layerID]);
  const double phiB = phiSize * (double(cellID) + offset);
  const TVector3 f(m_rSLayer[layerID] * cos(phiF), m_rSLayer[layerID] * sin(phiF), m_zSForwardLayer[layerID]);
  const TVector3 b(m_rSLayer[layerID] * cos(phiB), m_rSLayer[layerID] * sin(phiB), m_zSBackwardLayer[layerID]);

  return b;
}

const double* CDCGeometryPar::innerRadiusWireLayer() const
{
  static double IRWL[MAX_N_SLAYERS] = {0};

  IRWL[0] = outerRadiusInnerWall();
  for (unsigned i = 1; i < nWireLayers(); i++)
    //IRWL[i] = (m_rSLayer[i - 1] + m_rSLayer[i]) / 2.;
    IRWL[i] = m_rFLayer[i - 1];

  return IRWL;
}

const double* CDCGeometryPar::outerRadiusWireLayer() const
{
  static double ORWL[MAX_N_SLAYERS] = {0};

  ORWL[nWireLayers() - 1] = innerRadiusOuterWall();
  for (unsigned i = 0; i < nWireLayers() - 1; i++)
    //ORWL[i] = (m_rSLayer[i] + m_rSLayer[i + 1]) / 2.;
    ORWL[i] = m_rFLayer[i];

  return ORWL;
}

unsigned CDCGeometryPar::cellId(unsigned layerId, const TVector3& position) const
{
  const unsigned nWires = m_nWires[layerId];

  double offset = m_offSet[layerId];
  //...Offset modification to be aligned to axial at z=0...
  const double phiSize = 2 * M_PI / double(nWires);
  /*{
    const double phiF = phiSize * offset
                        + phiSize * 0.5 * double(m_nShifts[layerId]);
    const double phiB = phiSize * offset;
    const TVector3 f(m_rSLayer[layerId] * cos(phiF), m_rSLayer[layerId] * sin(phiF), m_zSForwardLayer[layerId]);
    const TVector3 b(m_rSLayer[layerId] * cos(phiB), m_rSLayer[layerId] * sin(phiB), m_zSBackwardLayer[layerId]);

    const TVector3 v = f - b;
    const TVector3 u = v.Unit();
    const double beta = (0 - b.z()) / u.z();
    const TVector3 p = b + beta * u;
    double phi0 = - atan2(p.y(), p.x());
    offset += phi0 / (2 * M_PI / double(nWires));
  }*/

  unsigned j = 0;
  for (unsigned i = 0; i < 1; i++) {
    const double phiF = phiSize * (double(i) + offset)
                        + phiSize * 0.5 * double(m_nShifts[layerId]);
    const double phiB = phiSize * (double(i) + offset);
    const TVector3 f(m_rSLayer[layerId] * cos(phiF), m_rSLayer[layerId] * sin(phiF), m_zSForwardLayer[layerId]);
    const TVector3 b(m_rSLayer[layerId] * cos(phiB), m_rSLayer[layerId] * sin(phiB), m_zSBackwardLayer[layerId]);
    const TVector3 v = f - b;
    const TVector3 u = v.Unit();
    const double beta = (position.z() - b.z()) / u.z();
    const TVector3 p = b + beta * u;
    double dPhi = std::atan2(position.y(), position.x())
                  - std::atan2(p.y(), p.x())
                  + phiSize / 2.;
    while (dPhi < 0) dPhi += (2. * M_PI);
    j = int(dPhi / phiSize);
    while (j > nWires) j -= nWires;
  }

  return j;
}

void CDCGeometryPar::generateXML(const string& of)
{
  //...Open xml file...
  std::ofstream ofs(of.c_str(), std::ios::out);
  if (! ofs) {
    B2ERROR("CDCGeometryPar::read !!! can not open file : "
            << of);
  }
  ofs << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"                                                                                                << endl
      << "<Subdetector type=\"CDC\">"                                                                                                                << endl
      << "  <Name>CDC BelleII </Name>"                                                                                                               << endl
      << "  <Description>CDC geometry parameters</Description>"                                                                                      << endl
      << "  <Version>0</Version>"                                                                                                                    << endl
      << "  <GeoCreator>CDCBelleII</GeoCreator>"                                                                                                     << endl
      << "  <Content>"                                                                                                                               << endl
      << "    <Rotation desc=\"Rotation of the whole cdc detector (should be the same as beampipe)\" unit=\"mrad\">0.0</Rotation>"                   << endl
      << "    <OffsetZ desc=\"The offset of the whole cdc in z with respect to the IP (should be the same as beampipe)\" unit=\"mm\">0.0</OffsetZ>"  << endl
      << "    <Material>CDCGas</Material>"                                                                                                           << endl
      << endl;

  ofs << "    <SLayers>" << endl;

  for (int i = 0; i < m_nSLayer; i++) {
    ofs << "      <SLayer id=\"" << i << "\">" << endl;
    ofs << "        <Radius desc=\"Radius of wires in this layer\" unit=\"mm\">" << senseWireR(i) << "</Radius>" << endl;
    ofs << "        <BackwardZ desc=\"z position of this wire layer at backward endplate\" unit=\"mm\">" << senseWireBZ(i) << "</BackwardZ>" << endl;
    ofs << "        <ForwardZ desc=\"z position of this wire layer at forward endplate\" unit=\"mm\">" << senseWireFZ(i) << "</ForwardZ>" << endl;
//    ofs << "        <BackwardPhi desc=\"azimuth angle of the first wire in this layer at backward endplate\" unit=\"rad\">" << wireBackwardPosition(i).phi() << "</BackwardPhi>" << endl;
//    ofs << "        <ForwardPhi desc=\"azimuth angle of the first wire in this layer at forward endplate\" unit=\"rad\">" << wireForwardPosition(i).phi() << "</ForwardPhi>" << endl;
    ofs << "        <NHoles desc=\"the number of holes in this layer, 2*(cell number)\">" << nWiresInLayer(i) * 2 << "</NHoles>" << endl;
    ofs << "        <NShift desc=\"the shifted hole number of each wire in this layer\">" << nShifts(i) << "</NShift>" << endl;
    ofs << "        <Offset desc=\"wire offset in phi direction at endplate\">" << m_offSet[i] << "</Offset>" << endl;
    ofs << "      </SLayer>" << endl;
  }

  ofs << "    </SLayers>" << endl;
  ofs << "    <FLayers>" << endl;

  for (int i = 0; i < m_nFLayer; i++) {
    ofs << "      <FLayer id=\"" << i << "\">" << endl;
    ofs << "        <Radius desc=\"Radius of field wires in this layer\" unit=\"mm\">" << fieldWireR(i) << "</Radius>" << endl;
    ofs << "        <BackwardZ desc=\"z position of this field wire layer at backward endplate\" unit=\"mm\">" << fieldWireBZ(i) << "</BackwardZ>" << endl;
    ofs << "        <ForwardZ desc=\"z position of this field wire layer at forward endplate\" unit=\"mm\">" << fieldWireFZ(i) << "</ForwardZ>" << endl;
    ofs << "      </FLayer>" << endl;
  }

  ofs << "    </FLayers>" << endl;

  ofs << "    <InnerWall name=\"InnerWall\">" << endl;
  ofs << "      <InnerR desc=\"Inner radius\" unit=\"mm\">" << innerRadiusInnerWall() << "</InnerR>" << endl;
  ofs << "      <OuterR desc=\"Outer radius\" unit=\"mm\">" << outerRadiusInnerWall() << "</OuterR>" << endl;
  ofs << "      <BackwardZ desc=\"z position at backward endplate\" unit=\"mm\">" << m_zWall[0][0] << "</BackwardZ>" << endl;
  ofs << "      <ForwardZ desc=\"z position at forward endplate\" unit=\"mm\">" << m_zWall[0][1] << "</ForwardZ>" << endl;
  ofs << "    </InnerWall>" << endl;

  ofs << "    <OuterWall name=\"OuterWall\">" << endl;
  ofs << "      <InnerR desc=\"Inner radius\" unit=\"mm\">" << innerRadiusOuterWall() << "</InnerR>" << endl;
  ofs << "      <OuterR desc=\"Outer radius\" unit=\"mm\">" << outerRadiusOuterWall() << "</OuterR>" << endl;
  ofs << "      <BackwardZ desc=\"z position at backward endplate\" unit=\"mm\">" << m_zWall[2][0] << "</BackwardZ>" << endl;
  ofs << "      <ForwardZ desc=\"z position at forward endplate\" unit=\"mm\">" << m_zWall[2][1] << "</ForwardZ>" << endl;
  ofs << "    </OuterWall>" << endl;

  ofs << "  </Content>"                                         << endl
      << "</Subdetector>"                                       << endl;
}
