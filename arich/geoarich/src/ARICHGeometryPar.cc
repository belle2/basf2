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

#include <arich/geoometry/ARICHGeometryPar.h>

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

ARICHGeometryPar::ARICHGeometryPar(const GearDir& content)
{
  clear();
  read(content);
  modulesPosition(content);
  chipLocPosition();
  padPositions();
  mirrorPositions();
}

ARICHGeometryPar::~ARICHGeometryPar()
{
}

void
ARICHGeometryPar::clear(void)
{
  _chipGap = 0.0;
  _detZpos = 0.0;
  _modXSize = 0.0;
  _modZSize = 0.0;
  _winThick = 0.0;
  _mirrorOuterRad = 0.0;
  _mirrorThickness = 0.0;
  _mirrorStartAng = 0.0;
  _nMirrors = 0;
  _nPadX = 0;
  _padSize = 0.0;
  _detInnerRadius = 0.0;
  _detOuterRadius = 0.0;
  _nRad = 0;
  _nrow = 0;

  _ncol.clear(); _fDFi.clear(); _fDR.clear(); _fR.clear();
  _fFi.clear(); _chipLocPos.clear(); _padWorldPositions.clear(); _mirrornorm.clear(); _mirrorpoint.clear();
}

void ARICHGeometryPar::read(const GearDir& content)
{

  //------------------------------
  // Get ARICH geometry parameters
  //------------------------------
  GearDir detParams(content, "Detector")
  _detZpos = detParams.getParamLength("Plane/zPosition");
  _detInnerRadius = detParams.getParamLength("Plane/TubeInnerRadius");
  _detOuterRadius = detParams.getParamLength("Plane/TubeOuterRadius");
  _modXSize = detParams.getParamLength("Module/ModuleXSize");
  _modZSize = detParams.getParamLength("Module/ModuleZSize");
  _winThick = detParams.getParamLength("Module/WindowThickness");
  _nPadX = int(detParams.getParamNumValue("Module/PadXNum"));
  _padSize = detParams.getParamLength("Module/PadSize");
  _chipGap = detParams.getParamLength("Module/ChipGap");

  GearDir mirrParams(content, "Mirrors");
  _nMirrors = int(gbxParams.getParamLength("nMirrors"));
  _mirrorThickness =  gbxParams.getParamLength("mirrorThickness");
  _mirrorOuterRad = gbxParams.getParamLength("outerRadius");
  _mirrorStartAng = gbxParams.getParamAngle("startAngle");
}

void ARICHGeometryPar::Print(void) const
{
}

int ARICHGeometryPar::getChannelID(TVector2 position)
{
  int ChipID = getChipID(position);
  int Npad = int(_nPadX / 2);
  TVector2 chipPos = getChipLocPos(ChipID);
  TVector2 locloc = position - chipPos;
  int ix = int(locloc.X() / _padSize);
  int iy = int(locloc.Y() / _padSize);
  if (ix > Npad - 1 || iy > Npad - 1) return -1;
  int chID = ChipID * Npad * Npad + iy + ix * Npad;
  return chID;
}

void ARICHGeometryPar::modulesPosition(const GearDir& content)
{

  GearDir detParams(content, "Detector/Plane/Rings");


  double r = _detOuterRadius - _modXSize - dR;
  BOOST_FOREACH(const GearDir &ring, content.getNodes("Ring")) {
    int nRing = gbxParams.getNumberNodes("Rings/Ring");
    int iRing = nRing;
    double dR = gbxParams.getParamLength((format("Rings/Ring[%1%]/dR") % (nRing)).str());

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

  int ARICHGeometryPar::getCopyNo(TVector3 hit) {
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

  TVector3 ARICHGeometryPar::getOrigin(int copyNo) {
    TVector2 origin;
    origin.SetMagPhi(_fR[copyNo], _fFi[copyNo]);
    return TVector3(origin.X(), origin.Y(), _detZpos + _modZSize / 2.);
  }

  double ARICHGeometryPar::getModAngle(int copyno) {
    return _fFi[copyno];
  }

  void ARICHGeometryPar::chipLocPosition() {
    double xycenter =  _padSize * _nPadX / 4. + _chipGap / 2.;
    _chipLocPos.push_back(TVector2(xycenter - _padSize*_nPadX / 4., xycenter - _padSize*_nPadX / 4.));
    _chipLocPos.push_back(TVector2(xycenter - _padSize*_nPadX / 4., -xycenter - _padSize*_nPadX / 4.));
    _chipLocPos.push_back(TVector2(-xycenter - _padSize*_nPadX / 4., xycenter - _padSize*_nPadX / 4.));
    _chipLocPos.push_back(TVector2(-xycenter - _padSize*_nPadX / 4., -xycenter - _padSize*_nPadX / 4.));
  }


  int ARICHGeometryPar::getChipID(TVector2 locpos) {
    if (locpos.X() > 0) {
      if (locpos.Y() > 0) return 0;
      return 1;
    }
    if (locpos.Y() > 0) return 2;
    return 3;
  }


  TVector3 ARICHGeometryPar::getChannelCenterGlob(int modID, int chanID) {
    std::pair<int, int> ModChan;
    ModChan.first = modID; ModChan.second = chanID;
    return _padWorldPositions[ModChan];
  }

  TVector2 ARICHGeometryPar::getChannelCenterLoc(int chID) {
    return _padLocPositions[chID];
  }


  void ARICHGeometryPar::padPositions() {
    int Npad = int(_nPadX / 2.);
    TVector2 xstart(_padSize / 2., _padSize / 2.);
    for (int chipID = 0; chipID < 4; chipID++) {
      TVector2 chipPos = getChipLocPos(chipID);
      for (int ix = 0; ix < Npad; ix++) {
        for (int iy = 0; iy < Npad; iy++) {
          int chanID = chipID * Npad * Npad + ix * Npad + iy;
          TVector2 center(_padSize / 2. + ix*_padSize, _padSize / 2. + iy*_padSize);
          center = center + chipPos;
          _padLocPositions[chanID] = center;
        }
      }
    }
    for (int iMod = 0; iMod < getNMCopies(); iMod++) {
      for (unsigned int iChan = 0; iChan < _padLocPositions.size(); iChan++) {
        TVector2 iModCenter;
        iModCenter.SetMagPhi(_fR[iMod], _fFi[iMod]);
        TVector2 iChanCenter = _padLocPositions[iChan];
        iChanCenter = iChanCenter.Rotate(_fFi[iMod]);
        TVector3 iWorld((iModCenter + iChanCenter).X(), (iModCenter + iChanCenter).Y(), _detZpos + _winThick);
        std::pair<int, int> ModChan;
        ModChan.first = iMod; ModChan.second = iChan;
        _padWorldPositions[ModChan] = iWorld;
      }
    }
  }

  void ARICHGeometryPar::mirrorPositions() {
    double rmir = _mirrorOuterRad * cos(M_PI / _nMirrors) - _mirrorThickness;
    for (int i = 0; i < _nMirrors; i++) {
      TVector3 norm(cos(2.*M_PI / double(_nMirrors)*(i + 0.5) + _mirrorStartAng), sin(2.*M_PI / double(_nMirrors)*(i + 0.5) + _mirrorStartAng), 0);
      _mirrornorm.push_back(norm);
      _mirrorpoint.push_back(rmir*norm);
    }
  }

  TVector3 ARICHGeometryPar::getMirrorNormal(int mirID) {
    return _mirrornorm[mirID];
  }

  TVector3 ARICHGeometryPar::getMirrorPoint(int mirID) {
    return _mirrorpoint[mirID];
  }
