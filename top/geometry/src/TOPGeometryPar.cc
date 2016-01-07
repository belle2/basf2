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
#include <stdlib.h>

using namespace std;
using namespace boost;

namespace Belle2 {

  namespace TOP {

    TOPGeometryPar* TOPGeometryPar::s_instance = 0;

    TOPGeometryPar* TOPGeometryPar::Instance()
    {
      if (!s_instance) {
        s_instance = new TOPGeometryPar();
      }
      return s_instance;
    }

    void TOPGeometryPar::Initialize(const GearDir& content)
    {
      read(content);

      GearDir frontEndMapping(content, "FrontEndMapping");
      m_frontEndMapper.initialize(frontEndMapping);

      GearDir channelMapping0(content, "ChannelMapping[@type='IRS3B']");
      m_channelMapperIRS3B.initialize(channelMapping0);

      GearDir channelMapping1(content, "ChannelMapping[@type='IRSX']");
      m_channelMapperIRSX.initialize(channelMapping1);

      m_initialized = true;
    }

    void TOPGeometryPar::read(const GearDir& content)
    {
      // Bars

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
      m_SigmaAlpha = barParams.getDouble("Surface/SigmaAlpha");

      double saved_unit = m_unit;
      m_unit = Unit::cm;
      double phi = m_phi0 - M_PI / 2.0;
      double Dphi = 2 * M_PI / m_Nbars;
      for (int i = 0; i < m_Nbars; i++) {
        TOPQbar bar(m_Qwidth, m_Qthickness, getZ1(), getZ2(), m_Radius, 0.0, phi, i + 1);
        m_bars.push_back(bar);
        phi += Dphi;
      }
      m_unit = saved_unit;

      // PMT's

      GearDir pmtParams(content, "PMTs");
      m_Xgap = pmtParams.getLength("Xgap");
      m_Ygap = pmtParams.getLength("Ygap");
      m_Npmtx = pmtParams.getInt("nPMTx");
      m_Npmty = pmtParams.getInt("nPMTy");
      m_Msizex = pmtParams.getLength("Module/ModuleXSize");
      m_Msizey = pmtParams.getLength("Module/ModuleYSize");
      m_Msizez = pmtParams.getLength("Module/ModuleZSize");
      m_MWallThickness = pmtParams.getLength("Module/ModuleWall");
      m_Asizex = pmtParams.getLength("Module/SensXSize");
      m_Asizey = pmtParams.getLength("Module/SensYSize");
      m_Asizez = pmtParams.getLength("Module/SensThickness");
      m_Winthickness = pmtParams.getLength("Module/WindowThickness");
      m_Botthickness = pmtParams.getLength("Module/BottomThickness");
      m_Npadx = pmtParams.getInt("Module/PadXNum");
      m_Npady = pmtParams.getInt("Module/PadYNum");
      m_dGlue = pmtParams.getLength("dGlue");
      m_pmtOffsetX = pmtParams.getLength("offsetX", 0.0);
      m_pmtOffsetY = pmtParams.getLength("offsetY", 0.0);
      m_padx = m_Asizex / (double)m_Npadx;
      m_pady = m_Asizey / (double)m_Npady;
      m_AsizexHalf = m_Asizex / 2;
      m_AsizeyHalf = m_Asizey / 2;

      // TDC

      GearDir tdcParams(content, "TDC");
      if (tdcParams) {
        int numWindows = tdcParams.getInt("numWindows");
        int numSamples = numWindows * c_WindowSize;
        int subBits = tdcParams.getInt("subBits");
        m_NTDC = subBits - 1;
        int k = numSamples;
        do {
          m_NTDC++;
          k /= 2;
        } while (k > 0);
        double samplingRate = tdcParams.getDouble("samplingRate");
        m_TDCwidth = 1.0 / samplingRate / (1 << subBits);
        m_TDCoffset = tdcParams.getTime("offset");
        m_pileupTime = tdcParams.getTime("pileupTime");
        m_doubleHitResolution = tdcParams.getTime("doubleHitResolution");
      } else {
        m_NTDC = pmtParams.getInt("Module/TDCbits");
        m_TDCwidth = pmtParams.getTime("Module/TDCbitwidth");
        m_TDCoffset = pmtParams.getTime("Module/TDCoffset", 0);
        m_pileupTime = pmtParams.getTime("Module/TDCpileupTime", 0);
        m_doubleHitResolution = pmtParams.getTime("Module/TDCdoubleHitResolution", 0);
      }

      // TTS

      GearDir ttsParams(pmtParams, "TTS");
      for (const GearDir& Gauss : ttsParams.getNodes("Gauss")) {
        m_TTSfrac.push_back(Gauss.getDouble("fraction"));
        m_TTSmean.push_back(Gauss.getTime("mean"));
        m_TTSsigma.push_back(Gauss.getTime("sigma"));
      }
      double fracsum = 0;
      for (unsigned i = 0; i < m_TTSfrac.size(); i++) {fracsum += m_TTSfrac[i];}
      if (fracsum > 0) {
        for (unsigned i = 0; i < m_TTSfrac.size(); i++) {m_TTSfrac[i] /= fracsum;}
        B2INFO("TOPGeometryPar: TTS defined with " << getNgaussTTS() << " Gaussian terms");
      } else {
        B2ERROR("TOPGeometryPar: TTS - sum of fractions = 0");
      }

      // quantum & collection efficiency

      GearDir qeParams(content, "QE");
      m_ColEffi = qeParams.getDouble("ColEffi");
      m_LambdaFirst = qeParams.getLength("LambdaFirst") / Unit::nm;
      m_LambdaStep = qeParams.getLength("LambdaStep") / Unit::nm;
      for (const GearDir& Qeffi : qeParams.getNodes("Qeffi")) {
        m_QE.push_back(Qeffi.getDouble(""));
      }

      // Mirror

      GearDir mirParams(content, "Mirror");
      m_Mirposx = mirParams.getLength("Xpos");
      m_Mirposy = mirParams.getLength("Ypos");
      m_Mirthickness = mirParams.getLength("mirrorThickness");
      m_Mirradius = mirParams.getLength("Radius");

      // Support structure

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

      // store alignment directory

      m_alignment = GearDir(content, "Alignment/");

    }

    double TOPGeometryPar::QE(double e) const
    {
      if (e < 0.001) return 0;
      double dlam = 1240 / e - m_LambdaFirst;
      if (dlam < 0) return 0;
      unsigned int i = int(dlam / m_LambdaStep);
      if (i > m_QE.size() - 2) return 0;
      return m_QE[i] + (m_QE[i + 1] - m_QE[i]) / m_LambdaStep * (dlam - i * m_LambdaStep);
    }

    int TOPGeometryPar::getPixelID(double x, double y, int pmtID) const
    {
      if (fabs(x) >= m_AsizexHalf) return 0;
      if (fabs(y) >= m_AsizeyHalf) return 0;

      // PMT channel
      int ix = m_Npadx - 1 - int((x + m_AsizexHalf) / m_padx); // runs opposite to x
      int iy = int((y + m_AsizeyHalf) / m_pady);

      // PMT
      pmtID--;
      int ipmtx = pmtID % m_Npmtx;
      int ipmty = pmtID / m_Npmtx;

      // pixel number
      int i = ix + ipmtx * m_Npadx;
      int j = iy + ipmty * m_Npady;
      int nx = m_Npmtx * m_Npadx;
      return i + j * nx + 1;  // 1-based ID
    }


    int TOPGeometryPar::getOldNumbering(int pixelID) const
    {
      if (pixelID == 0) return 0;

      pixelID--;
      int nx = m_Npmtx * m_Npadx;
      int i = pixelID % nx;
      int j = pixelID / nx;
      int ix = i % m_Npadx;
      int ipmtx = i / m_Npadx;
      int iy = j % m_Npady;
      int ipmty = j / m_Npady;

      ix = m_Npadx - 1 - ix;
      ipmtx = m_Npmtx - 1 - ipmtx;

      return ix + m_Npadx * (iy + m_Npady * (ipmtx + m_Npmtx * ipmty)) + 1;

    }


    int TOPGeometryPar::getNewNumbering(int pixelID) const
    {
      if (pixelID == 0) return 0;

      pixelID--;
      int ix = pixelID % m_Npadx;
      pixelID /= m_Npadx;
      int iy = pixelID % m_Npady;
      pixelID /= m_Npady;
      int ipmtx = pixelID % m_Npmtx;
      int ipmty = pixelID / m_Npmtx;

      ix = m_Npadx - 1 - ix;
      ipmtx = m_Npmtx - 1 - ipmtx;

      int i = ix + ipmtx * m_Npadx;
      int j = iy + ipmty * m_Npady;
      int nx = m_Npmtx * m_Npadx;
      return i + j * nx + 1;

    }


    G4Transform3D TOPGeometryPar::getAlignment(const string& component)
    {
      // Format the path using BOOST
      string path = (boost::format("Align[@component='%1%']/") % component).str();
      // Appendt path to alignement path
      GearDir params(m_alignment, path);
      // Check if parameter exists
      if (!params) {
        B2WARNING("Could not find alignment parameters for component " << component);
        return G4Transform3D();
      }
      // Read the translations
      double dU = params.getLength("du") / m_unit;
      double dV = params.getLength("dv") / m_unit;
      double dW = params.getLength("dw") / m_unit;
      // Read the rotations
      double alpha = params.getAngle("alpha");
      double beta  = params.getAngle("beta");
      double gamma = params.getAngle("gamma");
      // Combine rotations and tralstions
      G4RotationMatrix rotation(alpha, beta, gamma);
      G4ThreeVector translation(dU, dV, dW);
      // Return combine matrix
      return G4Transform3D(rotation, translation);
    }

  } // End namespace TOP
} // End namespace Belle2
