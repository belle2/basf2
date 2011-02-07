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
  chipLocPosition();
  PadPositions();
  MirrorPositions();
}

ARICHGeometryPar::~ARICHGeometryPar()
{
}

void
ARICHGeometryPar::clear(void)
{
  _version = "unknown";
  _tileSize = 0.0;
  _chipGap = 0.0;
  _tubeInnerRadius = 0.0;
  _tubeOuterRadius = 0.0;
  _tileGap = 0.0;
  _detZpos = 0.0;
  _modXSize = 0.0;
  _modZSize = 0.0;
  _winThick = 0.0;
  _winRefInd = 0.0;
  _mirrorZpos = 0.0;
  _mirrorLength = 0.0;
  _mirrorOuterRad = 0.0;
  _mirrorThickness = 0.0;
  _mirrorStartAng = 0.0;
  _trackPosRes = 0.0;
  _trackAngRes = 0.0;
  _photRes = 0.0;
  _detBack = 0.0;
  _normFact = 0.0;
  _nMirrors = 0;
  _nPadX = 0;
  _padSize = 0.0;
  _detInnerRadius = 0.0;
  _detOuterRadius = 0.0;
  _nRad = 0;
  _nrow = 0;
  for (int i = 0; i < MAX_N_ALAYERS; ++i) {
    _aeroZpos[i] = 0.0;
    _aeroThick[i] = 0.0;
    _aeroRefInd[i] = 0.0;
    _aeroTrLen[i] = 0.0;
  }
  _ncol.clear(); _fDFi.clear(); _fDR.clear(); _fR.clear();
  _fFi.clear(); _tilePos.clear(); _chipLocPos.clear(); _padWorldPositions.clear(); _mirrornorm.clear(); _mirrorpoint.clear();
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
  _modZSize = gbxParams.getParamLength("Detector/Module/ModuleZSize");
  _winThick = gbxParams.getParamLength("Detector/Module/WindowThickness");
  _winRefInd = gbxParams.getParamNumValue("Detector/Module/WindowRefIndex");
  _nPadX = int(gbxParams.getParamNumValue("Detector/Module/PadXNum"));
  _padSize = gbxParams.getParamLength("Detector/Module/PadSize");
  _chipGap = gbxParams.getParamLength("Detector/Module/ChipGap");
  _detInnerRadius = gbxParams.getParamLength("Detector/Plane/TubeInnerRadius");
  _detOuterRadius = gbxParams.getParamLength("Detector/Plane/TubeOuterRadius");
  _nMirrors = int(gbxParams.getParamLength("Mirrors/nMirrors"));
  _mirrorZpos = gbxParams.getParamLength("Mirrors/Zposition");
  _mirrorLength = gbxParams.getParamLength("Mirrors/mirrorLength");
  _mirrorThickness =  gbxParams.getParamLength("Mirrors/mirrorThickness");
  _mirrorOuterRad = gbxParams.getParamLength("Mirrors/outerRadius");
  _mirrorStartAng = gbxParams.getParamAngle("Mirrors/startAngle");
  _trackPosRes =  gbxParams.getParamLength("Reconstruction/TrackPosResolution");
  _trackAngRes =  gbxParams.getParamAngle("Reconstruction/TrackAngResolution");
  _photRes = gbxParams.getParamLength("Reconstruction/PhotResolutionWoPad");
  _detBack = gbxParams.getParamNumValue("Reconstruction/DetectBackground");
  _normFact = gbxParams.getParamNumValue("Reconstruction/NormalisationFactor");
  int nLayer = gbxParams.getNumberNodes("Aerogel/Layers/Layer");
  _nRad = nLayer;
  for (int iLayer = 1; iLayer <= nLayer; iLayer++) {
    _aeroThick[iLayer-1] = gbxParams.getParamLength((format("Aerogel/Layers/Layer[%1%]/Thickness") % (iLayer)).str());
    _aeroZpos[iLayer-1] = gbxParams.getParamLength((format("Aerogel/Layers/Layer[%1%]/Zposition") % (iLayer)).str());
    _aeroTrLen[iLayer-1] = gbxParams.getParamLength((format("Aerogel/Layers/Layer[%1%]/TransLen") % (iLayer)).str());
    _aeroRefInd[iLayer-1] = gbxParams.getParamNumValue((format("Aerogel/Layers/Layer[%1%]/RefInd") % (iLayer)).str());
    _aeroFigMerit[iLayer-1] = gbxParams.getParamNumValue((format("Aerogel/Layers/Layer[%1%]/FigOfMerit") % (iLayer)).str());
  }
}

void ARICHGeometryPar::Print(void) const
{
}

int ARICHGeometryPar::GetChannelID(TVector2 position)
{
  int ChipID = GetChipID(position);
  int Npad = int(_nPadX / 2);
  TVector2 chipPos = GetChipLocPos(ChipID);
  TVector2 locloc = position - chipPos;
  int ix = int(locloc.X() / _padSize);
  int iy = int(locloc.Y() / _padSize);
  if (ix > Npad - 1 || iy > Npad - 1) return -1;
  int chID = ChipID * Npad * Npad + iy + ix * Npad;
  return chID;
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
    int nSeg = int(gbxParams.getParamNumValue((format("Rings/Ring[%1%]/nSegments") % (nRing)).str())) ;
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
  double x = hit.X();
  double y = hit.Y();
  double r = sqrt(x * x + y * y);
  double fi = atan2(y, x);
  if (fi < 0) fi += 2 * M_PI;
  int ntot = 0;
  for (int i = 0; i < _nrow; i++) {
    int nfi = int(fi / _fDFi[i]);
    int copyno = ntot + nfi;
    if (fabs(r - _fR[copyno]) <  _modXSize / 2.) return copyno;
    ntot += _ncol[i];
  }
  return -1;
}

TVector3 ARICHGeometryPar::GetOrigin(int copyNo)
{
  TVector2 origin;
  origin.SetMagPhi(_fR[copyNo], _fFi[copyNo]);
  return TVector3(origin.X(), origin.Y(), _detZpos + _modZSize / 2.);
}

TVector2 ARICHGeometryPar::GetTilePos(int i)
{
  return _tilePos.at(i);
}

double ARICHGeometryPar::GetModAngle(int copyno)
{
  return _fFi[copyno];
}

void ARICHGeometryPar::chipLocPosition()
{
  double xycenter =  _padSize * _nPadX / 4. + _chipGap / 2.;
  _chipLocPos.push_back(TVector2(xycenter - _padSize*_nPadX / 4., xycenter - _padSize*_nPadX / 4.));
  _chipLocPos.push_back(TVector2(xycenter - _padSize*_nPadX / 4., -xycenter - _padSize*_nPadX / 4.));
  _chipLocPos.push_back(TVector2(-xycenter - _padSize*_nPadX / 4., xycenter - _padSize*_nPadX / 4.));
  _chipLocPos.push_back(TVector2(-xycenter - _padSize*_nPadX / 4., -xycenter - _padSize*_nPadX / 4.));
}


int ARICHGeometryPar::GetChipID(TVector2 locpos)
{
  if (locpos.X() > 0) {
    if (locpos.Y() > 0) return 0;
    return 1;
  }
  if (locpos.Y() > 0) return 2;
  return 3;
}


TVector3 ARICHGeometryPar::GetChannelCenterGlob(int modID, int chanID)
{
  std::pair<int, int> ModChan;
  ModChan.first = modID; ModChan.second = chanID;
  return _padWorldPositions[ModChan];
}

TVector2 ARICHGeometryPar::GetChannelCenterLoc(int chID)
{
  return _padLocPositions[chID];
}


void ARICHGeometryPar::PadPositions()
{
  int Npad = int(_nPadX / 2.);
  std::pair<int, int> ModChan;
  TVector2 xstart(_padSize / 2., _padSize / 2.);
  for (int chipID = 0; chipID < 4; chipID++) {
    TVector2 chipPos = GetChipLocPos(chipID);
    for (int ix = 0; ix < Npad; ix++) {
      for (int iy = 0; iy < Npad; iy++) {
        int chanID = chipID * Npad * Npad + ix * Npad + iy;
        TVector2 center(_padSize / 2. + ix*_padSize, _padSize / 2. + iy*_padSize);
        center = center + chipPos;
        _padLocPositions[chanID] = center;
      }
    }
  }
  for (int iMod = 0; iMod < GetNMCopies(); iMod++) {
    for (unsigned int iChan = 0; iChan < _padLocPositions.size(); iChan++) {
      TVector2 iModCenter;
      iModCenter.SetMagPhi(_fR[iMod], _fFi[iMod]);
      TVector2 iChanCenter = _padLocPositions[iChan];
      iChanCenter = iChanCenter.Rotate(_fFi[iMod]);
      TVector3 iWorld((iModCenter + iChanCenter).X(), (iModCenter + iChanCenter).Y(), _detZpos + _winThick);
      ModChan.first = iMod; ModChan.second = iChan;
      _padWorldPositions[ModChan] = iWorld;
    }
  }
}

void ARICHGeometryPar::MirrorPositions()
{
  double rmir = _mirrorOuterRad * cos(M_PI / _nMirrors) - _mirrorThickness;
  for (int i = 0; i < _nMirrors; i++) {
    TVector3 norm(cos(2.*M_PI / double(_nMirrors)*(i + 0.5) + _mirrorStartAng), sin(2.*M_PI / double(_nMirrors)*(i + 0.5) + _mirrorStartAng), 0);
    _mirrornorm.push_back(norm);
    _mirrorpoint.push_back(rmir*norm);
  }
}

TVector3 ARICHGeometryPar::GetMirrorNormal(int mirID)
{
  return _mirrornorm[mirID];
}

TVector3 ARICHGeometryPar::GetMirrorPoint(int mirID)
{
  return _mirrorpoint[mirID];
}
