/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <top/geometry/TOPGeometryPar.h>

#include <cmath>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <stdlib.h>

using namespace std;
using namespace boost;

namespace Belle2 {

  namespace TOP {

    TOPGeometryPar* TOPGeometryPar::p_B4TOPGeometryParDB = 0;

    TOPGeometryPar* TOPGeometryPar::Instance()
    {
      if (!p_B4TOPGeometryParDB) {
        p_B4TOPGeometryParDB = new TOPGeometryPar();
      }
      return p_B4TOPGeometryParDB;
    }

    TOPGeometryPar::TOPGeometryPar()
    {
      clear();
    }

    TOPGeometryPar::~TOPGeometryPar()
    {
    }

    void TOPGeometryPar::Initialize(const GearDir& content)
    {
      read(content);
    }

    void TOPGeometryPar::clear(void)
    {

      //! Bars
      m_Nbars = 0;
      m_Radius = 0;
      m_phi0 = 0;
      m_Qwidth = 0;
      m_Qthickness = 0;
      m_Bposition = 0;
      m_Length1 = 0;
      m_Length2 = 0;
      m_Length3 = 0;
      m_WLength = 0;
      m_Wwidth = 0;
      m_Wextdown = 0;
      m_Wflat = 0;
      m_Gwidth1 = 0;
      m_Gwidth2 = 0;
      m_Gwidth3 = 0;

      //! PMT's
      m_Xgap = 0;
      m_Ygap = 0;
      m_Npmtx = 0;
      m_Npmty = 0;
      m_Msizex = 0;
      m_Msizey = 0;
      m_Msizez = 0;
      m_MWallThickness = 0;
      m_Asizex = 0;
      m_Asizey = 0;
      m_Asizez = 0;
      m_Winthickness = 0;
      m_Botthickness = 0;
      m_Npadx = 0;
      m_Npady = 0;
      m_padx = 0;
      m_pady = 0;
      m_AsizexHalf = 0;
      m_AsizeyHalf = 0;
      m_dGlue = 0;
      m_pmtOffsetX = 0;
      m_pmtOffsetY = 0;

      //! TDC
      m_NTDC = 0;
      m_TDCwidth = 0;

      //! electronics jitter and efficiency (not in xml!)
      m_ELjitter = 0;
      m_ELefficiency = 1.0;

      //! TTS
      m_NgaussTTS = 0;
      for (int i = 0; i < MAXPTS_TTS; i++) {
        m_TTSfrac[i] = 0;
        m_TTSmean[i] = 0;
        m_TTSsigma[i] = 0;
      }

      //! QE
      m_ColEffi = 0;
      m_LambdaFirst = 0;
      m_LambdaStep = 0;
      m_NpointsQE = 0;
      for (int i = 0; i < MAXPTS_QE; i++) {m_QE[i] = 0;}

      //! Support structure
      m_PannelThickness = 0;
      m_PlateThickness = 0;
      m_LowerGap = 0;
      m_UpperGap = 0;
      m_SideGap = 0;
      m_ZForward = 0;
      m_ZBackward = 0;

      //! Mirror
      m_Mirposx = 0;
      m_Mirposy = 0;
      m_Mirthickness = 0;
      m_Mirradius = 0;

      //! Other

      m_unit = Unit::cm;

    }

    void TOPGeometryPar::read(const GearDir& content)
    {
      //! Bars

      GearDir barParams(content, "Bars");
      m_Nbars = barParams.getInt("Nbar");
      m_Radius = barParams.getLength("Radius");
      m_phi0 = barParams.getAngle("Phi0");
      m_Qwidth = barParams.getLength("QWidth");
      m_Qthickness = barParams.getLength("QThickness");
      m_Bposition = barParams.getLength("QZBackward");
      m_Length1 = barParams.getLength("QBar1Length");
      m_Length2 = barParams.getLength("QBar2Length");
      m_Length3 = barParams.getLength("QBarMirror");
      m_WLength = barParams.getLength("QWedgeLength");
      m_Wwidth = barParams.getLength("QWedgeWidth");
      m_Wextdown = barParams.getLength("QWedgeDown");
      m_Wflat = barParams.getLength("QWedgeFlat");
      m_Gwidth1 = barParams.getLength("Glue/Thicknes1");
      m_Gwidth2 = barParams.getLength("Glue/Thicknes2");
      m_Gwidth3 = barParams.getLength("Glue/Thicknes3");

      //! PMT's

      GearDir detParams(content, "PMTs");
      m_Xgap = detParams.getLength("Xgap");
      m_Ygap = detParams.getLength("Ygap");
      m_Npmtx = detParams.getInt("nPMTx");
      m_Npmty = detParams.getInt("nPMTy");
      m_Msizex = detParams.getLength("Module/ModuleXSize");
      m_Msizey = detParams.getLength("Module/ModuleYSize");
      m_Msizez = detParams.getLength("Module/ModuleZSize");
      m_MWallThickness = detParams.getLength("Module/ModuleWall");
      m_Asizex = detParams.getLength("Module/SensXSize");
      m_Asizey = detParams.getLength("Module/SensYSize");
      m_Asizez = detParams.getLength("Module/SensThickness");
      m_Winthickness = detParams.getLength("Module/WindowThickness");
      m_Botthickness = detParams.getLength("Module/BottomThickness");
      m_Npadx = detParams.getInt("Module/PadXNum");
      m_Npady = detParams.getInt("Module/PadYNum");
      m_dGlue = detParams.getLength("dGlue");
      m_pmtOffsetX = detParams.getLength("offsetX", 0.0);
      m_pmtOffsetY = detParams.getLength("offsetY", 0.0);
      m_padx = m_Asizex / (double)m_Npadx;
      m_pady = m_Asizey / (double)m_Npady;
      m_AsizexHalf = m_Asizex / 2;
      m_AsizeyHalf = m_Asizey / 2;


      //! TDC

      m_NTDC = detParams.getInt("Module/TDCbits");
      m_TDCwidth = detParams.getTime("Module/TDCbitwidth");

      //! TTS

      m_NgaussTTS = 0;
      for (int i = 0; i < MAXPTS_TTS; i++) {
        int ii = i + 1;
        stringstream ss; string cc;
        ss << ii; ss >> cc;
        string path = "TTS/Gauss[@component='term-" + cc + "']/";
        GearDir tts(detParams, path);
        if (!tts) break;
        m_NgaussTTS++;
        m_TTSfrac[i] = tts.getDouble("fraction");
        m_TTSmean[i] = tts.getTime("mean");
        m_TTSsigma[i] = tts.getTime("sigma");
      }
      double fracsum = 0;
      for (int i = 0; i < m_NgaussTTS; i++) {fracsum += m_TTSfrac[i];}
      if (fracsum > 0) {
        for (int i = 0; i < m_NgaussTTS; i++) {m_TTSfrac[i] /= fracsum;}
        B2INFO("TOPGeometryPar: TTS defined with " << m_NgaussTTS << " Gaussian terms");
      } else {
        m_NgaussTTS = 0;
        B2ERROR("TOPGeometryPar: TTS - sum of fractions =0 -> Ngauss set to 0");
      }

      //! quantum & collection efficiency

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

      //! Mirror

      GearDir mirParams(content, "Mirror");
      m_Mirposx = mirParams.getLength("Xpos");
      m_Mirposy = mirParams.getLength("Ypos");
      m_Mirthickness = mirParams.getLength("mirrorThickness");
      m_Mirradius = mirParams.getLength("Radius");

      //! Support structure

      GearDir supParams(content, "Support");
      m_PannelThickness = supParams.getLength("PannelThickness");
      m_PlateThickness = supParams.getLength("PlateThickness");
      m_LowerGap = supParams.getLength("lowerGap");
      m_UpperGap = supParams.getLength("upperGap");
      m_SideGap = supParams.getLength("sideGap");
      double forwardGap = supParams.getLength("forwardGap");
      m_ZForward = m_Bposition + m_Length1 + m_Gwidth2 + m_Length2 +
                   m_Gwidth3 + m_Length3 +
                   m_Mirthickness + forwardGap +  m_PannelThickness;
      double backGap = supParams.getLength("backGap");
      m_ZBackward = m_Bposition - m_Gwidth1 - m_WLength - backGap - m_PannelThickness;

      //! store alignment directory

      m_alignment = GearDir(content, "Alignment/");

    }

    double TOPGeometryPar::QE(double e) const
    {
      if (e < 0.001) return 0;
      double dlam = 1240 / e - m_LambdaFirst;
      if (dlam < 0) return 0;
      int i = int(dlam / m_LambdaStep);
      if (i > m_NpointsQE - 2) return 0;
      return m_QE[i] + (m_QE[i + 1] - m_QE[i]) / m_LambdaStep * (dlam - i * m_LambdaStep);
    }

    int TOPGeometryPar::getChannelID(double x, double y, int pmtID) const
    {
      if (fabs(x) >= m_AsizexHalf) return 0;
      if (fabs(y) >= m_AsizeyHalf) return 0;

      int ix = int((x + m_AsizexHalf) / m_padx);
      int iy = int((y + m_AsizeyHalf) / m_pady);
      int pmtch = ix + m_Npadx * iy;
      int chID = pmtch + (pmtID - 1) * m_Npadx * m_Npady + 1;
      return chID;
    }


    G4Transform3D TOPGeometryPar::getAlignment(const string& component)
    {
      //! Format the path using BOOST
      string path = (boost::format("Align[@component='%1%']/") % component).str();
      //! Appendt path to alignement path
      GearDir params(m_alignment, path);
      //! Check if parameter exists
      if (!params) {
        B2WARNING("Could not find alignment parameters for component " << component);
        return G4Transform3D();
      }
      //! Read the translations
      double dU = params.getLength("du") / m_unit;
      double dV = params.getLength("dv") / m_unit;
      double dW = params.getLength("dw") / m_unit;
      //! Read the rotations
      double alpha = params.getAngle("alpha");
      double beta  = params.getAngle("beta");
      double gamma = params.getAngle("gamma");
      //! Combine rotations and tralstions
      G4RotationMatrix rotation(alpha, beta, gamma);
      G4ThreeVector translation(dU, dV, dW);
      //! Return combine matrix
      return G4Transform3D(rotation, translation);
    }


    void TOPGeometryPar::Print(void) const
    {
      //! Here to be added the printout of the parameters
    }

  } //! End namespace TOP
} //! End namespace Belle2
