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

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <geometry/bfieldmap/BFieldMap.h>

using namespace std;

namespace Belle2 {
  namespace TOP {

    bool TOPconfigure::m_configured(false);

    TOPconfigure::TOPconfigure() :
      m_topgp(TOPGeometryPar::Instance()),
      m_R1(0),
      m_R2(0),
      m_Z1(0),
      m_Z2(0),
      m_timeRange(0)

    {
      if (!m_topgp->isInitialized()) B2FATAL("TOPconfigure: no geometry available");
      m_topgp->setBasfUnits();

      // space for TOP modules
      m_R1 = m_topgp->getRadius() - m_topgp->getWextdown();
      double x = m_topgp->getQwidth() / 2.0;
      double y = m_topgp->getRadius() + m_topgp->getQthickness();
      m_R2 = sqrt(x * x + y * y);
      m_Z1 = m_topgp->getZ1() - m_topgp->getWLength();
      m_Z2 = m_topgp->getZ2();
      // TDC time range
      m_timeRange = (1 << m_topgp->getTDCbits()) * m_topgp->getTDCbitwidth();

      if (m_configured) return;

      setTOPvolume(m_R1, m_R2, m_Z1, m_Z2);

      // get magnetic field at TOP
      TVector3 point(0, m_topgp->getRadius(), 0);
      TVector3 Bfield = BFieldMap::Instance().getBField(point);
      setBfield(-Bfield.Z());

      setPMT(m_topgp->getMsizex(), m_topgp->getMsizey(),
             m_topgp->getAsizex(), m_topgp->getAsizey(),
             m_topgp->getNpadx(), m_topgp->getNpady());

      int ng = m_topgp->getNgaussTTS();
      double sigmaTDC = m_topgp->getELjitter();
      if (ng > 0) {
        vector<float> frac, mean, sigma;
        for (int i = 0; i < ng; i++) {
          frac.push_back((float) m_topgp->getTTSfrac(i));
          mean.push_back((float) m_topgp->getTTSmean(i));
          double sigmaTTS = m_topgp->getTTSsigma(i);
          sigma.push_back((float) sqrt(sigmaTTS * sigmaTTS + sigmaTDC * sigmaTDC));
        }
        setTTS(ng, frac.data(), mean.data(), sigma.data());
      }

      int size = m_topgp->getNpointsQE();
      if (size > 0) {
        vector<float> Wavelength, QE;
        for (int i = 0; i < size; i++) {
          QE.push_back((float) m_topgp->getQE(i));
          float wl = m_topgp->getLambdaFirst() + m_topgp->getLambdaStep() * i;
          Wavelength.push_back(wl);
        }
        setQE(Wavelength.data(), QE.data(), size,
              m_topgp->getColEffi() * m_topgp->getELefficiency());
      }

      setTDC(m_topgp->getTDCbits(), m_topgp->getTDCbitwidth(), m_topgp->getTDCoffset());
      setCFD(m_topgp->getPileupTime(), m_topgp->getDoubleHitResolution());

      int n = m_topgp->getNbars();           // number of modules
      double Dphi = 2 * M_PI / n;
      double Phi = m_topgp->getPhi0() - 0.5 * M_PI;

      double R = m_topgp->getRadius();          // innner bar surface radius
      double MirrR = m_topgp->getMirradius();   // Mirror radius
      double MirrXc = m_topgp->getMirposx();    // Mirror X center of curvature
      double MirrYc = m_topgp->getMirposy();    // Mirror Y center of curvature
      double A = m_topgp->getQwidth();          // bar width
      double B = m_topgp->getQthickness();      // bar thickness
      double z1 = m_topgp->getZ1();             // backward bar position
      double z2 = m_topgp->getZ2();             // forward bar position
      double DzExp = m_topgp->getWLength();     // expansion volume length
      double Wwidth = m_topgp->getWwidth();     // expansion volume width
      double Wflat = m_topgp->getWflat();     // expansion volume flat part
      double filterThick = m_topgp->getdGlue();
      double pmtWindow = m_topgp->getWinthickness();
      double YsizExp = m_topgp->getWextdown() + m_topgp->getQthickness();
      double XsizPMT = m_topgp->getNpmtx() * m_topgp->getMsizex() +
                       (m_topgp->getNpmtx() - 1) * m_topgp->getXgap();
      double YsizPMT = m_topgp->getNpmty() * m_topgp->getMsizey() + m_topgp->getYgap();
      double x0 = m_topgp->getPMToffsetX();
      double y0 = m_topgp->getPMToffsetY();

      //! No edge roughness
      setEdgeRoughness(0);

      for (int i = 0; i < n; i++) {
        int id = setQbar(A, B, z1, z2, R, 0, Phi, c_PMT, c_SphericM);
        setMirrorRadius(id, MirrR);
        setMirrorCenter(id, MirrXc, MirrYc);
        addExpansionVolume(id, c_Left, c_Prism, DzExp - Wflat, B / 2, B / 2 - YsizExp,
                           0, 0, Wwidth);
        setBBoxWindow(id, Wflat + filterThick + pmtWindow);
        arrangePMT(id, c_Left, XsizPMT, YsizPMT, x0, y0);
        Phi += Dphi;
      }

      m_configured = TOPfinalize(0);
      if (!m_configured) B2ERROR("TOPconfigure: configuration failed");

    }

  } // top namespace
} // Belle2 namespace

