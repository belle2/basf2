/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * PxPyPzE fit object                                                     *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca) (TF)               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "analysis/OrcaKinFit/PxPyPzMFitObject.h"
#include <framework/logging/Logger.h>
#include <cmath>
#include <cassert>
#include <iostream>

using std::sqrt;
using std::sin;
using std::cos;

namespace Belle2 {
  namespace OrcaKinFit {

// constructor
    PxPyPzMFitObject::PxPyPzMFitObject(HepLorentzVector& particle, HepSymMatrix& covmatrix)
      : cachevalid(false), chi2(0), dEdpx(0), dEdpy(0), dEdpz(0),
        dE2dpxdpx(0), dE2dpxdpy(0), dE2dpxdpz(0), dE2dpydpy(0), dE2dpydpz(0), dE2dpzdpz(0)
    {

      assert(int(NPAR) <= int(BaseDefs::MAXPAR));

      initCov();
      setMass(particle.m());

      setParam(0, particle.px(), true);
      setParam(1, particle.py(), true);
      setParam(2, particle.pz(), true);

      setMParam(0, particle.px());
      setMParam(1, particle.py());
      setMParam(2, particle.pz());

      //set covariance matrix
      for (int i = 0; i < int(NPAR); i++) {
        for (int j = 0; j < int(NPAR); j++) {
          setCov(i, j, covmatrix[i][j]);
        }
      }

      invalidateCache();
    }

// destructor
    PxPyPzMFitObject::~PxPyPzMFitObject() {}

    PxPyPzMFitObject::PxPyPzMFitObject(const PxPyPzMFitObject& rhs)
      : ParticleFitObject(rhs), cachevalid(false), chi2(0), dEdpx(0), dEdpy(0), dEdpz(0),
        dE2dpxdpx(0), dE2dpxdpy(0), dE2dpxdpz(0), dE2dpydpy(0), dE2dpydpz(0), dE2dpzdpz(0)
    {

      PxPyPzMFitObject::assign(rhs);
    }

    PxPyPzMFitObject& PxPyPzMFitObject::operator= (const PxPyPzMFitObject& rhs)
    {
      if (this != &rhs) {
        assign(rhs);  // calls virtual function assign of derived class
      }
      return *this;
    }

    PxPyPzMFitObject* PxPyPzMFitObject::copy() const
    {
      return new PxPyPzMFitObject(*this);
    }

    PxPyPzMFitObject& PxPyPzMFitObject::assign(const BaseFitObject& source)
    {
      if (const PxPyPzMFitObject* psource = dynamic_cast<const PxPyPzMFitObject*>(&source)) {
        if (psource != this) {
          ParticleFitObject::assign(source);
          // only mutable data members, need not to be copied, if cache is invalid
        }
      } else {
        assert(0);
      }
      return *this;
    }

    const char* PxPyPzMFitObject::getParamName(int ilocal) const
    {
      switch (ilocal) {
        case 0: return "px";
        case 1: return "py";
        case 2: return "pz";
      }
      return "undefined";
    }


    bool PxPyPzMFitObject::updateParams(double p[], int idim)
    {
      invalidateCache();

      int ipx = getGlobalParNum(0);
      int ipy = getGlobalParNum(1);
      int ipz = getGlobalParNum(2);
      assert(ipx >= 0 && ipx  < idim);
      assert(ipy >= 0 && ipy  < idim);
      assert(ipz >= 0 && ipz  < idim);

      double px = p[ipx];
      double py = p[ipy];
      double pz = p[ipz];

      bool result = ((px - par[0]) * (px - par[0]) > eps2 * cov[0][0]) ||
                    ((py - par[1]) * (py - par[1]) > eps2 * cov[1][1]) ||
                    ((pz - par[2]) * (pz - par[2]) > eps2 * cov[2][2]);

      par[0] = px;
      par[1] = py;
      par[2] = pz;
      p[ipx] = par[0];
      p[ipy] = par[1];
      p[ipz] = par[2];
      return result;
    }

// these depend on actual parametrisation!

    double PxPyPzMFitObject::getDPx(int ilocal) const
    {
      assert(ilocal >= 0 && ilocal < NPAR);
      if (!cachevalid) updateCache();
      switch (ilocal) {
        case 0: return 1.0;
        case 1: return 0.0;
        case 2: return 0.0;
      }
      return 0;
    }

    double PxPyPzMFitObject::getDPy(int ilocal) const
    {
      assert(ilocal >= 0 && ilocal < NPAR);
      if (!cachevalid) updateCache();
      switch (ilocal) {
        case 0: return 0.0;
        // case 1: return dpydtheta;
        case 1: return 1.0;
        case 2: return 0.0;
      }
      return 0;
    }

    double PxPyPzMFitObject::getDPz(int ilocal) const
    {
      assert(ilocal >= 0 && ilocal < NPAR);
      if (!cachevalid) updateCache();
      switch (ilocal) {
        case 0: return 0.0;
        case 1: return 0.0;
        case 2: return 1.0;
      }
      return 0;
    }

    double PxPyPzMFitObject::getDE(int ilocal) const
    {
      assert(ilocal >= 0 && ilocal < NPAR);
      if (!cachevalid) updateCache();
      switch (ilocal) {
        case 0: return dEdpx;
        case 1: return dEdpy;
        case 2: return dEdpz;
      }
      return 0;
    }

    double PxPyPzMFitObject::getFirstDerivative_Meta_Local(int iMeta, int ilocal , int metaSet) const
    {
      // iMeta = intermediate variable (i.e. E,px,py,pz)
      // ilocal = local variable (ptinv, theta, phi)
      // metaSet = which set of intermediate varlables

      assert(metaSet == 0); // only defined for E,px,py,pz

      switch (iMeta) {
        case 0: // E
          return getDE(ilocal);
          break;
        case 1: // Px
          return getDPx(ilocal);
          break;
        case 2: // Py
          return getDPy(ilocal);
          break;
        case 3: // Pz
          return getDPz(ilocal);
          break;
        default:
          assert(0);
      }
    }

    double PxPyPzMFitObject::getSecondDerivative_Meta_Local(int iMeta, int ilocal , int jlocal , int metaSet) const
    {
      assert(metaSet == 0);
      if (!cachevalid) updateCache();
      if (jlocal < ilocal) {
        int temp = jlocal;
        jlocal = ilocal;
        ilocal = temp;
      }

      switch (iMeta) {
        case 0: // E
          if (ilocal == 0 && jlocal == 0) return dE2dpxdpx;
          else if (ilocal == 0 && jlocal == 1) return dE2dpxdpy;
          else if (ilocal == 0 && jlocal == 2) return dE2dpxdpz;
          else if (ilocal == 1 && jlocal == 1) return dE2dpydpy;
          else if (ilocal == 1 && jlocal == 2) return dE2dpydpz;
          else if (ilocal == 2 && jlocal == 2) return dE2dpzdpz;
          else return 0;
          break;
        case 1: // px
          return 0;
          break;
        case 2: // py
          return 0;
          break;
        case 3: // pz
          return 0;
          break;
        default:
          assert(0);
      }
    }

    void PxPyPzMFitObject::updateCache() const
    {

      // get the basic quantities
      double px = par[0];
      double py = par[1];
      double pz = par[2];
      double px2 = px * px;
      double py2 = py * py;
      double pz2 = pz * pz;
      double p  = px + py + pz;
      assert(p != 0);

      double mass2 = mass * mass;
      double e2 = px2 + py2 + pz2 + mass2;
      double e  = std::sqrt(e2);

      // get the first derivatives
      dEdpx = px / e;
      dEdpy = py / e;
      dEdpz = pz / e;

      // get second derivatives
      double e32 = std::pow(e, 1.5);
      dE2dpxdpx = (py2 + pz2 + mass2) / (e32);
      dE2dpxdpy = -(px * py) / (e32);
      dE2dpxdpz = -(px * pz) / (e32);
      dE2dpydpy = (px2 + pz2 + mass2) / (e32);
      dE2dpydpz = -(py * pz) / (e32);
      dE2dpzdpz = (px2 + py2 + mass2) / (e32);

      fourMomentum.setValues(e, px, py, pz);

      cachevalid = true;
    }
  }// end OrcaKinFit namespace
} // end Belle2 namespace


