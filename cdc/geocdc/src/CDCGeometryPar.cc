/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/gearbox/Gearbox.h>
#include <geometry/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <cdc/geocdc/CDCGeometryPar.h>

#include <cmath>
#include <boost/format.hpp>
#include <iostream>
#include <fstream>

using namespace std;
using namespace boost;
using namespace Belle2;

CDCGeometryPar* CDCGeometryPar::p_B4CDCGeometryParDB = 0;

CDCGeometryPar* CDCGeometryPar::Instance(void)
{
  if (!p_B4CDCGeometryParDB) p_B4CDCGeometryParDB = new CDCGeometryPar();
  return p_B4CDCGeometryParDB;
}

CDCGeometryPar::CDCGeometryPar()
{
  clear();
  read();
}

CDCGeometryPar::~CDCGeometryPar()
{
}

void
CDCGeometryPar::clear(void)
{
  _version = "unknown";
  _nSLayer = 0;
  _nFLayer = 0;
  _senseWireDiameter = 0.0;
  _fieldWireDiameter = 0.0;
  for (unsigned i = 0; i < 4; i++) {
    _rWall[i] = 0;
    for (unsigned j = 0; j < 2; j++)
      _zWall[i][j] = 0;
  }
  for (unsigned i = 0; i < MAX_N_SLAYERS; i++) {
    _rSLayer[i] = 0;
    _zSForwardLayer[i] = 0;
    _zSBackwardLayer[i] = 0;
    _cellSize[i] = 0;
    _nWires[i] = 0;
    _offSet[i] = 0;
    _nShifts[i] = 0;
    for (unsigned j = 0; j < MAX_N_SCELLS; j++) {
      _wireForwardPosition[i][j].SetX(0);
      _wireForwardPosition[i][j].SetY(0);
      _wireForwardPosition[i][j].SetZ(0);
      _wireBackwardPosition[i][j].SetX(0);
      _wireBackwardPosition[i][j].SetY(0);
      _wireBackwardPosition[i][j].SetZ(0);
    }
  }
  for (unsigned i = 0; i < MAX_N_FLAYERS; i++) {
    _rFLayer[i] = 0;
    _zFForwardLayer[i] = 0;
    _zFBackwardLayer[i] = 0;
  }
}

void CDCGeometryPar::read()
{

  // Get Gearbox parameters
  GearDir gbxParams = Gearbox::Instance().getContent("CDC");

  //------------------------------
  // Get CDC geometry parameters
  //------------------------------

  // Get inner wall parameters
  _rWall[0]    = gbxParams.getParamLength("InnerWalls/InnerWall[3]/InnerR");
  _zWall[0][0] = gbxParams.getParamLength("InnerWalls/InnerWall[1]/BackwardZ");
  _zWall[0][1] = gbxParams.getParamLength("InnerWalls/InnerWall[1]/ForwardZ");

  _rWall[1] = gbxParams.getParamLength("InnerWalls/InnerWall[1]/OuterR");
  _zWall[1][0] = gbxParams.getParamLength("InnerWalls/InnerWall[1]/BackwardZ");
  _zWall[1][1] = gbxParams.getParamLength("InnerWalls/InnerWall[1]/ForwardZ");

  // Get outer wall parameters
  _rWall[2] = gbxParams.getParamLength("OuterWalls/OuterWall[1]/InnerR");
  _zWall[2][0] = gbxParams.getParamLength("OuterWalls/OuterWall[1]/BackwardZ");
  _zWall[2][1] = gbxParams.getParamLength("OuterWalls/OuterWall[1]/ForwardZ");

  _rWall[3] = gbxParams.getParamLength("OuterWalls/OuterWall[2]/OuterR");
  _zWall[3][0] = gbxParams.getParamLength("OuterWalls/OuterWall[1]/BackwardZ");
  _zWall[3][1] = gbxParams.getParamLength("OuterWalls/OuterWall[1]/ForwardZ");

  // Get sense layers parameters
  int nSLayer = gbxParams.getNumberNodes("SLayers/SLayer");
  _nSLayer = nSLayer;

  // Loop over all sense layers
  for (int iSLayer = 0; iSLayer < nSLayer; iSLayer++) {
    int layerId = atoi((gbxParams.getParamString((format("SLayers/SLayer[%1%]/@id") % (iSLayer + 1)).str())).c_str());
    _rSLayer[layerId] = gbxParams.getParamLength((format("SLayers/SLayer[%1%]/Radius") % (iSLayer + 1)).str());
    _zSBackwardLayer[layerId] = gbxParams.getParamLength((format("SLayers/SLayer[%1%]/BackwardZ") % (iSLayer + 1)).str());
    _zSForwardLayer[layerId] = gbxParams.getParamLength((format("SLayers/SLayer[%1%]/ForwardZ") % (iSLayer + 1)).str());
    _nWires[layerId] = atoi((gbxParams.getParamString((format("SLayers/SLayer[%1%]/NHoles") % (iSLayer + 1)).str())).c_str()) / 2;
    _nShifts[layerId] = atoi((gbxParams.getParamString((format("SLayers/SLayer[%1%]/NShift") % (iSLayer + 1)).str())).c_str());
    _offSet[layerId] = atof((gbxParams.getParamString((format("SLayers/SLayer[%1%]/Offset") % (iSLayer + 1)).str())).c_str());
    _cellSize[layerId] = 2 * M_PI * _rSLayer[layerId] / (double) _nWires[layerId];
  }

  // Get field layers parameters
  int nFLayer = gbxParams.getNumberNodes("FLayers/FLayer");
  _nFLayer = nFLayer;

  // Loop over all field layers
  for (int iFLayer = 0; iFLayer < nFLayer; iFLayer++) {
    int layerId = atoi((gbxParams.getParamString((format("FLayers/FLayer[%1%]/@id") % (iFLayer + 1)).str())).c_str());
    _rFLayer[layerId] = gbxParams.getParamLength((format("FLayers/FLayer[%1%]/Radius") % (iFLayer + 1)).str());
    _zFBackwardLayer[layerId] = gbxParams.getParamLength((format("FLayers/FLayer[%1%]/BackwardZ") % (iFLayer + 1)).str());
    _zFForwardLayer[layerId] = gbxParams.getParamLength((format("FLayers/FLayer[%1%]/ForwardZ") % (iFLayer + 1)).str());
  }

  // Get sense wire diameter
  _senseWireDiameter = gbxParams.getParamLength("SenseWire/Diameter");

  // Get field wire diameter
  _fieldWireDiameter = gbxParams.getParamLength("FieldWire/Diameter");

  //Print();

}

void CDCGeometryPar::Print(void) const
{
}

TVector3 CDCGeometryPar::wireForwardPosition(int layerID, int cellID)
{
  const unsigned nWires = _nWires[layerID];
  double offset = _offSet[layerID];
  //...Offset modification to be aligned to axial at z=0...
  const double phiSize = 2 * M_PI / double(nWires);
  {
    const double phiF = phiSize * offset
                        + phiSize * 0.5 * double(_nShifts[layerID]);
    const double phiB = phiSize * offset;
    const TVector3 f(_rSLayer[layerID] * cos(phiF), _rSLayer[layerID] * sin(phiF), _zSForwardLayer[layerID]);
    const TVector3 b(_rSLayer[layerID] * cos(phiB), _rSLayer[layerID] * sin(phiB), _zSBackwardLayer[layerID]);

    const TVector3 v = f - b;
    const TVector3 u = v.Unit();
    const double beta = (0 - b.z()) / u.z();
    const TVector3 p = b + beta * u;
    double phi0 = - atan2(p.y(), p.x());
    offset += phi0;
  }

  const double phiF = phiSize * (double(cellID) + offset)
                      + phiSize * 0.5 * double(_nShifts[layerID]);
  const double phiB = phiSize * (double(cellID) + offset);
  const TVector3 f(_rSLayer[layerID] * cos(phiF), _rSLayer[layerID] * sin(phiF), _zSForwardLayer[layerID]);
  const TVector3 b(_rSLayer[layerID] * cos(phiB), _rSLayer[layerID] * sin(phiB), _zSBackwardLayer[layerID]);
  _wireForwardPosition[layerID][cellID].SetX(f.X());
  _wireForwardPosition[layerID][cellID].SetY(f.Y());
  _wireForwardPosition[layerID][cellID].SetZ(f.Z());

  return _wireForwardPosition[layerID][cellID];
}

TVector3 CDCGeometryPar::wireBackwardPosition(int layerID, int cellID)
{
  const unsigned nWires = _nWires[layerID];

  double offset = _offSet[layerID];
  //...Offset modification to be aligned to axial at z=0...
  const double phiSize = 2 * M_PI / double(nWires);
  {
    const double phiF = phiSize * offset
                        + phiSize * 0.5 * double(_nShifts[layerID]);
    const double phiB = phiSize * offset;
    const TVector3 f(_rSLayer[layerID] * cos(phiF), _rSLayer[layerID] * sin(phiF), _zSForwardLayer[layerID]);
    const TVector3 b(_rSLayer[layerID] * cos(phiB), _rSLayer[layerID] * sin(phiB), _zSBackwardLayer[layerID]);

    const TVector3 v = f - b;
    const TVector3 u = v.Unit();
    const double beta = (0 - b.z()) / u.z();
    const TVector3 p = b + beta * u;
    double phi0 = - atan2(p.y(), p.x());
    offset += phi0;
  }

  const double phiF = phiSize * (double(cellID) + offset)
                      + phiSize * 0.5 * double(_nShifts[layerID]);
  const double phiB = phiSize * (double(cellID) + offset);
  const TVector3 f(_rSLayer[layerID] * cos(phiF), _rSLayer[layerID] * sin(phiF), _zSForwardLayer[layerID]);
  const TVector3 b(_rSLayer[layerID] * cos(phiB), _rSLayer[layerID] * sin(phiB), _zSBackwardLayer[layerID]);
  _wireBackwardPosition[layerID][cellID].SetX(b.X());
  _wireBackwardPosition[layerID][cellID].SetY(b.Y());
  _wireBackwardPosition[layerID][cellID].SetZ(b.Z());

  return _wireBackwardPosition[layerID][cellID];
}

const double *
CDCGeometryPar::innerRadiusWireLayer(void) const
{
  static double IRWL[MAX_N_SLAYERS] = {0};

  IRWL[0] = outerRadiusInnerWall();
  for (unsigned i = 1; i < nWireLayers(); i++)
    //IRWL[i] = (_rSLayer[i - 1] + _rSLayer[i]) / 2.;
    IRWL[i] = _rFLayer[i - 1];

  return IRWL;
}

const double *
CDCGeometryPar::outerRadiusWireLayer(void) const
{
  static double ORWL[MAX_N_SLAYERS] = {0};

  ORWL[nWireLayers() - 1] = innerRadiusOuterWall();
  for (unsigned i = 0; i < nWireLayers() - 1; i++)
    //ORWL[i] = (_rSLayer[i] + _rSLayer[i + 1]) / 2.;
    ORWL[i] = _rFLayer[i];

  return ORWL;
}

unsigned
CDCGeometryPar::cellId(unsigned layerId,
                       const TVector3 & position) const
{
  const unsigned nWires = _nWires[layerId];

  double offset = _offSet[layerId];
  //...Offset modification to be aligned to axial at z=0...
  const double phiSize = 2 * M_PI / double(nWires);
  {
    const double phiF = phiSize * offset
                        + phiSize * 0.5 * double(_nShifts[layerId]);
    const double phiB = phiSize * offset;
    const TVector3 f(_rSLayer[layerId] * cos(phiF), _rSLayer[layerId] * sin(phiF), _zSForwardLayer[layerId]);
    const TVector3 b(_rSLayer[layerId] * cos(phiB), _rSLayer[layerId] * sin(phiB), _zSBackwardLayer[layerId]);

    const TVector3 v = f - b;
    const TVector3 u = v.Unit();
    const double beta = (0 - b.z()) / u.z();
    const TVector3 p = b + beta * u;
    double phi0 = - atan2(p.y(), p.x());
    offset += phi0;
  }

  unsigned j = 0;
  for (unsigned i = 0; i < 1; i++) {
    const double phiF = phiSize * (double(i) + offset)
                        + phiSize * 0.5 * double(_nShifts[layerId]);
    const double phiB = phiSize * (double(i) + offset);
    const TVector3 f(_rSLayer[layerId] * cos(phiF), _rSLayer[layerId] * sin(phiF), _zSForwardLayer[layerId]);
    const TVector3 b(_rSLayer[layerId] * cos(phiB), _rSLayer[layerId] * sin(phiB), _zSBackwardLayer[layerId]);
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

void CDCGeometryPar::generateXML(const string & of)
{
  //...Open xml file...
  std::ofstream ofs(of.c_str(), std::ios::out);
  if (! ofs) {
    ERROR("CDCGeometryPar::read !!! can not open file : "
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

  for (int i = 0; i < _nSLayer; i++) {
    ofs << "      <SLayer id=\"" << i << "\">" << endl;
    ofs << "        <Radius desc=\"Radius of wires in this layer\" unit=\"mm\">" << senseWireR(i) << "</Radius>" << endl;
    ofs << "        <BackwardZ desc=\"z position of this wire layer at backward endplate\" unit=\"mm\">" << senseWireBZ(i) << "</BackwardZ>" << endl;
    ofs << "        <ForwardZ desc=\"z position of this wire layer at forward endplate\" unit=\"mm\">" << senseWireFZ(i) << "</ForwardZ>" << endl;
//    ofs << "        <BackwardPhi desc=\"azimuth angle of the first wire in this layer at backward endplate\" unit=\"rad\">" << wireBackwardPosition(i).phi() << "</BackwardPhi>" << endl;
//    ofs << "        <ForwardPhi desc=\"azimuth angle of the first wire in this layer at forward endplate\" unit=\"rad\">" << wireForwardPosition(i).phi() << "</ForwardPhi>" << endl;
    ofs << "        <NHoles desc=\"the number of holes in this layer, 2*(cell number)\">" << nWiresInLayer(i)*2 << "</NHoles>" << endl;
    ofs << "        <NShift desc=\"the shifted hole number of each wire in this layer\">" << nShifts(i) << "</NShift>" << endl;
    ofs << "        <Offset desc=\"wire offset in phi direction at endplate\">" << _offSet[i] << "</Offset>" << endl;
    ofs << "      </SLayer>" << endl;
  }

  ofs << "    </SLayers>" << endl;
  ofs << "    <FLayers>" << endl;

  for (int i = 0; i < _nFLayer; i++) {
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
  ofs << "      <BackwardZ desc=\"z position at backward endplate\" unit=\"mm\">" << _zWall[0][0] << "</BackwardZ>" << endl;
  ofs << "      <ForwardZ desc=\"z position at forward endplate\" unit=\"mm\">" << _zWall[0][1] << "</ForwardZ>" << endl;
  ofs << "    </InnerWall>" << endl;

  ofs << "    <OuterWall name=\"OuterWall\">" << endl;
  ofs << "      <InnerR desc=\"Inner radius\" unit=\"mm\">" << innerRadiusOuterWall() << "</InnerR>" << endl;
  ofs << "      <OuterR desc=\"Outer radius\" unit=\"mm\">" << outerRadiusOuterWall() << "</OuterR>" << endl;
  ofs << "      <BackwardZ desc=\"z position at backward endplate\" unit=\"mm\">" << _zWall[2][0] << "</BackwardZ>" << endl;
  ofs << "      <ForwardZ desc=\"z position at forward endplate\" unit=\"mm\">" << _zWall[2][1] << "</ForwardZ>" << endl;
  ofs << "    </OuterWall>" << endl;

  ofs << "  </Content>"                                         << endl
  << "</Subdetector>"                                       << endl;
}
