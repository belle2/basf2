/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj, Rok Pestotnik                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <arich/geoarich/ARICHGeometryPar.h>

#include <cmath>
#include <boost/format.hpp>
#include <iostream>
#include <fstream>

using namespace std;
using namespace boost;
using namespace Belle2;

ARICHGeometryPar* ARICHGeometryPar::p_B4ARICHGeometryParDB = 0;

ARICHGeometryPar* ARICHGeometryPar::Instance(void)
{
  if (!p_B4ARICHGeometryParDB) p_B4ARICHGeometryParDB = new ARICHGeometryPar();
  return p_B4ARICHGeometryParDB;
}

ARICHGeometryPar::ARICHGeometryPar()
{
  clear();
  read();
  modules_position();
  aerotile_position();
}

ARICHGeometryPar::~ARICHGeometryPar()
{
}

void
ARICHGeometryPar::clear(void)
{
  _tileSize = 0.0;
  _tubeInnerRadius = 0.0;
  _tubeOuterRadius = 0.0;
  _tileGap = 0.0;
  _detZpos = 0.0;
  _modXSize = 0.0;
  _modYSize = 0.0;
  _modZSize = 0.0;
  _winThick = 0.0;
  _sensXSize = 0.0;
  _sensYSize = 0.0;
  _mirrorZpos = 0.0;
  _mirrorLength = 0.0;
  _mirrorOuterRad = 0.0;
  _mirrorThickness = 0.0;
  _nPadX = 0;
  _nPadY = 0;
  _padSize = 0.0;
  _detInnerRadius = 0.0;
  _detOuterRadius = 0.0;
  _nRad = 0;
  _nrow = 0;
  for (int i = 0; i < MAX_N_ALAYERS; ++i) {
    _aeroZpos[i] = 0.0;
    _aeroThick[i] = 0.0;
  }
  _ncol.clear(); _fDFi.clear(); _fDR.clear(); _fR.clear();
  _fFi.clear(); _tilePos.clear();
}

void ARICHGeometryPar::read()
{

  GearDir gbxParams = Gearbox::Instance().getContent("ARICH");

  //------------------------------
  // Get ARICH geometry parameters
  //------------------------------

  _tileSize    = gbxParams.getParamLength("Aerogel/TileSize");
  _tubeInnerRadius = gbxParams.getParamLength("Aerogel/TubeInnerRadius");
  _tubeOuterRadius = gbxParams.getParamLength("Aerogel/TubeOuterRadius");
  _tileGap = gbxParams.getParamLength("Aerogel/TileGap");
  _detZpos = gbxParams.getParamLength("Detector/Plane/zPosition");
  _modXSize = gbxParams.getParamLength("Detector/Module/ModuleXSize");
  _modYSize = gbxParams.getParamLength("Detector/Module/ModuleYSize");
  _modZSize = gbxParams.getParamLength("Detector/Module/ModuleZSize");
  _winThick = gbxParams.getParamLength("Detector/Module/WindowThickness");
  _sensXSize = gbxParams.getParamLength("Detector/Module/SensXSize");
  _sensYSize = gbxParams.getParamLength("Detector/Module/SensYSize");
  _nPadX = gbxParams.getParamNumValue("Detector/Module/PadXNum");
  _nPadY = gbxParams.getParamNumValue("Detector/Module/PadYNum");
  _padSize = gbxParams.getParamLength("Detector/Module/PadSize");
  _detInnerRadius = gbxParams.getParamLength("Detector/Plane/TubeInnerRadius");
  _detOuterRadius = gbxParams.getParamLength("Detector/Plane/TubeOuterRadius");
  _nMirrors = gbxParams.getParamLength("Mirrors/nMirrors");
  _mirrorZpos = gbxParams.getParamLength("Mirrors/Zposition");
  _mirrorLength = gbxParams.getParamLength("Mirrors/mirrorLength");
  _mirrorThickness =  gbxParams.getParamLength("Mirrors/mirrorThickness");
  _mirrorOuterRad = gbxParams.getParamLength("Mirrors/outerRadius");
  int nLayer = gbxParams.getNumberNodes("Aerogel/Layers/Layer");
  _nRad = nLayer;
  for (int iLayer = 1; iLayer <= nLayer; iLayer++) {
    _aeroThick[iLayer-1] = gbxParams.getParamLength((format("Aerogel/Layers/Layer[%1%]/Thickness") % (iLayer)).str());
    _aeroZpos[iLayer-1] = gbxParams.getParamLength((format("Aerogel/Layers/Layer[%1%]/Zposition") % (iLayer)).str());
  }
}

void ARICHGeometryPar::Print(void) const
{
}

int ARICHGeometryPar::GetChannelID(TVector3 position)
{
  int ix = position.x() / _padSize + _nPadX / 2;
  int iy = position.y() / _padSize + _nPadY / 2;
  int channelID = ix + iy * _nPadX;
  return channelID;
}

TVector3 ARICHGeometryPar::GetChannelCenter(int chID)
{
  int yi = (chID / _nPadX);
  int xi = chID - yi * _nPadX;
  double x  = xi * _padSize - (_nPadX - 1) * _padSize / 2.;
  double y  = (2 * yi + 1 - _nPadX) * _padSize / 2.;
  TVector3 center(x, y, 0);
  return center;
}

void ARICHGeometryPar::modules_position()
{
  GearDir gbxParams = Gearbox::Instance().getContent("ARICH");
  gbxParams.append("Detector/Plane/");
  int nRing = gbxParams.getNumberNodes("Rings/Ring");
  int iRing = nRing;
  _nrow = 0;
  double dR = gbxParams.getParamLength((format("Rings/Ring[%1%]/dR") % (nRing)).str());
  double r = _detOuterRadius - _modXSize - dR;
  while (r > _detInnerRadius && iRing > 0) {
    int nSeg = gbxParams.getParamNumValue((format("Rings/Ring[%1%]/nSegments") % (nRing)).str());
    double dFi = gbxParams.getParamLength((format("Rings/Ring[%1%]/dFi") % (nRing)).str());
    _nrow += 1;
    double dR = gbxParams.getParamLength((format("Rings/Ring[%1%]/dR") % (nRing)).str());
    _fDR.push_back(dR);
    double f = 2.*atan2((_modXSize + dFi) / 2., r);
    int blaa = int(2.*M_PI / f / nSeg) * nSeg;
    _ncol.push_back(blaa);
    f = 2.*M_PI / blaa;
    _fDFi.push_back(f);
    for (int nv = 0; nv < blaa; ++nv) {
      double rcenter = r + _modXSize / 2.;
      _fR.push_back(rcenter);
      double fi = f * (nv + 0.5);
      _fFi.push_back(fi);
    }
    r -= (_modXSize + dR + r * (1 - cos(f / 2.)));
    iRing -= 1;
  }

}

void ARICHGeometryPar::aerotile_position()
{
  double TileFrameSize = _tileSize + _tileGap / 2.;
  double TFSdiag = TileFrameSize * 2. / sqrt(3.);
  TVector2 dx(sqrt(3.)*TileFrameSize, TileFrameSize);
  TVector2 xstart(- _tubeOuterRadius - TFSdiag, -_tubeOuterRadius - TFSdiag);
  TVector2 x = xstart;
  int j = 1;
  while (x.X() < _tubeOuterRadius + TFSdiag) {
    while (x.Y() < _tubeOuterRadius + TFSdiag) {
      if (sqrt(x*x) < _tubeOuterRadius + TFSdiag && sqrt(x*x) > _tubeInnerRadius - TFSdiag) _tilePos.push_back(x);
      x.Set(x.X(), x.Y() + 2*TileFrameSize);
    }
    j *= -1;
    xstart.Set(xstart.X() + dx.X(), xstart.Y() + j*dx.Y());
    x = xstart;
  }
}

int ARICHGeometryPar::GetCopyNo(TVector3 hit)
{
  double x = hit.x();
  double y = hit.y();
  double r = sqrt(x * x + y * y);
  double fi = atan2(y, x);
  if (fi < 0) fi += 2 * M_PI;
  int ntot = 0;
  for (int i = 0; i < _nrow; i++) {
    int nfi = int(fi / _fDFi[i]);
    int copyno = ntot + nfi;
    if (r > _fR[copyno] - _modXSize / 2. && r < _fR[copyno] + _modYSize / 2.) return copyno;
    ntot += _ncol[i];
  }
  return -1;
}

TVector3 ARICHGeometryPar::GetOrigin(int copyNo)
{
  double cfi = cos(_fFi[copyNo]);
  double sfi = sin(_fFi[copyNo]);
  double x0 = _fR[copyNo] * cfi;
  double y0 = _fR[copyNo] * sfi;
  return TVector3(x0, y0, _detZpos + _modZSize / 2.);
}

TVector2 ARICHGeometryPar::GetTilePos(int i)
{
  return _tilePos.at(i);
}

