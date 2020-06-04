/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/reconstruction/TOPconfigure.h>
#include <top/geometry/TOPGeometryPar.h>
#include <top/reconstruction/TOPf77fun.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/geometry/BFieldManager.h>

extern "C" {
  void top_geo_dump_();
  void set_mirror_refl_(int*, float*);
  void set_surface_refl_(int*, float*);
  void set_alignment_(int*, float*, float*, float*, float*, float*, float*);
  int set_tts_type_(int*, int*, float*, float*, float*);
  int set_pmt_type_(int*, int*, int*);
}

using namespace std;

namespace Belle2 {
  namespace TOP {

    bool TOPconfigure::m_configured(false);
    DBObjPtr<TOPCalModuleAlignment> TOPconfigure::m_alignment;
    OptionalDBArray<TOPPmtInstallation> TOPconfigure::m_pmtInstalled;

    TOPconfigure::TOPconfigure()
    {
      const auto* geo = TOPGeometryPar::Instance()->getGeometry();

      // space for TOP modules
      m_R1 = geo->getInnerRadius();
      m_R2 = geo->getOuterRadius();
      m_Z1 = geo->getBackwardZ();
      m_Z2 = geo->getForwardZ();

      // TDC time range
      const auto& tdc = geo->getNominalTDC();
      m_timeRange = tdc.getOverflowValue() * tdc.getBinWidth();

      if (m_configured) return;

      // space for the modules
      setTOPvolume(m_R1, m_R2, m_Z1, m_Z2);

      // magnetic field at TOP
      TVector3 Bfield = BFieldManager::getField(0, geo->getRadius(), 0) / Unit::T;
      setBfield(-Bfield.Z());

      // PMT dimensions: NOTE reconstruction assumes all modules have the same PMT's
      const auto& pmt = geo->getModule(1).getPMTArray().getPMT();
      setPMT(pmt.getSizeX(), pmt.getSizeY(),
             pmt.getSensSizeX(), pmt.getSensSizeY(),
             pmt.getNumColumns(), pmt.getNumRows());

      // Nominal TTS parameterization
      const auto& tts = geo->getNominalTTS().getTTS();
      std::vector<float> frac, mean, sigma;
      for (const auto& gauss : tts) {
        frac.push_back(gauss.fraction);
        mean.push_back(gauss.position);
        sigma.push_back(gauss.sigma);
      }
      setTTS(tts.size(), frac.data(), mean.data(), sigma.data());

      // PMT dependent TTS-es
      setPMTDependentTTSes();

      // pixel table of PMT types
      setPMTTypes();
      m_pmtInstalled.addCallback(&setPMTTypes);

      // quantum efficiency
      const auto& nominalQE = geo->getNominalQE();
      std::vector<float> wavelength;
      for (unsigned i = 0; i < nominalQE.getQE().size(); i++) {
        float wl = nominalQE.getLambdaFirst() + nominalQE.getLambdaStep() * i;
        wavelength.push_back(wl);
      }
      auto QE = nominalQE.getQE();
      // multiply QE with filter transmittance
      const auto& wavelengthFilter = geo->getWavelengthFilter();
      for (unsigned i = 0; i < QE.size(); i++) {
        QE[i] *= wavelengthFilter.getBulkTransmittance(wavelength[i]);
      }
      setQE(wavelength.data(), QE.data(), QE.size(),
            nominalQE.getCE() * tdc.getEfficiency());

      // time to digit conversion
      setTDC(tdc.getNumBits(), tdc.getBinWidth(), tdc.getOffset());
      setCFD(tdc.getPileupTime(), tdc.getDoubleHitResolution());

      setEdgeRoughness(0); // No edge roughness

      // geometry of modules
      for (unsigned i = 0; i < geo->getNumModules(); i++) {
        const auto& module = geo->getModule(i + 1);
        double A = module.getBarWidth();
        double B = module.getBarThickness();
        double z1 = module.getBackwardZ();
        double z2 = module.getForwardZ();
        double R = module.getInnerRadius();
        double phi = module.getPhi() - M_PI / 2;
        int id = setQbar(A, B, z1, z2, R, 0, phi, c_PMT, c_SphericM);
        setSurfaceReflectivity(id, module.getSurfaceReflectivity(3.0));

        const auto& mirror = module.getMirrorSegment();
        setMirrorRadius(id, mirror.getRadius());
        setMirrorCenter(id, mirror.getXc(), mirror.getYc());
        setMirrorReflectivity(id, mirror.getMirrorReflectivity(3.0));

        const auto& prism = module.getPrism();
        double prismLength = prism.getLength();
        double prismFlat = prism.getFlatLength();
        double prismExit = prism.getExitThickness();
        double prismWidth = prism.getWidth();
        addExpansionVolume(id, c_Left, c_Prism, prismLength - prismFlat,
                           B / 2, B / 2 - prismExit, 0, 0, prismWidth);

        const auto& pmtArray = module.getPMTArray();
        double pmtWindow = pmtArray.getPMT().getWinThickness();
        // a call to prism.getFilterThickness is added for backward compatibility
        double filterThickness = pmtArray.getFilterThickness() + prism.getFilterThickness();
        double cookieThickness = pmtArray.getCookieThickness();
        setBBoxWindow(id, prismFlat + cookieThickness + filterThickness + pmtWindow);

        double x0 = module.getPMTArrayDisplacement().getX();
        double y0 = module.getPMTArrayDisplacement().getY();
        arrangePMT(id, c_Left, pmtArray.getSizeX(), pmtArray.getSizeY(), x0, y0);
      }

      // alignment
      setAlignmentConstants();
      m_alignment.addCallback(&setAlignmentConstants);

      m_configured = TOPfinalize(0);
      if (!m_configured) B2ERROR("TOPconfigure: configuration failed");

    }

    void TOPconfigure::setAlignmentConstants()
    {
      if (!m_alignment.isValid()) {
        B2ERROR("TOPconfigure: No valid alignment for TOP found in database");
        return;
      }

      const auto* geo = TOPGeometryPar::Instance()->getGeometry();
      for (unsigned i = 0; i < geo->getNumModules(); i++) {
        int moduleID = i + 1;
        if (m_alignment->isCalibrated(moduleID)) {
          int id = i;
          float dx = m_alignment->getX(moduleID);
          float dy = m_alignment->getY(moduleID);
          float dz = m_alignment->getZ(moduleID);
          float alfa = m_alignment->getAlpha(moduleID);
          float beta = m_alignment->getBeta(moduleID);
          float gama = m_alignment->getGamma(moduleID);
          set_alignment_(&id, &dx, &dy, &dz, &alfa, &beta, &gama);
        }
      }

      B2INFO("TOPconfigure: Alignment constants are set in reconstruction");

    }

    void TOPconfigure::setPMTDependentTTSes()
    {
      const auto* geo = TOPGeometryPar::Instance()->getGeometry();
      const auto& ttses = geo->getTTSes();
      for (const auto& tts : ttses) {
        int type = tts.second.getPMTType();
        std::vector<float> frac, mean, sigma;
        for (const auto& gauss : tts.second.getTTS()) {
          frac.push_back(gauss.fraction);
          mean.push_back(gauss.position);
          sigma.push_back(gauss.sigma);
        }
        int size = frac.size();
        int err = set_tts_type_(&type, &size, frac.data(), mean.data(), sigma.data());
        if (err == 1) {
          B2ERROR("TOPconfigure: sum of TTS fractions is zero."
                  << LogVar("TTS type", type));
        } else if (err == 2) {
          B2ERROR("TOPconfigure: TTS type is out of range."
                  << LogVar("TTS type", type));
        }
      }
    }


    void TOPconfigure::setPMTTypes()
    {
      const auto* geo = TOPGeometryPar::Instance()->getGeometry();

      for (const auto& module : geo->getModules()) {
        auto moduleID = module.getModuleID();
        const auto& pmtArray = module.getPMTArray();
        const auto& pmt = pmtArray.getPMT();
        for (unsigned pmtID = 1; pmtID <= pmtArray.getSize(); pmtID++) {
          int pmtType = TOPGeometryPar::Instance()->getPMTType(moduleID, pmtID);
          for (unsigned pmtPixel = 1; pmtPixel <= pmt.getNumPixels(); pmtPixel++) {
            auto pixelID = pmtArray.getPixelID(pmtID, pmtPixel);
            int ich = pixelID - 1;
            int mdn = moduleID - 1;
            int err = set_pmt_type_(&mdn, &ich, &pmtType);
            if (err == 1) {
              B2ERROR("TOPconfigure: PMT type is out of range."
                      << LogVar("PMT type", pmtType));
            } else if (err == 2) {
              B2ERROR("TOPconfigure: invalid slot ID or pixel ID."
                      << LogVar("slot ID", moduleID)
                      << LogVar("pixel ID", pixelID));
            }
          }
        }
      }

      B2INFO("TOPconfigure: new map of PMT types has been passed to reconstruction");

    }

    void TOPconfigure::setTOPvolume(double R1, double R2, double Z1, double Z2)
    {
      float r1 = (float) R1; float r2 = (float) R2;
      float z1 = (float) Z1; float z2 = (float) Z2;
      set_topvol_(&r1, &r2, &z1, &z2);
    }

    void TOPconfigure::setBfield(double B)
    {
      float b = (float) B;
      set_bfield_(&b);
    }

    void TOPconfigure::setEdgeRoughness(double R)
    {
      float r = (float) R;
      set_qbar_redg_(&r);
    }

    void TOPconfigure::setPMT(double A, double B, double Asens, double Bsens,
                              int Nx, int Ny, double TTS)
    {
      float a = (float) A; float b = (float) B;
      float aa = (float) Asens; float bb = (float) Bsens;
      float tts = (float) TTS;
      set_pmt_(&a, &b, &aa, &bb, &Nx, &Ny, &tts);
    }

    void TOPconfigure::setTTS(int ng, float Frac[], float Mean[], float Sigma[])
    {
      set_tts_(&ng, Frac, Mean, Sigma);
    }

    void TOPconfigure::setQE(float Wavelength[], float QE[], int Size, double CE)
    {
      float ce = (float) CE;
      set_qeffi_(Wavelength, QE, &Size, &ce);
    }

    int TOPconfigure::setQbar(double A, double B, double Z1, double Z2, double R,
                              double Dx, double Phi, int Lside, int Rside)
    {
      float a = (float) A; float b = (float) B;
      float z1 = (float) Z1; float z2 = (float) Z2; float r = (float) R;
      float dx = (float) Dx; float phi = (float) Phi;
      int id = set_qbar_(&a, &b, &z1, &z2, &r, &dx, &phi, &Lside, &Rside);
      return id;
    }

    void TOPconfigure::setBBoxWindow(int moduleID, double thickness)
    {
      float d = thickness;
      set_bbwin_(&moduleID, &d);
    }

    void TOPconfigure::addExpansionVolume(int moduleID, int LR, int Shape, double Dz,
                                          double Yup, double Ydown,
                                          double YupIn, double YdownIn,
                                          double Width, double DzRefl, double Refl)
    {
      float dz = Dz; float yup = Yup; float ydn = Ydown; float a = Width;
      float yupi = YupIn; float ydni = YdownIn;
      float dzrefl = DzRefl; float refl = Refl;
      set_extvol_(&moduleID, &LR, &Shape, &dz, &yup, &ydn, &a, &yupi, &ydni,
                  &dzrefl, &refl);
    }

    void TOPconfigure::arrangePMT(int moduleID, double sizX, double sizY,
                                  double Dx, double Dy)
    {
      float sizx = (float) sizX; float sizy = (float) sizY;
      float dx = (float) Dx; float dy = (float) Dy;
      int LR = 0;
      arrange_pmt_(&moduleID, &LR, &sizx, &sizy, &dx, &dy);
      LR = 1;
      arrange_pmt_(&moduleID, &LR, &sizx, &sizy, &dx, &dy);
    }

    void TOPconfigure::arrangePMT(int moduleID, int LR, double sizX, double sizY,
                                  double Dx, double Dy)
    {
      float sizx = (float) sizX; float sizy = (float) sizY;
      float dx = (float) Dx; float dy = (float) Dy;
      arrange_pmt_(&moduleID, &LR, &sizx, &sizy, &dx, &dy);
    }

    void TOPconfigure::setMirrorRadius(int moduleID, double R)
    {
      float r = (float) R;
      set_rmi_(&moduleID, &r);
    }

    void TOPconfigure::setMirrorCenter(int moduleID, double Xc, double Yc)
    {
      float xc = (float) Xc; float yc = (float) Yc;
      set_xyc_(&moduleID, &xc, &yc);
    }

    void TOPconfigure::setMirrorReflectivity(int moduleID, double reflectivity)
    {
      float refl = (float) reflectivity;
      if (refl > 1) {
        refl = 1;
        B2WARNING("TOPconfigure: mirror reflectivity greater than 1 -> set to 1 in reconstruction");
      }
      set_mirror_refl_(&moduleID, &refl);
    }

    void TOPconfigure::setSurfaceReflectivity(int moduleID, double reflectivity)
    {
      float refl = (float) reflectivity;
      if (refl > 1) {
        refl = 1;
        B2WARNING("TOPconfigure: surface reflectivity greater than 1 -> set to 1 in reconstruction");
      }
      set_surface_refl_(&moduleID, &refl);
    }

    void TOPconfigure::setTDC(int NBIT, double ChWid, double Offset)
    {
      float chwid = (float) ChWid; float offset = (float) Offset;
      set_tdc_(&NBIT, &chwid, &offset);
    }

    void TOPconfigure::setCFD(double delTpileup, double delTdoubleHit)
    {
      float dt = delTpileup; float delt = delTdoubleHit;
      set_cfd_(&dt, &delt);
    }

    bool TOPconfigure::TOPfinalize(int Dump)
    {
      return top_geo_finalize_(&Dump);
    }

    void TOPconfigure::print() const
    {
      top_geo_dump_();
    }

    void TOPconfigure::getQbar(int moduleID, double& A, double& B, double& C,
                               int& Lside, int& Rside, int& LexpvID, int& RexpvID)
    {
      moduleID--;
      float a, b, c;
      get_qbar_(&moduleID, &a, &b, &c, &Lside, &Rside, &LexpvID, &RexpvID);
      A = a; B = b; C = c;
    }

    void TOPconfigure::getExpansionVolume(int expvID, double& Dz, double& Yup, double& Ydown,
                                          double& YupIn, double& YdownIn,
                                          double& Width, int& Shape, int& LR)
    {
      float dz, yup, ydn, a, yupi, ydni;
      get_expvol_(&expvID, &dz, &yup, &ydn, &yupi, &ydni, &a, &Shape, &LR);
      Dz = dz; Yup = yup; Ydown = ydn; YupIn = yupi; YdownIn = ydni; Width = a;
    }

    void TOPconfigure::getMirror(int moduleID, double& R, double& Xc, double& Yc, double& Zc,
                                 double& F)
    {
      moduleID--;
      float r, xc, yc, zc, f;
      get_mirror_(&moduleID, &r, &xc, &yc, &zc, &f);
      R = r; Xc = xc; Yc = yc; Zc = zc; F = f;
    }

    double TOPconfigure::getBBoxWindow(int moduleID)
    {
      moduleID--;
      return get_bbwin_(&moduleID);
    }

  } // top namespace
} // Belle2 namespace

