/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Further information about the fit engine and the user interface        *
 * provided in MarlinKinfit can be found at                               *
 * https://www.desy.de/~blist/kinfit/doc/html/                            *
 * and in the LCNotes LC-TOOL-2009-001 and LC-TOOL-2009-004 available     *
 * from http://www-flc.desy.de/lcnotes/                                   *
 *                                                                        *
 * Adopted by: Torben Ferber (ferber@physics.ubc.ca) (TF)                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "analysis/OrcaKinFit/ParticleFitObject.h"
#include <framework/logging/Logger.h>

#include <iostream>
#undef NDEBUG
#include <cassert>
#include <cmath>
using std::isfinite;
using std::endl;

// #include <TMatrixDSym.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>


namespace Belle2 {
  namespace OrcaKinFit {

    ParticleFitObject::ParticleFitObject()
      : mass(0), fourMomentum(FourVector(0, 0, 0, 0))
    {
      for (int i = 0; i < BaseDefs::MAXPAR; i++)
        paramCycl[i] = -1;
    }

    ParticleFitObject::~ParticleFitObject()
    {}

    ParticleFitObject::ParticleFitObject(const ParticleFitObject& rhs)
      : BaseFitObject(rhs), mass(0), fourMomentum(FourVector(0, 0, 0, 0))
    {
      //B2INFO( "copying ParticleFitObject with name" << rhs.name );
      ParticleFitObject::assign(rhs);
    }

    ParticleFitObject& ParticleFitObject::operator= (const ParticleFitObject& rhs)
    {
      if (this != &rhs) {
        assign(rhs);  // calls virtual function assign of derived class
      }
      return *this;
    }

    bool ParticleFitObject::setMass(double mass_)
    {
      if (!isfinite(mass_)) return false;
      if (mass == mass_) return true;
      invalidateCache();
      mass = std::abs(mass_);
      return true;
    }


    ParticleFitObject& ParticleFitObject::assign(const BaseFitObject& source)
    {
      if (const ParticleFitObject* psource = dynamic_cast<const ParticleFitObject*>(&source)) {
        if (psource != this) {
          BaseFitObject::assign(source);
          mass = psource->mass;
          for (int i = 0; i < BaseDefs::MAXPAR; ++i)
            paramCycl[i] = psource->paramCycl[i];
        }
      } else {
        assert(0);
      }
      return *this;
    }


    double ParticleFitObject::getMass() const
    {
      return mass;
    }

    std::ostream&  ParticleFitObject::print4Vector(std::ostream& os) const
    {
      os << "[" << getE() << ", " << getPx() << ", "
         << getPy() << ", "  << getPz() << "]";
      return os;
    }

    FourVector ParticleFitObject::getFourMomentum() const
    {
      if (!cachevalid) updateCache();
      return fourMomentum;
    }
    double ParticleFitObject::getE()   const
    {
      return getFourMomentum().getE();
    }
    double ParticleFitObject::getPx()  const
    {
      return getFourMomentum().getPx();
    }
    double ParticleFitObject::getPy()  const
    {
      return getFourMomentum().getPy();
    }
    double ParticleFitObject::getPz()  const
    {
      return getFourMomentum().getPz();
    }
    double ParticleFitObject::getP()   const
    {
      return getFourMomentum().getP();
    }
    double ParticleFitObject::getP2()  const
    {
      return getFourMomentum().getP2();
    }
    double ParticleFitObject::getPt()  const
    {
      return getFourMomentum().getPt();
    }
    double ParticleFitObject::getPt2() const
    {
      return getFourMomentum().getPt2();
    }


    std::ostream&  ParticleFitObject::print(std::ostream& os) const
    {

      if (!cachevalid) updateCache();

      printParams(os);
      os << " => ";
      print4Vector(os);
      os << std::endl;
      return os;
    }

    void ParticleFitObject::addToGlobalChi2DerVectorNum(double* y, int idim, double eps)
    {
      for (int ilocal = 0; ilocal < getNPar(); ++ilocal) {
        int iglobal = getGlobalParNum(ilocal);
        assert(iglobal >= 0 && iglobal < idim);
        y[iglobal] += num1stDerivative(ilocal, eps);
      }
    }


    void ParticleFitObject::addToGlobalChi2DerMatrixNum(double* M, int idim, double eps)
    {
      for (int ilocal1 = 0; ilocal1 < getNPar(); ++ilocal1) {
        int iglobal1 = getGlobalParNum(ilocal1);
        for (int ilocal2 = ilocal1; ilocal2 < getNPar(); ++ilocal2) {
          int iglobal2 = getGlobalParNum(ilocal2);
          M[idim * iglobal1 + iglobal2] += num2ndDerivative(ilocal1, eps, ilocal2, eps);
        }
      }
    }

    void ParticleFitObject::getDerivatives(double der[], int idim) const
    {
      for (int ilocal = 0; ilocal < getNPar(); ++ilocal) {
        assert(ilocal < idim);
        der [4 * ilocal]   = getDE(ilocal);
        der [4 * ilocal + 1] = getDPx(ilocal);
        der [4 * ilocal + 2] = getDPy(ilocal);
        der [4 * ilocal + 3] = getDPz(ilocal);
      }
    }

    void ParticleFitObject::test1stDerivatives()
    {
      B2INFO("ParticleFitObject::test1stDerivatives, object " << getName() << "\n");
      double ycalc[100], ynum[100];
      for (int i = 0; i < 100; ++i) ycalc[i] = ynum[i] = 0;
      addToGlobalChi2DerVector(ycalc, 100);
      double eps = 0.00001;
      addToGlobalChi2DerVectorNum(ynum, 100, eps);
      for (int ilocal = 0; ilocal < getNPar(); ++ilocal) {
        int iglobal = getGlobalParNum(ilocal);
        double calc = ycalc[iglobal];
        double num = ynum[iglobal];
        B2INFO("fo: " << getName() << " par " << ilocal << "/"
               << iglobal << " (" << getParamName(ilocal)
               << ") calc: " << calc << " - num: " << num << " = " << calc - num);
      }
    }

    void ParticleFitObject::test2ndDerivatives()
    {
      B2INFO("ParticleFitObject::test2ndDerivatives, object " << getName() << "\n");
      const int idim = 100;
      double* Mnum = new double[idim * idim];
      double* Mcalc = new double[idim * idim];
      for (int i = 0; i < idim * idim; ++i) Mnum[i] = Mcalc[i] = 0;
      addToGlobalChi2DerMatrix(Mcalc, idim);
      double eps = 0.0001;
      B2INFO("eps=" << eps);
      addToGlobalChi2DerMatrixNum(Mnum, idim, eps);
      for (int ilocal1 = 0; ilocal1 < getNPar(); ++ilocal1) {
        int iglobal1 = getGlobalParNum(ilocal1);
        for (int ilocal2 = ilocal1; ilocal2 < getNPar(); ++ilocal2) {
          int iglobal2 = getGlobalParNum(ilocal2);
          double calc = Mcalc[idim * iglobal1 + iglobal2];
          double num = Mnum[idim * iglobal1 + iglobal2];
          B2INFO("fo: " << getName() << " par " << ilocal1 << "/"
                 << iglobal1 << " (" << getParamName(ilocal1)
                 << "), par " << ilocal2 << "/"
                 << iglobal2 << " (" << getParamName(ilocal2)
                 << ") calc: " << calc << " - num: " << num << " = " << calc - num);
        }
      }
      delete[] Mnum;
      delete[] Mcalc;
    }

    double ParticleFitObject::num1stDerivative(int ilocal, double eps)
    {
      double save = getParam(ilocal);
      setParam(ilocal, save + eps);
      double v1 = getChi2();
      setParam(ilocal, save - eps);
      double v2 = getChi2();
      double result = (v1 - v2) / (2 * eps);
      setParam(ilocal, save);
      return result;
    }

    double ParticleFitObject::num2ndDerivative(int ilocal1, double eeps1,
                                               int ilocal2, double eeps2)
    {
      double result;

      if (ilocal1 == ilocal2) {
        double save = getParam(ilocal1);
        double v0 = getChi2();
        setParam(ilocal1, save + eeps1);
        double v1 = getChi2();
        setParam(ilocal1, save - eeps1);
        double v2 = getChi2();
        result = (v1 + v2 - 2 * v0) / (eeps1 * eeps1);
        setParam(ilocal1, save);
      } else {
        double save1 = getParam(ilocal1);
        double save2 = getParam(ilocal2);
        setParam(ilocal1, save1 + eeps1);
        setParam(ilocal2, save2 + eeps2);
        double v11 = getChi2();
        setParam(ilocal2, save2 - eeps2);
        double v12 = getChi2();
        setParam(ilocal1, save1 - eeps1);
        double v22 = getChi2();
        setParam(ilocal2, save2 + eeps2);
        double v21 = getChi2();
        result = (v11 + v22 - v12 - v21) / (4 * eeps1 * eeps2);
        setParam(ilocal1, save1);
        setParam(ilocal2, save2);
      }
      return result;
    }


    double ParticleFitObject::getChi2() const
    {
      // reimplemented here to take account of cyclical variables e.g azimuthal angle phi - DJeans

      if (!covinvvalid) calculateCovInv();
      if (!covinvvalid) return -1;

      double resid[BaseDefs::MAXPAR] = {0};
      for (int i = 0; i < getNPar(); i++) {
        if (isParamMeasured(i) && !isParamFixed(i)) {
          resid[i] = par[i] - mpar[i];
          if (paramCycl[i] > 0) {
            resid[i] = fmod(resid[i], paramCycl[i]);
            if (resid[i] >  paramCycl[i] / 2) resid[i] -= paramCycl[i];
            if (resid[i] < -paramCycl[i] / 2) resid[i] += paramCycl[i];
          }
        }
      }


      double chi2 = 0;
      for (int i = 0; i < getNPar(); i++) {
        if (isParamMeasured(i) && !isParamFixed(i)) {
          for (int j = 0; j < getNPar(); j++) {
            if (isParamMeasured(j) && !isParamFixed(j)) {
              chi2 += resid[i] * covinv[i][j] * resid[j];
              // B2INFO( getName () << " === " << i << " " << j << " : " <<
              // resid[i] << "*" << covinv[i][j] << "*" << resid[j] << " = " << resid[i]*covinv[i][j]*resid[j] << " , sum " << chi2);
            }
          }
        }
      }

      //  B2INFO("ParticleFitObject::getChi2 () chi2 = " << chi2 );
      return chi2;
    }

  }// end OrcaKinFit namespace
} // end Belle2 namespace


