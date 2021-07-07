/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <arich/geometry/ARICHGeometryPar.h>

#include <cmath>
#include <boost/format.hpp>
#include <boost/foreach.hpp>

using namespace std;
using namespace boost;

namespace Belle2 {

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
      mirrorPositions(content);
      frontEndMapping(content);
    }
    chipLocPosition();
    padPositions();
    initDetectorMask(m_fR.size());
    readModuleInfo(content);
    m_init = true;
  }

  void ARICHGeometryPar::Initialize(const GearDir& content)
  {
    Initialize(content, content);
  }

  void ARICHGeometryPar::clear(void)
  {
    m_chipGap = 0.0;
    m_detZpos = 0.0;
    m_modXSize = 0.0;
    m_modZSize = 0.0;
    m_winThick = 0.0;
    m_winRefInd = 0.0;
    m_mirrorOuterRad = 0.0;
    m_mirrorThickness = 0.0;
    m_mirrorStartAng = 0.0;
    m_nMirrors = 0;
    m_mirrorZPos = 0.0;
    m_nPadX = 0;
    m_padSize = 0.0;
    m_detInnerRadius = 0.0;
    m_detOuterRadius = 0.0;
    m_nrow = 0;
    m_nRad = 0;
    m_init = false;
    m_simple = false;
    m_ncol.clear(); m_fDFi.clear(); m_fDR.clear(); m_fR.clear();
    m_fFi.clear(); m_fFiMod.clear(); m_chipLocPos.clear(); m_padWorldPositions.clear(); m_mirrornorm.clear(); m_mirrorpoint.clear();
    m_ChannelQE.clear();
    for (int i = 0; i < MAX_N_ALAYERS; i++) {
      m_aeroTrLength[i] = 0; m_aeroRefIndex[i] = 0;
      m_aeroZPosition[i] = 0; m_aeroThickness[i] = 0;
    }
    m_nPads = 0;
    m_ColEffi = 0.;
    m_LambdaFirst = 0.;
    m_LambdaStep = 0.;
    m_NpointsQE = 0;
    m_qeScale = 0.;
    m_windowAbsorbtion = 0.;
    m_chipNegativeCrosstalk = 0;
    for (int i = 0; i < MAXPTS_QE; i++) {m_QE[i] = 0.;}

    m_tileNr = 0;
    m_tileGap = 0.;
    m_aeroRin = 0.;
    m_aeroRout = 0.;
    for (int i = 0; i < 5; i++) m_tileNphi[i] = 0;

  }

  void ARICHGeometryPar::read(const GearDir& content)
  {

    //------------------------------
    // Get ARICH geometry parameters
    //------------------------------
    GearDir detParams(content, "Detector");
    m_modXSize = detParams.getLength("Module/moduleXSize");
    m_modZSize = detParams.getLength("Module/moduleZSize");
    m_winThick = detParams.getLength("Module/windowThickness");
    m_nPadX = detParams.getInt("Module/padXNum");
    m_nPads = m_nPadX * m_nPadX;
    m_padSize = detParams.getLength("Module/padSize");
    m_chipGap = detParams.getLength("Module/chipGap");
    m_detZpos = detParams.getLength("Plane/zPosition");
    m_qeScale = detParams.getDouble("Module/qeScale");
    m_windowAbsorbtion = detParams.getDouble("Module/windowAbsorbtion");
    m_chipNegativeCrosstalk = detParams.getDouble("Module/chipNegativeCrosstalk");
    string Type = content.getString("@type", "");
    if (Type == "beamtest") return;
    m_detInnerRadius = detParams.getLength("Plane/tubeInnerRadius");
    m_detOuterRadius = detParams.getLength("Plane/tubeOuterRadius");

    GearDir mirrParams(content, "Mirrors");
    if (mirrParams) {
      m_nMirrors = mirrParams.getInt("nMirrors");
      m_mirrorThickness =  mirrParams.getLength("mirrorThickness");
      m_mirrorOuterRad = mirrParams.getLength("outerRadius");
      m_mirrorStartAng = mirrParams.getAngle("startAngle");
      m_mirrorZPos = mirrParams.getLength("Zposition");
    }

    GearDir qeParams(content, "QE");
    m_ColEffi = qeParams.getDouble("ColEffi");
    m_LambdaFirst = qeParams.getLength("LambdaFirst") / Unit::nm;
    m_LambdaStep = qeParams.getLength("LambdaStep") / Unit::nm;
    m_NpointsQE = 0;
    for (int i = 0; i < MAXPTS_QE; i++) {
      int ii = i + 1;
      stringstream ss; string cc;
      ss << ii; ss >> cc;
      string path = "Qeffi[@component='point-" + cc + "']/";
      GearDir qe(qeParams, path);
      if (!qe) break;
      m_NpointsQE++;
      m_QE[i] = qe.getDouble("qe");
    }

    GearDir aerogel(content, "Aerogel");
    m_tileNr = aerogel.getInt("tileNr");
    m_tileGap = aerogel.getLength("tileGap") / Unit::cm;
    m_aeroRin = aerogel.getLength("tubeInnerRadius") / Unit::cm;
    m_aeroRout = aerogel.getLength("tubeOuterRadius") / Unit::cm;

    int i = 0;
    for (const GearDir& ring : aerogel.getNodes("tileNphi/Ring")) {
      m_tileNphi[i] = ring.getInt();
      i++;
    }

  }

  void ARICHGeometryPar::frontEndMapping(const GearDir& content)
  {

    GearDir mapping(content, "FrontEndMapping");

    for (const GearDir& merger : mapping.getNodes("Merger")) {
      unsigned mergerID = (unsigned) merger.getInt("@id");

      auto testMer = m_mergerIDs.find(mergerID);
      if (testMer != m_mergerIDs.end()) {
        B2ERROR(mapping.getPath() << "/MergerID " << mergerID <<
                " ***input already used");
      }

      m_mergerIDs.insert(mergerID);

      std::vector<unsigned> boardIDs;
      for (const GearDir& board : merger.getNodes("FEboards/FEboard")) {
        unsigned boardID = board.getInt();
        auto testBor = m_boardIDs.find(boardID);
        if (testBor != m_boardIDs.end()) {
          B2ERROR(mapping.getPath() << "/FEboardID " << boardID <<
                  " ***input already used");
        }
        boardIDs.push_back(boardID);
        m_boardIDs.insert(boardID);
      }
      m_merger2feb.insert(std::pair<int, std::vector<unsigned>>(mergerID, boardIDs));
      unsigned copperID = (unsigned) merger.getInt("COPPERid");
      string finesseSlot = merger.getString("FinesseSlot");
      int finesse = 0;
      if (finesseSlot == "A") {finesse = 0;}
      else if (finesseSlot == "B") {finesse = 1;}
      else if (finesseSlot == "C") {finesse = 2;}
      else if (finesseSlot == "D") {finesse = 3;}
      else {
        B2ERROR(merger.getPath() << "/FinesseSlot " << finesseSlot <<
                " ***invalid slot (valid are A, B, C, D)");
        continue;
      }

      m_copperIDs.insert(copperID);

      std::pair<unsigned, int> copfin(copperID, finesse);
      m_copper2merger.insert(std::pair<std::pair<unsigned, int>, unsigned>(copfin, mergerID));

    }

  }

  int ARICHGeometryPar::getMergerFromCooper(int copperID, int finesse)
  {
    auto merger = m_copper2merger.find(std::pair<unsigned, int>(copperID, finesse));
    if (merger == m_copper2merger.end()) {
      // B2INFO("getMergerFromCooper: " << " copper " << copperID << ", finesse "
      //       << finesse << " is not assigned to any merger board");
      return 0;
    }

    return merger->second;
  }

  int ARICHGeometryPar::getBoardFromMerger(int mergerID, int slot)
  {
    auto boards = m_merger2feb.find(mergerID);
    if (boards == m_merger2feb.end()) {
      B2ERROR("getBoardFromMerger: " << " merger " << mergerID << " is not mapped");
      return -1;
    }
    if ((boards->second).size() <= unsigned(slot)) {
      B2ERROR("getBoardFromMerger: " << " merger " << mergerID << " slot " << slot << " is not assigned to FE board.");
      return -1;
    }
    return (boards->second).at(slot);
  }

  int ARICHGeometryPar::getNBoardsOnMerger(int mergerID)
  {
    auto boards = m_merger2feb.find(mergerID);
    if (boards == m_merger2feb.end()) {
      B2ERROR("getNBoardsOnMerger: " << " merger " << mergerID << " is not mapped");
    }
    return (boards->second).size();
  }

  void ARICHGeometryPar::readModuleInfo(const GearDir& content)
  {
    istringstream chstream;
    int ch; double qq;
    GearDir modParams(content, "ModuleInfo");
    uint8_t defqe = uint8_t(modParams.getDouble("DefaultQE") * 100);
    m_ChannelQE.assign(m_nPads * m_fR.size(), defqe);
    BOOST_FOREACH(const GearDir & module, modParams.getNodes("Module")) {
      int modid = atoi(module.getString("@id", "").c_str());
      chstream.str(module.getString("ChannelsQE"));
      while (chstream >> ch >> qq) {
        int chid = (modid - 1) * m_nPads + ch;
        m_ChannelQE[chid] = uint8_t(qq * 100);
      }
      chstream.clear();
      chstream.str(module.getString("DeadChannels"));
      while (chstream >> ch) {
        setActive(modid, ch, false);
      }
      chstream.clear();
    }
  }

  double ARICHGeometryPar::QE(double e) const
  {
    if (e < 0.001) return 0;
    double dlam = 1240 / e - m_LambdaFirst;
    if (dlam < 0) return 0;
    int i = int(dlam / m_LambdaStep);
    if (i > m_NpointsQE - 2) return 0;
    return m_QE[i] + (m_QE[i + 1] - m_QE[i]) / m_LambdaStep * (dlam - i * m_LambdaStep);
  }


  void ARICHGeometryPar::Print(void) const
  {
  }

  double ARICHGeometryPar::getChannelQE(int moduleID, int channelID)
  {
    int id = (moduleID - 1) * m_nPads + channelID;
    return m_ChannelQE.at(id) / 100.0;
  }

  int ARICHGeometryPar::getChannelID(TVector2 position)
  {
    int ChipID = getChipID(position);
    int Npad = int(m_nPadX / 2);
    TVector2 chipPos = getChipLocPos(ChipID);
    TVector2 locloc = position - chipPos;
    int ix = int(locloc.X() / m_padSize);
    int iy = int(locloc.Y() / m_padSize);
    if (locloc.X() < 0 || locloc.Y() < 0) return -1;
    if (ix > Npad - 1 || iy > Npad - 1) return -1;
    int chID = ChipID * Npad * Npad + iy + ix * Npad;
    return chID;
  }

  void ARICHGeometryPar::modulesPosition(const GearDir& content)
  {

    GearDir detParams(content, "Detector/Plane/Rings");

    double r = m_detInnerRadius;

    BOOST_FOREACH(const GearDir & ring, detParams.getNodes("Ring")) {
      double dR = ring.getLength("dR");
      r += dR;
      double rcenter = r + m_modXSize / 2.;
      if (rcenter + m_modXSize * sqrt(2) / 2. > m_detOuterRadius) {
        B2WARNING(m_ncol.size() + 1  << "th ring of ARICH photon detectors will not be placed (out of detector tube).");
        break;
      }
      m_nrow++;
      int nSeg = ring.getInt("nSegments") ;
      double dFi = ring.getLength("dFi");
      m_fDR.push_back(dR);
      double f = 2.*atan2((m_modXSize + dFi) / 2., r);
      int blaa = int(2.*M_PI / f / nSeg) * nSeg;
      m_ncol.push_back(blaa);
      f = 2.*M_PI / double(blaa);
      m_fDFi.push_back(f);
      B2INFO(blaa << " modules of " << m_ncol.size() << "th ring of ARICH photon detectors will be placed at r = " << rcenter << "cm. ");
      for (int nv = 0; nv < blaa; ++nv) {
        m_fR.push_back(rcenter);
        double fi = f * (nv + 0.5);
        m_fFi.push_back(fi);
        m_fFiMod.push_back(fi);
      }
      r += (m_modXSize  + r * (1 - cos(f / 2.)));
    }
    B2INFO("Altogether " << m_fR.size() << " ARICH photon detector modules will be placed.");
  }

  void ARICHGeometryPar::modulesPositionSimple(const GearDir& content)
  {
    BOOST_FOREACH(const GearDir & module, content.getNodes("Detector/Plane/Modules/Module")) {
      TVector2 position(module.getLength("xPos"), module.getLength("yPos"));
      double angle = module.getAngle("angle") / Unit::rad;
      m_fFi.push_back(position.Phi());
      m_fR.push_back(position.Mod());
      m_fFiMod.push_back(angle);
    }
    B2INFO("Altogether " << m_fR.size() << " ARICH photon detector modules will be placed.");
  }

  int ARICHGeometryPar::getCopyNo(TVector3 hit)
  {
    double x = hit.X();
    double y = hit.Y();
    double r = sqrt(x * x + y * y);
    double fi = atan2(y, x);
    if (fi < 0) fi += 2 * M_PI;
    int ntot = 0;
    for (int i = 0; i < m_nrow; i++) {
      int nfi = int(fi / m_fDFi[i]);
      int copyno = ntot + nfi;
      if (fabs(r - m_fR[copyno]) <  m_modXSize / 2.) return copyno + 1;
      ntot += m_ncol[i];
    }
    return -1;
  }

  TVector3 ARICHGeometryPar::getOrigin(int copyNo)
  {
    TVector2 origin;
    origin.SetMagPhi(m_fR[copyNo - 1], m_fFi[copyNo - 1]);
    return TVector3(origin.X(), origin.Y(), m_detZpos + m_modZSize / 2.);
  }

  G4ThreeVector ARICHGeometryPar::getOriginG4(int copyNo)
  {
    TVector3 origin = getOrigin(copyNo);
    return G4ThreeVector(origin.X() / Unit::mm, origin.Y() / Unit::mm, origin.Z() / Unit::mm);
  }

  double ARICHGeometryPar::getModAngle(int copyno)
  {
    return m_fFiMod[copyno - 1];
  }

  void ARICHGeometryPar::chipLocPosition()
  {
    double xycenter =  m_padSize * m_nPadX / 4. + m_chipGap / 2.;
    m_chipLocPos.push_back(TVector2(xycenter - m_padSize * m_nPadX / 4., xycenter - m_padSize * m_nPadX / 4.));
    m_chipLocPos.push_back(TVector2(xycenter - m_padSize * m_nPadX / 4., -xycenter - m_padSize * m_nPadX / 4.));
    m_chipLocPos.push_back(TVector2(-xycenter - m_padSize * m_nPadX / 4., xycenter - m_padSize * m_nPadX / 4.));
    m_chipLocPos.push_back(TVector2(-xycenter - m_padSize * m_nPadX / 4., -xycenter - m_padSize * m_nPadX / 4.));
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
    int id = (modID - 1) * m_nPads + chanID;
    return TVector3(m_padWorldPositions.at(id).X(), m_padWorldPositions.at(id).Y(), m_detZpos + m_winThick);
  }

  TVector2 ARICHGeometryPar::getChannelCenterLoc(int chID)
  {
    return m_padLocPositions[chID];
  }


  void ARICHGeometryPar::padPositions()
  {
    int Npad = int(m_nPadX / 2.);
    TVector2 xstart(m_padSize / 2., m_padSize / 2.);
    for (int chipID = 0; chipID < 4; chipID++) {
      TVector2 chipPos = getChipLocPos(chipID);
      for (int ix = 0; ix < Npad; ix++) {
        for (int iy = 0; iy < Npad; iy++) {
          int chanID = chipID * Npad * Npad + ix * Npad + iy;
          TVector2 center(m_padSize / 2. + ix * m_padSize, m_padSize / 2. + iy * m_padSize);
          center = center + chipPos;
          m_padLocPositions[chanID] = center;
        }
      }
    }
    for (int iMod = 0; iMod < getNMCopies(); iMod++) {
      for (unsigned int iChan = 0; iChan < m_padLocPositions.size(); iChan++) {
        TVector2 iModCenter;
        iModCenter.SetMagPhi(m_fR[iMod], m_fFi[iMod]);
        TVector2 iChanCenter = m_padLocPositions[iChan];
        iChanCenter = iChanCenter.Rotate(m_fFiMod[iMod]);
        TVector2 iWorld((iModCenter + iChanCenter).X(), (iModCenter + iChanCenter).Y());
        m_padWorldPositions.push_back(iWorld);
      }
    }
  }

  void ARICHGeometryPar::mirrorPositions(const GearDir& content)
  {
    double rmir = m_mirrorOuterRad * cos(M_PI / m_nMirrors) - m_mirrorThickness;
    for (int i = 0; i < m_nMirrors; i++) {
      TVector3 norm(cos(2.*M_PI / double(m_nMirrors) * (i + 0.5) + m_mirrorStartAng),
                    sin(2.*M_PI / double(m_nMirrors) * (i + 0.5) + m_mirrorStartAng), 0);
      m_mirrornorm.push_back(norm);
      m_mirrorpoint.push_back(rmir * norm);
    }
    readMirrorAlignment(content);
  }

  void ARICHGeometryPar::mirrorPositionSimple(const GearDir& content)
  {
    double thick = content.getLength("Mirrors/thickness");
    BOOST_FOREACH(const GearDir & mirror, content.getNodes("Mirrors/Mirror")) {
      double angle = mirror.getAngle("angle");
      TVector3 point(mirror.getLength("xPos") - cos(angle)*thick / 2., mirror.getLength("yPos") - sin(angle)*thick / 2., 0);
      TVector3 norm(cos(angle), sin(angle), 0);
      m_mirrorpoint.push_back(point);
      m_mirrornorm.push_back(norm);
      m_mirrorZPos = mirror.getLength("zPos");
      m_nMirrors++;
    }
  }

  TVector3 ARICHGeometryPar::getMirrorNormal(int mirID)
  {
    return m_mirrornorm[mirID];
  }

  TVector3 ARICHGeometryPar::getMirrorPoint(int mirID)
  {
    return m_mirrorpoint[mirID];
  }

  void ARICHGeometryPar::setAeroTransLength(int layer, double trlength)
  {
    m_aeroTrLength[layer] = trlength;
  }

  void ARICHGeometryPar::setAeroRefIndex(int layer, double n)
  {
    if (n) m_aeroRefIndex[layer] = n;
  }

  void ARICHGeometryPar::setAerogelThickness(int layer, double thick)
  {
    m_nRad++;
    m_aeroThickness[layer] = thick;
  }

  void ARICHGeometryPar::setAerogelZPosition(int layer, double zPos)
  {
    m_aeroZPosition[layer] = zPos;
  }

  void ARICHGeometryPar::setWindowRefIndex(double refInd)
  {
    m_winRefInd = refInd;
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
    int ch  = (module - 1) * m_nPads + channel;
    int bit = ch % 32;
    unsigned int idx = ch / 32;
    if (idx >= m_DetectorMask.size()) {
      B2WARNING(idx  << " Wrong detector mask size >= " << m_DetectorMask.size());
    }
    if (active) m_DetectorMask[idx] |= (1 << bit);
    else        m_DetectorMask[idx] &= ~(1 << bit);
  }

  bool ARICHGeometryPar::isActive(int module, int channel)
  {
    int ch  = (module - 1) * m_nPads + channel;
    int bit = ch % 32;
    int idx = ch / 32;
    return    m_DetectorMask[idx] & (1 << bit);
  }

  void ARICHGeometryPar::readMirrorAlignment(const GearDir& content)
  {
    GearDir modParams(content, "Mirrors/Alignment");

    BOOST_FOREACH(const GearDir & plate, modParams.getNodes("Plate")) {
      int id = atoi(plate.getString("@id").c_str());
      double dr = plate.getLength("dr");
      double dphi = plate.getAngle("dphi");
      double dtheta = plate.getAngle("dtheta");
      m_mirrorpoint[id - 1].SetMag(m_mirrorpoint[id - 1].Mag() + dr);
      m_mirrornorm[id - 1].SetTheta(m_mirrornorm[id - 1].Theta() + dtheta);
      m_mirrornorm[id - 1].SetPhi(m_mirrornorm[id - 1].Phi() + dphi);
    }
  }


  int ARICHGeometryPar::getAerogelTileID(TVector2 locpos)
  {

    double size = (m_aeroRout - m_aeroRin) / double(m_tileNr);
    double r = locpos.Mod();
    double phi = TVector2::Phi_0_2pi(locpos.Phi());
    int nr = int((r - m_aeroRin) / size);

    if (r < m_aeroRin + nr * size + m_tileGap / 2. || r >  m_aeroRin + (nr + 1)*size - m_tileGap / 2.) return 0;

    double dphi = 2.*M_PI / double(m_tileNphi[nr]);
    double gapPhi = m_tileGap / (m_aeroRin + (nr + 1) * size - m_tileGap / 2.);

    int nphi = int(phi / dphi);
    if (phi < nphi * dphi + gapPhi / 2. || phi > (nphi + 1)*dphi - gapPhi / 2.) return 0;

    int tileID = nphi;
    for (int i = 0; i < nr; i++) tileID += m_tileNphi[i];

    return tileID + 1;

  }


} // namespace Belle2
