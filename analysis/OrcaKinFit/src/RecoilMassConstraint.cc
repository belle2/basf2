/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "analysis/OrcaKinFit/RecoilMassConstraint.h"
#include "analysis/OrcaKinFit/ParticleFitObject.h"

#include<iostream>
#include<cmath>

#undef NDEBUG
#include<cassert>

namespace Belle2 {
  namespace OrcaKinFit {

// constructor
    RecoilMassConstraint::RecoilMassConstraint(double recoilmass, double beampx, double beampy, double beampz, double beampe)
      : m_recoilMass(recoilmass), m_beamPx(beampx), m_beamPy(beampy), m_beamPz(beampz), m_beamE(beampe)
    {}

// destructor
    RecoilMassConstraint::~RecoilMassConstraint()
    {
      ;
    }

// calculate current value of constraint function
    double RecoilMassConstraint::getValue() const
    {
      double totE = 0.;
      double totpx = 0.;
      double totpy = 0.;
      double totpz = 0.;

      for (auto fitobject : fitobjects) {
        auto* pfo = dynamic_cast < ParticleFitObject* >(fitobject);
        assert(pfo);
        totE  += pfo->getE();
        totpx += pfo->getPx();
        totpy += pfo->getPy();
        totpz += pfo->getPz();
      }

      const double recoilE   = (m_beamE - totE);
      const double recoilE2  = recoilE * recoilE;
      const double recoilpx  = (m_beamPx - totpx);
      const double recoilpx2 = recoilpx * recoilpx;
      const double recoilpy  = (m_beamPy - totpy);
      const double recoilpy2 = recoilpy * recoilpy;
      const double recoilpz  = (m_beamPz - totpz);
      const double recoilpz2 = recoilpz * recoilpz;
      const double recoil2   = recoilE2 - recoilpx2 - recoilpy2 - recoilpz2;
      const double result    = recoil2 - m_recoilMass * m_recoilMass;
      return result;
    }

// calculate vector/array of derivatives of this constraint
// w.r.t. to ALL parameters of all fitobjects
// here: d RM2 /d par(j)
//          = d RM2 /d p(i) * d p(i) /d par(j)
//          =  -+2 * recoil_p(i) * d p(i) /d par(j)

    void RecoilMassConstraint::getDerivatives(int idim, double der[]) const
    {

      double totE = 0.;
      double totpx = 0.;
      double totpy = 0.;
      double totpz = 0.;

      for (auto fitobject : fitobjects) {
        auto* pfo = dynamic_cast < ParticleFitObject* >(fitobject);
        assert(pfo);
        totE  += pfo->getE();
        totpx += pfo->getPx();
        totpy += pfo->getPy();
        totpz += pfo->getPz();
      }

      const double recoilE   = (m_beamE - totE);
      const double recoilpx  = (m_beamPx - totpx);
      const double recoilpy  = (m_beamPy - totpy);
      const double recoilpz  = (m_beamPz - totpz);

      for (auto fitobject : fitobjects) {
        for (int ilocal = 0; ilocal < fitobject->getNPar(); ilocal++) {
          if (!fitobject->isParamFixed(ilocal)) {
            int iglobal = fitobject->getGlobalParNum(ilocal);
            assert(iglobal >= 0 && iglobal < idim);

            auto* pfo = dynamic_cast < ParticleFitObject* >(fitobject);
            assert(pfo);
            der[iglobal] = - recoilE  * pfo->getDE(ilocal)
                           + recoilpx * pfo->getDPx(ilocal)
                           + recoilpy * pfo->getDPy(ilocal)
                           + recoilpz * pfo->getDPz(ilocal);
            der[iglobal] *= 2;
          }
        }
      }
    }


    double RecoilMassConstraint::getRecoilMass()
    {
      double totE = 0.;
      double totpx = 0.;
      double totpy = 0.;
      double totpz = 0.;

      for (auto& fitobject : fitobjects) {
        auto* pfo = dynamic_cast < ParticleFitObject* >(fitobject);
        assert(pfo);
        totE  += pfo->getE();
        totpx += pfo->getPx();
        totpy += pfo->getPy();
        totpz += pfo->getPz();
      }

      const double recoilE   = (m_beamE - totE);
      const double recoilE2  = recoilE * recoilE;
      const double recoilpx  = (m_beamPx - totpx);
      const double recoilpx2 = recoilpx * recoilpx;
      const double recoilpy  = (m_beamPy - totpy);
      const double recoilpy2 = recoilpy * recoilpy;
      const double recoilpz  = (m_beamPz - totpz);
      const double recoilpz2 = recoilpz * recoilpz;
      const double recoil2   = recoilE2 - recoilpx2 - recoilpy2 - recoilpz2;
      const double recoil    = std::sqrt(std::fabs(recoil2));

      return recoil;
    }

    void RecoilMassConstraint::setRecoilMass(double recoilmass_)
    {
      m_recoilMass = recoilmass_;
    }

    bool RecoilMassConstraint::secondDerivatives(int i, int j, double* dderivatives) const
    {
      (void) i;
      (void) j;

      assert(dderivatives);
      for (int k = 0; k < 16; ++k) dderivatives[k] = 0;


      dderivatives[4 * 0 + 0] = 2; //dE^2
      dderivatives[4 * 0 + 1] = 0; //dEdpx
      dderivatives[4 * 0 + 2] = 0; //dEdpy
      dderivatives[4 * 0 + 3] = 0; //dEdpz
      dderivatives[4 * 1 + 1] = -2;//dpx^2
      dderivatives[4 * 1 + 2] = 0; //dpxdpy
      dderivatives[4 * 1 + 3] = 0; //dpxdpz
      dderivatives[4 * 2 + 2] = -2;//dpy^2
      dderivatives[4 * 2 + 3] = 0; //dpydpz
      dderivatives[4 * 3 + 3] = -2;//dpz^2

      return true;

    }

    bool RecoilMassConstraint::firstDerivatives(int i, double* dderivatives) const
    {
      (void) i;

      double totE = 0.;
      double totpx = 0.;
      double totpy = 0.;
      double totpz = 0.;

      for (auto fitobject : fitobjects) {
        auto* pfo = dynamic_cast < ParticleFitObject* >(fitobject);
        assert(pfo);
        totE  += pfo->getE();
        totpx += pfo->getPx();
        totpy += pfo->getPy();
        totpz += pfo->getPz();
      }

      const double recoilE   = (m_beamE - totE);
      const double recoilpx  = (m_beamPx - totpx);
      const double recoilpy  = (m_beamPy - totpy);
      const double recoilpz  = (m_beamPz - totpz);

      dderivatives[0] = -2 * recoilE;
      dderivatives[1] = 2 * recoilpx;
      dderivatives[2] = 2 * recoilpy;
      dderivatives[3] = 2 * recoilpz;

      return true;
    }

    int RecoilMassConstraint::getVarBasis() const
    {
      return VAR_BASIS;
    }
  }// end OrcaKinFit namespace
} // end Belle2 namespace

