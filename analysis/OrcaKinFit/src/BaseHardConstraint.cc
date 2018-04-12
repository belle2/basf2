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

#include "analysis/OrcaKinFit/BaseHardConstraint.h"
#include <framework/logging/Logger.h>

#undef NDEBUG
#include <cassert>

#include <cstring>
#include <iostream>
#include <cmath>

BaseHardConstraint::~BaseHardConstraint()
{}


void BaseHardConstraint::add1stDerivativesToMatrix(double* M, int idim) const
{
  double dgdpi[BaseDefs::MAXINTERVARS];
  for (unsigned int i = 0; i < fitobjects.size(); ++i) {
    const BaseFitObject* foi = fitobjects[i];
    assert(foi);
    if (firstDerivatives(i, dgdpi)) {
      foi->addTo1stDerivatives(M, idim, dgdpi, getGlobalNum(), getVarBasis());
    }
  }
}


/**
 * Calculates the second derivative of the constraint g w.r.t. the various parameters,
 * multiplies it by lambda and adds it to the global covariance matrix
 *
 *    in case of particlefitobject:
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
 *    but here it's been generalised
 */


void BaseHardConstraint::add2ndDerivativesToMatrix(double* M, int idim, double lambda) const
{

  /** First, treat the part
   * $$
   *    \frac{\partial ^2 g}{\partial P_i \partial P_j}  \cdot
   *     \frac{\partial P_i}{\partial a_k} \cdot \frac{\partial P_j}{\partial a_l}
   * $$
   */
  // Derivatives $\frac{\partial ^2 g}{\partial P_i \partial P_j}$ at fixed i, j
  // d2GdPidPj[4*ii+jj] is derivative w.r.t. P_i,ii and P_j,jj, where ii=0,1,2,3 for E,px,py,pz
  double d2GdPidPj[BaseDefs::MAXINTERVARS * BaseDefs::MAXINTERVARS];

  // Derivatives $\frac {\partial P_i}{\partial a_k}$ for all i;
  // k is local parameter number
  // dPidAk[KMAX*4*i + 4*k + ii] is $\frac {\partial P_{i,ii}}{\partial a_k}$,
  // with ii=0, 1, 2, 3 for E, px, py, pz
  const int n = fitobjects.size();
  double* dPidAk = new double[n * BaseDefs::MAXPAR * BaseDefs::MAXINTERVARS];
  bool* dPidAkval = new bool[n];

  for (int i = 0; i < n; ++i) dPidAkval[i] = false;

  // Derivatives $\frac{\partial ^2 g}{\partial P_i \partial a_l}$ at fixed i
  // d2GdPdAl[4*l + ii] is $\frac{\partial ^2 g}{\partial P_{i,ii} \partial a_l}$
  double d2GdPdAl[BaseDefs::MAXINTERVARS * BaseDefs::MAXPAR];
  // Derivatives $\frac{\partial ^2 g}{\partial a_k \partial a_l}$
  double d2GdAkdAl[BaseDefs::MAXPAR * BaseDefs::MAXPAR];

  // Global parameter numbers: parglobal[BaseDefs::MAXPAR*i+klocal]
  // is global parameter number of local parameter klocal of i-th Fit object
  int* parglobal = new int[BaseDefs::MAXPAR * n];

  for (int i = 0; i < n; ++i) {
    const BaseFitObject* foi =  fitobjects[i];
    assert(foi);
    for (int klocal = 0; klocal < foi->getNPar(); ++klocal) {
      parglobal [BaseDefs::MAXPAR * i + klocal] = foi->getGlobalParNum(klocal);
    }
  }


  for (int i = 0; i < n; ++i) {
    const BaseFitObject* foi =  fitobjects[i];
    assert(foi);
    for (int j = 0; j < n; ++j) {
      const BaseFitObject* foj =  fitobjects[j];
      assert(foj);
      if (secondDerivatives(i, j, d2GdPidPj)) {
        if (!dPidAkval[i]) {
          foi->getDerivatives(dPidAk + i * (BaseDefs::MAXPAR * BaseDefs::MAXINTERVARS), BaseDefs::MAXPAR * BaseDefs::MAXINTERVARS);
          dPidAkval[i] = true;
        }
        if (!dPidAkval[j]) {
          foj->getDerivatives(dPidAk + j * (BaseDefs::MAXPAR * BaseDefs::MAXINTERVARS), BaseDefs::MAXPAR * BaseDefs::MAXINTERVARS);
          dPidAkval[j] = true;
        }
        // Now sum over E/px/Py/Pz for object j:
        // $$\frac{\partial ^2 g}{\partial P_{i,ii} \partial a_l}
        //   = (sum_{j}) sum_{jj} frac{\partial ^2 g}{\partial P_{i,ii} \partial P_{j,jj}}
        //     \cdot \frac{\partial P_{j,jj}}{\partial a_l}
        // We're summing over jj here
        for (int llocal = 0; llocal < foj->getNPar(); ++llocal) {
          for (int ii = 0; ii < BaseDefs::MAXINTERVARS; ++ii) {
            int ind1 = BaseDefs::MAXINTERVARS * ii;
            int ind2 = (BaseDefs::MAXPAR * BaseDefs::MAXINTERVARS) * j + BaseDefs::MAXINTERVARS * llocal;
            double& r = d2GdPdAl[BaseDefs::MAXINTERVARS * llocal + ii];
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
            int ind1 = BaseDefs::MAXINTERVARS * llocal;
            int ind2 = (BaseDefs::MAXPAR * BaseDefs::MAXINTERVARS) * i + BaseDefs::MAXINTERVARS * klocal;
            double& r = d2GdAkdAl[BaseDefs::MAXPAR * klocal + llocal];
            r  = d2GdPdAl[  ind1] * dPidAk[  ind2];    //E
            r += d2GdPdAl[++ind1] * dPidAk[++ind2];   // px
            r += d2GdPdAl[++ind1] * dPidAk[++ind2];   // py
            r += d2GdPdAl[++ind1] * dPidAk[++ind2];   // pz
          }
        }
        // Now expand the local parameter numbers to global ones
        for (int klocal = 0; klocal < foi->getNPar(); ++klocal) {
          int kglobal = parglobal [BaseDefs::MAXPAR * i + klocal];
          for (int llocal = 0; llocal < foj->getNPar(); ++llocal) {
            int lglobal = parglobal [BaseDefs::MAXPAR * j + llocal];
            M [idim * kglobal + lglobal] += lambda * d2GdAkdAl[BaseDefs::MAXPAR * klocal + llocal];
          }
        }
      }
    }
  }
  /** Second, treat the part
   * $$
   * \sum_i \frac{\partial g}{\partial P_i} \cdot
   *        \frac{\partial^2 P_i}{\partial a_k \partial a_l}
   * $$
   * Here, $\frac{\partial g}{\partial P_i}$ is a 4-vector, which we pass on to
   * the FitObject
   */

  double dgdpi[BaseDefs::MAXINTERVARS];
  for (int i = 0; i < n; ++i) {
    const BaseFitObject* foi = fitobjects[i];
    assert(foi);
    if (firstDerivatives(i, dgdpi)) {
      foi->addTo2ndDerivatives(M, idim, lambda, dgdpi, getVarBasis());
    }
  }

  delete[] dPidAk;
  delete[] dPidAkval;
  delete[] parglobal;
}

void BaseHardConstraint::addToGlobalChi2DerVector(double* y, int idim, double lambda) const
{
  double dgdpi[BaseDefs::MAXINTERVARS];
  for (unsigned int i = 0; i < fitobjects.size(); ++i) {
    const BaseFitObject* foi = fitobjects[i];
    assert(foi);
    if (firstDerivatives(i, dgdpi)) {
      foi->addToGlobalChi2DerVector(y, idim, lambda, dgdpi, getVarBasis());
    }
  }
}


double BaseHardConstraint::getError() const
{
  double dgdpi[BaseDefs::MAXINTERVARS];
  double error2 = 0;
  for (unsigned int i = 0; i < fitobjects.size(); ++i) {
    const BaseFitObject* foi = fitobjects[i];
    assert(foi);
    if (firstDerivatives(i, dgdpi)) {
      error2 += foi->getError2(dgdpi, getVarBasis());
    }
  }
  return std::sqrt(std::abs(error2));
}


double BaseHardConstraint::dirDer(double* p, double* w, int idim, double mu)
{
  double* pw, *pp;
  for (pw = w; pw < w + idim; * (pw++) = 0);
  addToGlobalChi2DerVector(w, idim, mu);
  double result = 0;
  for (pw = w, pp = p; pw < w + idim; result += *(pp++) * *(pw++));
  return mu * result;
}

double BaseHardConstraint::dirDerAbs(double* p, double* w, int idim, double mu)
{
  double val = getValue();
  if (val == 0) return mu * std::fabs(dirDer(p, w, idim, 1));
  else if (val > 0) return mu * dirDer(p, w, idim, 1);
  else return -mu * dirDer(p, w, idim, 1);
}


void BaseHardConstraint::test1stDerivatives()
{
  B2INFO("BaseConstraint::test1stDerivatives for " << getName());
  double y[100];
  for (int i = 0; i < 100; ++i) y[i] = 0;
  addToGlobalChi2DerVector(y, 100, 1);
  double eps = 0.00001;
  for (unsigned int ifo = 0; ifo < fitobjects.size(); ++ifo) {
    BaseFitObject* fo =  fitobjects[ifo];
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

void BaseHardConstraint::test2ndDerivatives()
{
  B2INFO("BaseConstraint::test2ndDerivatives for " << getName());
  const int idim = 100;
  double* M = new double[idim * idim];
  for (int i = 0; i < idim * idim; ++i) M[i] = 0;
  add2ndDerivativesToMatrix(M, idim, 1);
  double eps = 0.0001;
  B2INFO("eps=" << eps);

  for (unsigned int ifo1 = 0; ifo1 < fitobjects.size(); ++ifo1) {
    BaseFitObject* fo1 =  fitobjects[ifo1];
    assert(fo1);
    for (unsigned int ifo2 = ifo1; ifo2 < fitobjects.size(); ++ifo2) {
      BaseFitObject* fo2 = fitobjects[ifo2];
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


double BaseHardConstraint::num1stDerivative(int ifo, int ilocal, double eps)
{
  BaseFitObject* fo =  fitobjects[ifo];
  assert(fo);
  double save = fo->getParam(ilocal);
  fo->setParam(ilocal, save + eps);
  double v1 = getValue();
  fo->setParam(ilocal, save - eps);
  double v2 = getValue();
  double result = (v1 - v2) / (2 * eps);
  fo->setParam(ilocal, save);
  return result;
}

double BaseHardConstraint::num2ndDerivative(int ifo1, int ilocal1, double eps1,
                                            int ifo2, int ilocal2, double eps2)
{
  double result;

  if (ifo1 == ifo2 && ilocal1 == ilocal2) {
    BaseFitObject* fo =  fitobjects[ifo1];
    assert(fo);
    double save = fo->getParam(ilocal1);
    double v0 = getValue();
    fo->setParam(ilocal1, save + eps1);
    double v1 = getValue();
    fo->setParam(ilocal1, save - eps1);
    double v2 = getValue();
    result = (v1 + v2 - 2 * v0) / (eps1 * eps1);
    fo->setParam(ilocal1, save);
  } else {
    BaseFitObject* fo1 =  fitobjects[ifo1];
    assert(fo1);
    BaseFitObject* fo2 =  fitobjects[ifo2];
    assert(fo2);
    double save1 = fo1->getParam(ilocal1);
    double save2 = fo2->getParam(ilocal2);
    fo1->setParam(ilocal1, save1 + eps1);
    fo2->setParam(ilocal2, save2 + eps2);
    double v11 = getValue();
    fo2->setParam(ilocal2, save2 - eps2);
    double v12 = getValue();
    fo1->setParam(ilocal1, save1 - eps1);
    double v22 = getValue();
    fo2->setParam(ilocal2, save2 + eps2);
    double v21 = getValue();
    result = (v11 + v22 - v12 - v21) / (4 * eps1 * eps2);
    fo1->setParam(ilocal1, save1);
    fo2->setParam(ilocal2, save2);
  }
  return result;
}

void BaseHardConstraint::printFirstDerivatives() const
{

  B2INFO("BaseHardConstraint::printFirstDerivatives " << fitobjects.size());

  double dgdpi[BaseDefs::MAXINTERVARS];
  for (unsigned int i = 0; i < fitobjects.size(); ++i) {
    const BaseFitObject* foi = fitobjects[i];
    assert(foi);
    if (firstDerivatives(i, dgdpi)) {
      B2INFO("first derivs for obj " << i << " : ");
      for (int j = 0; j < BaseDefs::MAXINTERVARS; j++)
        B2INFO(dgdpi[j] << " ");
    }
  }

  return;
}

void BaseHardConstraint::printSecondDerivatives() const
{

  double d2GdPidPj[BaseDefs::MAXINTERVARS * BaseDefs::MAXINTERVARS];

  int n = fitobjects.size();

  for (int i = 0; i < n; ++i) {
    const BaseFitObject* foi =  fitobjects[i];
    assert(foi);
    for (int j = 0; j < n; ++j) {
      const BaseFitObject* foj =  fitobjects[j];
      assert(foj);
      if (secondDerivatives(i, j, d2GdPidPj)) {

        B2INFO("second derivs for objs " << i << " " << j);

        int k(0);
        for (int k1 = 0; k1 < BaseDefs::MAXINTERVARS; k1++) {
          for (int k2 = 0; k2 < BaseDefs::MAXINTERVARS; k2++) {
            B2INFO(d2GdPidPj[k++] << " ");
          }
        }
      }
    }
  }

  return;
}
