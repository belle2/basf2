/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * See https://github.com/tferber/OrcaKinfit, forked from                 *
 * https://github.com/iLCSoft/MarlinKinfit                                *
 *                                                                        *
 * Further information about the fit engine and the user interface        *
 * provided in MarlinKinfit can be found at                               *
 * https://www.desy.de/~blist/kinfit/doc/html/                            *
 * and in the LCNotes LC-TOOL-2009-001 and LC-TOOL-2009-004 available     *
 * from http://www-flc.desy.de/lcnotes/                                   *
 *                                                                        *
 * Adopted by: Torben Ferber (torben.ferber@desy.de) (TF)                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifdef MARLIN_USE_ROOT


#include "analysis/OrcaKinFit/SoftBWParticleConstraint.h"
#include "analysis/OrcaKinFit/ParticleFitObject.h"
#include <framework/logging/Logger.h>

#include "Math/ProbFuncMathCore.h"
#include "Math/QuantFuncMathCore.h"

#include <iostream>
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace OrcaKinFit {

    SoftBWParticleConstraint::SoftBWParticleConstraint(double gamma_, double emin_, double emax_)
      :
      fitobjects(FitObjectContainer()), derivatives(std::vector <double> ()), flags(std::vector <int> ()),
      gamma(gamma_), emin(emin_), emax(emax_),
      cachevalid(false),
      atanxmin(0), atanxmax(0), diffatanx(0)
    {
      invalidateCache();
    }

    double SoftBWParticleConstraint::getGamma() const
    {
      return gamma;
    }

    double SoftBWParticleConstraint::setGamma(double gamma_)
    {
      if (gamma_ != 0) gamma = std::abs(gamma_);
      invalidateCache();
      return gamma;
    }

    double SoftBWParticleConstraint::getChi2() const
    {
      return penalty(getValue());
    }

    double SoftBWParticleConstraint::getError() const
    {
      double dgdpi[4];
      double error2 = 0;
      for (unsigned int i = 0; i < fitobjects.size(); ++i) {
        const ParticleFitObject* foi = fitobjects[i];
        assert(foi);
        if (firstDerivatives(i, dgdpi)) {
          error2 += foi->getError2(dgdpi, getVarBasis());
        }
      }
      return std::sqrt(std::abs(error2));
    }

    /**
     * Calculates the second derivative of the constraint g w.r.t. the various parameters
     * and adds it to the global covariance matrix
     *
     * We denote with P_i the 4-vector of the i-th ParticleFitObject,
     * then
     * $$ \frac{\partial ^2 g}{\partial a_k \partial a_l}
     *   = \sum_i \sum_j \frac{\partial ^2 g}{\partial P_i \partial P_j} \cdot
     *     \frac{\partial P_i}{\partial a_k} \cdot \frac{\partial P_j}{\partial a_l}
     *     + \sum_i \frac{\partial g}{\partial P_i} \cdot
     *        \frac{\partial^2 P_i}{\partial a_k \partial a_l}
     * $$
     * Here, $\frac{\partial P_i}{\partial a_k}$ is a $4 \times n_i$ Matrix, where
     * $n_i$ is the number of parameters of FitObject i;
     * Correspondingly, $\frac{\partial^2 P_i}{\partial a_k \partial a_l}$ is a
     * $4 \times n_i \times n_i$ matrix.
     * Also, $\frac{\partial ^2 g}{\partial P_i \partial P_j}$ is a $4\times 4$ matrix
     * for a given i and j, and $\frac{\partial g}{\partial P_i}$ is a 4-vector
     * (though not a Lorentz-vector!).
     *
     */


    void SoftBWParticleConstraint::add2ndDerivativesToMatrix(double* M, int idim) const
    {

      /** First, treat the part
       * $$
       *    \frac{\partial h}{\partial g}
       *    \frac{\partial ^2 g}{\partial P_i \partial P_j}  \cdot
       *     \frac{\partial P_i}{\partial a_k} \cdot \frac{\partial P_j}{\partial a_l}
       * $$
       */
      double e = getValue();
      double fact = penalty1stder(e);
      double fact2 = penalty2ndder(e);

      // Derivatives $\frac{\partial ^2 g}{\partial P_i \partial P_j}$ at fixed i, j
      // d2GdPidPj[4*ii+jj] is derivative w.r.t. P_i,ii and P_j,jj, where ii=0,1,2,3 for E,px,py,pz
      double d2GdPidPj[16];
      // Derivatives $\frac {\partial P_i}{\partial a_k}$ for all i;
      // k is local parameter number
      // dPidAk[KMAX*4*i + 4*k + ii] is $\frac {\partial P_{i,ii}}{\partial a_k}$,
      // with ii=0, 1, 2, 3 for E, px, py, pz
      const int KMAX = 4;
      const int n = fitobjects.size();
      double* dPidAk = new double[n * KMAX * 4];
      bool* dPidAkval = new bool[n];

      for (int i = 0; i < n; ++i) dPidAkval[i] = false;

      // Derivatives $\frac{\partial ^2 g}{\partial P_i \partial a_l}$ at fixed i
      // d2GdPdAl[4*l + ii] is $\frac{\partial ^2 g}{\partial P_{i,ii} \partial a_l}$
      double d2GdPdAl[4 * KMAX];
      // Derivatives $\frac{\partial ^2 g}{\partial a_k \partial a_l}$
      double d2GdAkdAl[KMAX * KMAX] = {0};



      // Global parameter numbers: parglobal[KMAX*i+klocal]
      // is global parameter number of local parameter klocal of i-th Fit object
      int* parglobal = new int[KMAX * n];

      for (int i = 0; i < n; ++i) {
        const ParticleFitObject* foi = fitobjects[i];
        assert(foi);
        for (int klocal = 0; klocal < foi->getNPar(); ++klocal) {
          parglobal [KMAX * i + klocal] = foi->getGlobalParNum(klocal);
        }
      }


      for (int i = 0; i < n; ++i) {
        const ParticleFitObject* foi = fitobjects[i];
        assert(foi);
        for (int j = 0; j < n; ++j) {
          const ParticleFitObject* foj = fitobjects[j];
          assert(foj);
          if (secondDerivatives(i, j, d2GdPidPj)) {
            if (!dPidAkval[i]) {
              foi->getDerivatives(dPidAk + i * (KMAX * 4), KMAX * 4);
              dPidAkval[i] = true;
            }
            if (!dPidAkval[j]) {
              foj->getDerivatives(dPidAk + j * (KMAX * 4), KMAX * 4);
              dPidAkval[j] = true;
            }
            // Now sum over E/px/Py/Pz for object j:
            // $$\frac{\partial ^2 g}{\partial P_{i,ii} \partial a_l}
            //   = (sum_{j}) sum_{jj} frac{\partial ^2 g}{\partial P_{i,ii} \partial P_{j,jj}}
            //     \cdot \frac{\partial P_{j,jj}}{\partial a_l}
            // We're summing over jj here
            for (int llocal = 0; llocal < foj->getNPar(); ++llocal) {
              for (int ii = 0; ii < 4; ++ii) {
                int ind1 = 4 * ii;
                int ind2 = (KMAX * 4) * j + 4 * llocal;
                double& r = d2GdPdAl[4 * llocal + ii];
                r  = d2GdPidPj[  ind1] * dPidAk[  ind2];   // E
                r += d2GdPidPj[++ind1] * dPidAk[++ind2];   // px
                r += d2GdPidPj[++ind1] * dPidAk[++ind2];   // py
                r += d2GdPidPj[++ind1] * dPidAk[++ind2];   // pz
              }
            }
            // Now sum over E/px/Py/Pz for object i, i.e. sum over ii:
            // $$
            // \frac{\partial ^2 g}{\partial a_k \partial a_l}
            //      = \sum_{ii} \frac{\partial ^2 g}{\partial P_{i,ii} \partial a_l} \cdot
            //        \frac{\partial P_{i,ii}}{\partial a_k}
            // $$
            for (int klocal = 0; klocal < foi->getNPar(); ++klocal) {
              for (int llocal = 0; llocal < foj->getNPar(); ++llocal) {
                int ind1 = 4 * llocal;
                int ind2 = (KMAX * 4) * i + 4 * klocal;
                double& r = d2GdAkdAl[KMAX * klocal + llocal];
                r  = d2GdPdAl[  ind1] * dPidAk[  ind2];    //E
                r += d2GdPdAl[++ind1] * dPidAk[++ind2];   // px
                r += d2GdPdAl[++ind1] * dPidAk[++ind2];   // py
                r += d2GdPdAl[++ind1] * dPidAk[++ind2];   // pz
              }
            }
            // Now expand the local parameter numbers to global ones
            for (int klocal = 0; klocal < foi->getNPar(); ++klocal) {
              int kglobal = parglobal [KMAX * i + klocal];
              for (int llocal = 0; llocal < foj->getNPar(); ++llocal) {
                int lglobal = parglobal [KMAX * j + llocal];
                M [idim * kglobal + lglobal] += fact * d2GdAkdAl[KMAX * klocal + llocal];
              }
            }
          }
        }
      }
      /** Second, treat the parts
       * $$
       * \frac{\partial h}{\partial g}
       * \sum_i \frac{\partial g}{\partial P_i} \cdot
       *        \frac{\partial^2 P_i}{\partial a_k \partial a_l}
       * $$
       * and
       * $$
       * \frac{\partial^2 h}{\partial g^2}
       * \sum_i \frac{\partial g}{\partial P_i} \cdot
       *        \frac{\partial P_i}{\partial a_k}
       * \sum_j \frac{\partial g}{\partial P_j} \cdot
       *        \frac{\partial P_j}{\partial a_l}
       * $$
       *
       * Here, $\frac{\partial g}{\partial P_i}$ is a 4-vector, which we pass on to
       * the FitObject
       */

      double* v = new double[idim];
      for (int i = 0; i < idim; ++i) v[i] = 0;

      // fact2 may be negative, so don't use sqrt(fact2)
      double dgdpi[4];
      for (int i = 0; i < n; ++i) {
        const ParticleFitObject* foi = fitobjects[i];
        assert(foi);
        if (firstDerivatives(i, dgdpi)) {
          foi->addTo2ndDerivatives(M, idim, fact, dgdpi, getVarBasis());
          foi->addToGlobalChi2DerVector(v, idim, 1, dgdpi, getVarBasis());
        }
      }

      for (int i = 0; i < idim; ++i) {
        if (double vi = v[i]) {
          int ioffs = i * idim;
          for (double* pvj = v; pvj < v + idim; ++pvj) {
            M[ioffs++] += fact2 * vi * (*pvj);
          }
        }
      }


      delete[] dPidAk;
      delete[] dPidAkval;
      delete[] parglobal;
      delete[] v;
    }

    void SoftBWParticleConstraint::addToGlobalChi2DerVector(double* y, int idim) const
    {
      double dgdpi[4];
      double r = penalty1stder(getValue());
      for (unsigned int i = 0; i < fitobjects.size(); ++i) {
        const ParticleFitObject* foi = fitobjects[i];
        assert(foi);
        if (firstDerivatives(i, dgdpi)) {
          foi->addToGlobalChi2DerVector(y, idim, r, dgdpi, getVarBasis());
        }
      }
    }

    void SoftBWParticleConstraint::test1stDerivatives()
    {
      B2INFO("SoftBWParticleConstraint::test1stDerivatives for " << getName());
      double y[100];
      for (int i = 0; i < 100; ++i) y[i] = 0;
      addToGlobalChi2DerVector(y, 100);
      double eps = 0.00001;
      for (unsigned int ifo = 0; ifo < fitobjects.size(); ++ifo) {
        ParticleFitObject* fo = fitobjects[ifo];
        assert(fo);
        for (int ilocal = 0; ilocal < fo->getNPar(); ++ilocal) {
          int iglobal = fo->getGlobalParNum(ilocal);
          double calc = y[iglobal];
          double num = num1stDerivative(ifo, ilocal, eps);
          B2INFO("fo: " << fo->getName() << " par " << ilocal << "/"
                 << iglobal << " (" << fo->getParamName(ilocal)
                 << ") calc: " << calc << " - num: " << num << " = " << calc - num);
        }
      }
    }
    void SoftBWParticleConstraint::test2ndDerivatives()
    {
      B2INFO("SoftBWParticleConstraint::test2ndDerivatives for " << getName());
      const int idim = 100;
      double* M = new double[idim * idim];
      for (int i = 0; i < idim * idim; ++i) M[i] = 0;
      add2ndDerivativesToMatrix(M, idim);
      double eps = 0.0001;
      B2INFO("eps=" << eps);

      for (unsigned int ifo1 = 0; ifo1 < fitobjects.size(); ++ifo1) {
        ParticleFitObject* fo1 = fitobjects[ifo1];
        assert(fo1);
        for (unsigned int ifo2 = ifo1; ifo2 < fitobjects.size(); ++ifo2) {
          ParticleFitObject* fo2 = fitobjects[ifo2];
          assert(fo2);
          for (int ilocal1 = 0; ilocal1 < fo1->getNPar(); ++ilocal1) {
            int iglobal1 = fo1->getGlobalParNum(ilocal1);
            for (int ilocal2 = (ifo1 == ifo2 ? ilocal1 : 0); ilocal2 < fo2->getNPar(); ++ilocal2) {
              int iglobal2 = fo2->getGlobalParNum(ilocal2);
              double calc = M[idim * iglobal1 + iglobal2];
              double num = num2ndDerivative(ifo1, ilocal1, eps, ifo2, ilocal2, eps);
              B2INFO("fo1: " << fo1->getName() << " par " << ilocal1 << "/"
                     << iglobal1 << " (" << fo1->getParamName(ilocal1)
                     << "), fo2: " << fo2->getName() << " par " << ilocal2 << "/"
                     << iglobal2 << " (" << fo2->getParamName(ilocal2)
                     << ") calc: " << calc << " - num: " << num << " = " << calc - num);
            }
          }
        }
      }
      delete[] M;
    }


    double SoftBWParticleConstraint::num1stDerivative(int ifo, int ilocal, double eps)
    {
      ParticleFitObject* fo = fitobjects[ifo];
      assert(fo);
      double save = fo->getParam(ilocal);
      fo->setParam(ilocal, save + eps);
      double v1 = getChi2();
      fo->setParam(ilocal, save - eps);
      double v2 = getChi2();
      double result = (v1 - v2) / (2 * eps);
      fo->setParam(ilocal, save);
      return result;
    }

    double SoftBWParticleConstraint::num2ndDerivative(int ifo1, int ilocal1, double eps1,
                                                      int ifo2, int ilocal2, double eps2)
    {
      double result;

      if (ifo1 == ifo2 && ilocal1 == ilocal2) {
        ParticleFitObject* fo = fitobjects[ifo1];
        assert(fo);
        double save = fo->getParam(ilocal1);
        double v0 = getChi2();
        fo->setParam(ilocal1, save + eps1);
        double v1 = getChi2();
        fo->setParam(ilocal1, save - eps1);
        double v2 = getChi2();
        result = (v1 + v2 - 2 * v0) / (eps1 * eps1);
        fo->setParam(ilocal1, save);
      } else {
        ParticleFitObject* fo1 = fitobjects[ifo1];
        assert(fo1);
        ParticleFitObject* fo2 = fitobjects[ifo2];
        assert(fo2);
        double save1 = fo1->getParam(ilocal1);
        double save2 = fo2->getParam(ilocal2);
        fo1->setParam(ilocal1, save1 + eps1);
        fo2->setParam(ilocal2, save2 + eps2);
        double v11 = getChi2();
        fo2->setParam(ilocal2, save2 - eps2);
        double v12 = getChi2();
        fo1->setParam(ilocal1, save1 - eps1);
        double v22 = getChi2();
        fo2->setParam(ilocal2, save2 + eps2);
        double v21 = getChi2();
        result = (v11 + v22 - v12 - v21) / (4 * eps1 * eps2);
        fo1->setParam(ilocal1, save1);
        fo2->setParam(ilocal2, save2);
      }
      return result;
    }

    double SoftBWParticleConstraint::erfinv(double x)
    {
//   static const double a = 8*(M_PI-3)/(3*M_PI*(4-M_PI));
//   static const double aa = 3*(4-M_PI)/(4*(M_PI-3));  // = 2/(M_PI*a);
//   double s = (x<0) ? -1 : 1;
//   x *= s;
//   if (x >= 1) return s*HUGE_VAL;
//   double ll = std::log (1 - x*x);
//   double xx = aa + 0.5*ll;
//   return s * std::sqrt(-xx + std::sqrt (xx*xx - ll/a));
      return 2 * ROOT::Math::normal_quantile(std::sqrt(2.0) * x, 1.0) - 1;
    }

    double SoftBWParticleConstraint::normal_quantile(double x)
    {
      return ROOT::Math::normal_quantile(x, 1.0);
    }

    double SoftBWParticleConstraint::normal_quantile_1stderiv(double x)
    {
      double y = ROOT::Math::normal_quantile(x, 1.0);
      return 1 / normal_pdf(y);
    }

    double SoftBWParticleConstraint::normal_quantile_2ndderiv(double x)
    {
      double y = ROOT::Math::normal_quantile(x, 1.0);
      return -y / pow(normal_pdf(y), 2);
    }

    double SoftBWParticleConstraint::normal_pdf(double x)
    {
      static const double C_1_SQRT2PI = 1 / (std::sqrt(2 * M_PI));
      return C_1_SQRT2PI * std::exp(-0.5 * x * x);
    }

    double SoftBWParticleConstraint::normal_pdf_deriv(double x)
    {
      static const double C_1_SQRT2PI = 1 / (std::sqrt(2 * M_PI));
      return -C_1_SQRT2PI * x * std::exp(-0.5 * x * x);
    }

    double SoftBWParticleConstraint::penalty(double e) const
    {
      double x = e / gamma;
      // x is distributed according to the Cauchy distribution
      // f(x) = 1/pi 1/(1 + x^2)
      // or (if limits are active)
      // f(x) = 1/(arctan(x_max) - arctan(x_min)) 1/(1 + x^2)

      // The integral pdf is
      // F(x) = 1/2 + 1/pi arctan (x)
      // or (if limits are active)
      // F(x) = 1/2 + arctan (x)/(arctan(x_max) - arctan(x_min))

      // So, chi2 = 2 (erf^-1 (1 + 2 F(x)) )^2
      // or chi2 = norm_quantile (F(x))^2

      if (!cachevalid) updateCache();
      double F = 0.5 + std::atan(x) / diffatanx;
      if (F < 0 || F > 1 || !std::isfinite(F))
        B2INFO("SoftBWParticleConstraint::penalty: error for e=" << e
               << ", gamma=" << gamma << " -> x=" << x << " => F=" << F);

      assert(F >= 0);
      assert(F <= 1);
      double result = std::pow(normal_quantile(F), 2);
      assert(std::isfinite(result));
      return result;



      //double chi = erfinv (2/M_PI *std:arctan (x));
      //return 2*chi*chi;;

      // anyway, a very good and much simpler approximation is
      // return 0.75*std::log (1 + x*x);
    }

    double SoftBWParticleConstraint::penalty1stder(double e) const
    {
      double x = e / gamma;
      if (!cachevalid) updateCache();
      double F = 0.5 + std::atan(x) / diffatanx;
      double dF_de = 1. / ((1 + x * x) * diffatanx * gamma);
      double result = 2 * normal_quantile(F) * normal_quantile_1stderiv(F) * dF_de;
      assert(std::isfinite(result));
      return result;

    }
    double SoftBWParticleConstraint::penalty2ndder(double e) const
    {
      double x = e / gamma;
      if (!cachevalid) updateCache();
      double F = 0.5 + std::atan(x) / diffatanx;
      double dF_de = 1. / ((1 + x * x) * diffatanx * gamma);
      double d2F_de2 = -2 * diffatanx * x * dF_de * dF_de;

      double result = 2 * pow(normal_quantile_1stderiv(F) * dF_de, 2)
                      + 2 * normal_quantile(F) * normal_quantile_2ndderiv(F) * dF_de * dF_de
                      + 2 * normal_quantile(F) * normal_quantile_1stderiv(F) * d2F_de2;
      assert(std::isfinite(result));
      return result;


    }

    void SoftBWParticleConstraint::invalidateCache() const
    {
      cachevalid = false;
    }

    void SoftBWParticleConstraint::updateCache() const
    {
      if (emin == -numeric_limits<double>::infinity())
        atanxmin = -M_PI_2;
      else if (emin == numeric_limits<double>::infinity())
        atanxmin =  M_PI_2;
      else  atanxmin = std::atan(emin / gamma);
      if (emax == -numeric_limits<double>::infinity())
        atanxmax = -M_PI_2;
      else if (emax == numeric_limits<double>::infinity())
        atanxmax =  M_PI_2;
      else  atanxmax = std::atan(emax / gamma);
      diffatanx = atanxmax - atanxmin;
      cachevalid = true;

      B2INFO("SoftBWParticleConstraint::updateCache(): "
             << "gamma=" << gamma
             << ", emin=" << emin << " -> atanxmin=" << atanxmin
             << ", emax=" << emax << " -> atanxmax=" << atanxmax
             << " => diffatanx=" << diffatanx);

    }

    bool SoftBWParticleConstraint::cacheValid() const
    {
      return cachevalid;
    }

    int SoftBWParticleConstraint::getVarBasis() const
    {
      return VAR_BASIS;
    }

  }// end OrcaKinFit namespace
} // end Belle2 namespace


#endif // MARLIN_USE_ROOT
