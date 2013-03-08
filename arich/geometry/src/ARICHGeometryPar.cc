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
#include <framework/gearbox/Unit.h>

#include <arich/geometry/ARICHGeometryPar.h>

#include <cmath>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <TGraph.h>

using namespace std;
using namespace boost;

namespace Belle2 {
  namespace arich {

    ARICHGeometryPar* ARICHGeometryPar::p_B4ARICHGeometryParDB = 0;

    ARICHGeometryPar* ARICHGeometryPar::Instance()
    {
      if (!p_B4ARICHGeometryParDB) {
        p_B4ARICHGeometryParDB = new ARICHGeometryPar();
      }
      return p_B4ARICHGeometryParDB;
    }

    ARICHGeometryPar::ARICHGeometryPar()
    {
      clear();
    }

    ARICHGeometryPar::~ARICHGeometryPar()
    {
    }

    void ARICHGeometryPar::Initialize(const GearDir& content, const GearDir& mirrorcontent)
    {

      read(content);
      string Type = content.getString("@type", "");
      if (Type == "beamtest") {
        m_simple = true;
        modulesPositionSimple(content);
        mirrorPositionSimple(mirrorcontent);
      } else {
        modulesPosition(content);
        mirrorPositions();
      }
      chipLocPosition();
      padPositions();
      initDetectorMask(_fR.size());
      m_init = true;
    }

    void ARICHGeometryPar::Initialize(const GearDir& content)
    {
      Initialize(content, content);
    }

    void ARICHGeometryPar::clear(void)
    {
      _chipGap = 0.0;
      _detZpos = 0.0;
      _modXSize = 0.0;
      _modZSize = 0.0;
      _winThick = 0.0;
      _winRefInd = 0.0;
      _mirrorOuterRad = 0.0;
      _mirrorThickness = 0.0;
      _mirrorStartAng = 0.0;
      _nMirrors = 0;
      _mirrorZPos = 0.0;
      _nPadX = 0;
      _padSize = 0.0;
      _detInnerRadius = 0.0;
      _detOuterRadius = 0.0;
      _nrow = 0;
      _nRad = 0;
      m_init = false;
      m_simple = false;
      _ncol.clear(); _fDFi.clear(); _fDR.clear(); _fR.clear();
      _fFi.clear(); _fFiMod.clear(); _chipLocPos.clear(); _padWorldPositions.clear(); _mirrornorm.clear(); _mirrorpoint.clear();
      for (int i = 0; i < MAX_N_ALAYERS; i++) {
        _aeroTrLength[i] = 0; _aeroRefIndex[i] = 0;
        _aeroZPosition[i] = 0; _aeroThickness[i] = 0;
      }
      m_nPads = 0;
    }

    void ARICHGeometryPar::read(const GearDir& content)
    {

      //------------------------------
      // Get ARICH geometry parameters
      //------------------------------
      GearDir detParams(content, "Detector");
      _modXSize = detParams.getLength("Module/moduleXSize");
      _modZSize = detParams.getLength("Module/moduleZSize");
      _winThick = detParams.getLength("Module/windowThickness");
      _nPadX = detParams.getInt("Module/padXNum");
      m_nPads = _nPadX * _nPadX;
      _padSize = detParams.getLength("Module/padSize");
      _chipGap = detParams.getLength("Module/chipGap");
      _detZpos = detParams.getLength("Plane/zPosition");
      string Type = content.getString("@type", "");
      if (Type == "beamtest") return;
      _detInnerRadius = detParams.getLength("Plane/tubeInnerRadius");
      _detOuterRadius = detParams.getLength("Plane/tubeOuterRadius");

      GearDir mirrParams(content, "Mirrors");
      if (mirrParams) {
        _nMirrors = mirrParams.getInt("nMirrors");
        _mirrorThickness =  mirrParams.getLength("mirrorThickness");
        _mirrorOuterRad = mirrParams.getLength("outerRadius");
        _mirrorStartAng = mirrParams.getAngle("startAngle");
        _mirrorZPos = mirrParams.getLength("Zposition");
      }
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

      double r = _detInnerRadius;

      BOOST_FOREACH(const GearDir & ring, detParams.getNodes("Ring")) {
        double dR = ring.getLength("dR");
        r += dR;
        double rcenter = r + _modXSize / 2.;
        if (rcenter + _modXSize * sqrt(2) / 2. > _detOuterRadius) {
          B2WARNING(_ncol.size() + 1  << "th ring of ARICH photon detectors will not be placed (out of detector tube).");
          break;
        }
        _nrow++;
        int nSeg = ring.getInt("nSegments") ;
        double dFi = ring.getLength("dFi");
        _fDR.push_back(dR);
        double f = 2.*atan2((_modXSize + dFi) / 2., r);
        int blaa = int(2.*M_PI / f / nSeg) * nSeg;
        _ncol.push_back(blaa);
        f = 2.*M_PI / double(blaa);
        _fDFi.push_back(f);
        B2INFO(blaa << " modules of " << _ncol.size() << "th ring of ARICH photon detectors will be placed at r = " << rcenter << "cm. ");
        for (int nv = 0; nv < blaa; ++nv) {
          _fR.push_back(rcenter);
          double fi = f * (nv + 0.5);
          _fFi.push_back(fi);
          _fFiMod.push_back(fi);
        }
        r += (_modXSize  + r * (1 - cos(f / 2.)));
      }
      B2INFO("Altogether " << _fR.size() << " ARICH photon detector modules will be placed.");
    }

    void ARICHGeometryPar::modulesPositionSimple(const GearDir& content)
    {
      BOOST_FOREACH(const GearDir & module, content.getNodes("Detector/Plane/Modules/Module")) {
        TVector2 position(module.getLength("xPos"), module.getLength("yPos"));
        double angle = module.getAngle("angle") / Unit::rad;
        _fFi.push_back(position.Phi());
        _fR.push_back(position.Mod());
        _fFiMod.push_back(angle);
      }
      B2INFO("Altogether " << _fR.size() << " ARICH photon detector modules will be placed.");
    }

    int ARICHGeometryPar::getCopyNo(TVector3 hit)
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
        if (fabs(r - _fR[copyno]) <  _modXSize / 2.) return copyno + 1;
        ntot += _ncol[i];
      }
      return -1;
    }

    TVector3 ARICHGeometryPar::getOrigin(int copyNo)
    {
      TVector2 origin;
      origin.SetMagPhi(_fR[copyNo - 1], _fFi[copyNo - 1]);
      return TVector3(origin.X(), origin.Y(), _detZpos + _modZSize / 2.);
    }

    G4ThreeVector ARICHGeometryPar::getOriginG4(int copyNo)
    {
      TVector3 origin = getOrigin(copyNo);
      return G4ThreeVector(origin.X() / Unit::mm, origin.Y() / Unit::mm, origin.Z() / Unit::mm);
    }

    double ARICHGeometryPar::getModAngle(int copyno)
    {
      return _fFiMod[copyno - 1];
    }

    void ARICHGeometryPar::chipLocPosition()
    {
      double xycenter =  _padSize * _nPadX / 4. + _chipGap / 2.;
      _chipLocPos.push_back(TVector2(xycenter - _padSize * _nPadX / 4., xycenter - _padSize * _nPadX / 4.));
      _chipLocPos.push_back(TVector2(xycenter - _padSize * _nPadX / 4., -xycenter - _padSize * _nPadX / 4.));
      _chipLocPos.push_back(TVector2(-xycenter - _padSize * _nPadX / 4., xycenter - _padSize * _nPadX / 4.));
      _chipLocPos.push_back(TVector2(-xycenter - _padSize * _nPadX / 4., -xycenter - _padSize * _nPadX / 4.));
    }


    int ARICHGeometryPar::getChipID(TVector2 locpos)
    {
      if (locpos.X() > 0) {
        if (locpos.Y() > 0) return 0;
        return 1;
      }
      if (locpos.Y() > 0) return 2;
      return 3;
    }


    TVector3 ARICHGeometryPar::getChannelCenterGlob(int modID, int chanID)
    {
      std::pair<int, int> ModChan;
      ModChan.first = modID - 1; ModChan.second = chanID;
      return _padWorldPositions[ModChan];
    }

    TVector2 ARICHGeometryPar::getChannelCenterLoc(int chID)
    {
      return _padLocPositions[chID];
    }


    void ARICHGeometryPar::padPositions()
    {
      int Npad = int(_nPadX / 2.);
      TVector2 xstart(_padSize / 2., _padSize / 2.);
      for (int chipID = 0; chipID < 4; chipID++) {
        TVector2 chipPos = getChipLocPos(chipID);
        for (int ix = 0; ix < Npad; ix++) {
          for (int iy = 0; iy < Npad; iy++) {
            int chanID = chipID * Npad * Npad + ix * Npad + iy;
            TVector2 center(_padSize / 2. + ix * _padSize, _padSize / 2. + iy * _padSize);
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
          iChanCenter = iChanCenter.Rotate(_fFiMod[iMod]);
          TVector3 iWorld((iModCenter + iChanCenter).X(), (iModCenter + iChanCenter).Y(), _detZpos + _winThick);
          std::pair<int, int> ModChan;
          ModChan.first = iMod; ModChan.second = iChan;
          _padWorldPositions[ModChan] = iWorld;
        }
      }
    }

    void ARICHGeometryPar::mirrorPositions()
    {
      double rmir = _mirrorOuterRad * cos(M_PI / _nMirrors) - _mirrorThickness;
      for (int i = 0; i < _nMirrors; i++) {
        TVector3 norm(cos(2.*M_PI / double(_nMirrors) * (i + 0.5) + _mirrorStartAng), sin(2.*M_PI / double(_nMirrors) * (i + 0.5) + _mirrorStartAng), 0);
        _mirrornorm.push_back(norm);
        _mirrorpoint.push_back(rmir * norm);
      }
    }

    void ARICHGeometryPar::mirrorPositionSimple(const GearDir& content)
    {
      double thick = content.getLength("Mirrors/thickness");
      BOOST_FOREACH(const GearDir & mirror, content.getNodes("Mirrors/Mirror")) {
        double angle = mirror.getAngle("angle");
        TVector3 point(mirror.getLength("xPos") - cos(angle)*thick / 2., mirror.getLength("yPos") - sin(angle)*thick / 2., 0);
        TVector3 norm(cos(angle), sin(angle), 0);
        _mirrorpoint.push_back(point);
        _mirrornorm.push_back(norm);
        _mirrorZPos = mirror.getLength("zPos");
        _nMirrors++;
      }
    }

    TVector3 ARICHGeometryPar::getMirrorNormal(int mirID)
    {
      return _mirrornorm[mirID];
    }

    TVector3 ARICHGeometryPar::getMirrorPoint(int mirID)
    {
      return _mirrorpoint[mirID];
    }

    void ARICHGeometryPar::setAeroTransLength(int layer, double trlength)
    {
      _aeroTrLength[layer] = trlength;
    }

    void ARICHGeometryPar::setAeroRefIndex(int layer, double n)
    {
      if (n) _aeroRefIndex[layer] = n;
    }

    void ARICHGeometryPar::setAerogelThickness(int layer, double thick)
    {
      _nRad++;
      _aeroThickness[layer] = thick;
    }

    void ARICHGeometryPar::setAerogelZPosition(int layer, double zPos)
    {
      _aeroZPosition[layer] = zPos;
    }

    void ARICHGeometryPar::setWindowRefIndex(double refInd)
    {
      _winRefInd = refInd;
    }

    void ARICHGeometryPar::initDetectorMask(int nmodules)
    {
      int m_DetectorMaskSize = m_nPads * nmodules / 32 + 1;
      for (int i = 0; i < m_DetectorMaskSize; i++) {
        m_DetectorMask.push_back(0xFFFFFFFF);
      }
      B2INFO("DetectorMask initialized size=" << m_nPads << " * " << nmodules << " +1 =" << m_DetectorMaskSize);

    }

    void ARICHGeometryPar::setActive(int module, int channel, bool active)
    {
      int ch  = module * m_nPads + channel;
      int bit = ch % 32;
      insigned int idx = ch / 32;
      if (idx >= m_DetectorMask.size()) {
        B2WARNING(idx  << " Wrong detector mask size >= " << m_DetectorMask.size());
      }
      if (active) m_DetectorMask[idx] |= (1 << bit);
      else        m_DetectorMask[idx] &= ~(1 << bit);

    }

    bool ARICHGeometryPar::isActive(int module, int channel)
    {
      int ch  = module * m_nPads + channel;
      int bit = ch % 32;
      int idx = ch / 32;
      return    m_DetectorMask[idx] & (1 << bit);
    }


  } // namespace arich
} // namespace Belle2
