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

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/geometry/BFieldManager.h>

using namespace std;

namespace Belle2 {
  namespace TOP {

    bool TOPconfigure::m_configured(false);

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

      // TTS parameterization
      const auto& tts = geo->getNominalTTS().getTTS();
      std::vector<float> frac, mean, sigma;
      for (const auto& gauss : tts) {
        frac.push_back(gauss.fraction);
        mean.push_back(gauss.position);
        sigma.push_back(gauss.sigma);
      }
      setTTS(tts.size(), frac.data(), mean.data(), sigma.data());

      // quantum efficiency
      const auto& nominalQE = geo->getNominalQE();
      std::vector<float> wavelength;
      for (unsigned i = 0; i < nominalQE.getQE().size(); i++) {
        float wl = nominalQE.getLambdaFirst() + nominalQE.getLambdaStep() * i;
        wavelength.push_back(wl);
      }
      auto QE = nominalQE.getQE();
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

        const auto& mirror = module.getMirrorSegment();
        setMirrorRadius(id, mirror.getRadius());
        setMirrorCenter(id, mirror.getXc(), mirror.getYc());

        const auto& prism = module.getPrism();
        double prismLength = prism.getLength();
        double prismFlat = prism.getFlatLength();
        double prismExit = prism.getExitThickness();
        double prismWidth = prism.getWidth();
        addExpansionVolume(id, c_Left, c_Prism, prismLength - prismFlat,
                           B / 2, B / 2 - prismExit, 0, 0, prismWidth);

        double filter = prism.getFilterThickness();
        const auto& pmtArray = module.getPMTArray();
        double pmtWindow = pmtArray.getPMT().getWinThickness();
        setBBoxWindow(id, prismFlat + filter + pmtWindow);

        double x0 = module.getPMTArrayDisplacement().getX();
        double y0 = module.getPMTArrayDisplacement().getY();
        arrangePMT(id, c_Left, pmtArray.getSizeX(), pmtArray.getSizeY(), x0, y0);
      }

      m_configured = TOPfinalize(0);
      if (!m_configured) B2ERROR("TOPconfigure: configuration failed");

    }

  } // top namespace
} // Belle2 namespace

