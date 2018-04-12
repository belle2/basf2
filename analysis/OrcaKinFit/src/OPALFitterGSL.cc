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

#include<iostream>
#include<cmath>
#include<cassert>

#include "analysis/OrcaKinFit/OPALFitterGSL.h"

#include "analysis/OrcaKinFit/BaseFitObject.h"
#include "analysis/OrcaKinFit/BaseHardConstraint.h"
#include "analysis/OrcaKinFit/BaseTracer.h"
#include <framework/logging/Logger.h>

#include <gsl/gsl_block.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_permutation.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_cdf.h>

using std::cout;
using std::cerr;
using std::endl;
using std::abs;


// constructor
OPALFitterGSL::OPALFitterGSL()
  : npar(0), nmea(0), nunm(0), ncon(0), ierr(0), nit(0),
    fitprob(0), chi2(0),
    f(0), r(0), Fetaxi(0), S(0), Sinv(0), SinvFxi(0), SinvFeta(0),
    W1(0), G(0), H(0), HU(0), IGV(0), V(0), VLU(0), Vinv(0), Vnew(0),
    Minv(0), dxdt(0), Vdxdt(0),
    dxi(0), Fxidxi(0), lambda(0), FetaTlambda(0),
    etaxi(0), etasv(0), y(0), y_eta(0), Vinvy_eta(0), FetaV(0),
    permS(0), permU(0), permV(0), debug(0)
{}

// destructor
OPALFitterGSL::~OPALFitterGSL()
{
  // B2INFO( "destroying OPALFitterGSL");
  if (f) gsl_vector_free(f);
  if (r) gsl_vector_free(r);
  if (Fetaxi) gsl_matrix_free(Fetaxi);
  if (S) gsl_matrix_free(S);
  if (Sinv) gsl_matrix_free(Sinv);
  if (SinvFxi) gsl_matrix_free(SinvFxi);
  if (SinvFeta) gsl_matrix_free(SinvFeta);
  if (W1) gsl_matrix_free(W1);
  if (G) gsl_matrix_free(G);
  if (H) gsl_matrix_free(H);
  if (HU) gsl_matrix_free(HU);
  if (IGV) gsl_matrix_free(IGV);
  if (V) gsl_matrix_free(V);
  if (VLU) gsl_matrix_free(VLU);
  if (Vinv) gsl_matrix_free(Vinv);
  if (Vnew) gsl_matrix_free(Vnew);
  if (Minv) gsl_matrix_free(Minv);
  if (dxdt) gsl_matrix_free(dxdt);
  if (Vdxdt) gsl_matrix_free(Vdxdt);
  if (dxi) gsl_vector_free(dxi);
  if (Fxidxi) gsl_vector_free(Fxidxi);
  if (lambda) gsl_vector_free(lambda);
  if (FetaTlambda) gsl_vector_free(FetaTlambda);
  if (etaxi) gsl_vector_free(etaxi);
  if (etasv) gsl_vector_free(etasv);
  if (y) gsl_vector_free(y);
  if (y_eta) gsl_vector_free(y_eta);
  if (Vinvy_eta) gsl_vector_free(Vinvy_eta);
  if (FetaV) gsl_matrix_free(FetaV);
  if (permS) gsl_permutation_free(permS);
  if (permU) gsl_permutation_free(permU);
  if (permV) gsl_permutation_free(permV);

  f = 0;
  r = 0;
  Fetaxi = 0;
  S = 0;
  Sinv = 0;
  SinvFxi = 0;
  SinvFeta = 0;
  W1 = 0;
  G = 0;
  H = 0;
  HU = 0;
  IGV = 0;
  V = 0;
  VLU = 0;
  Vinv = 0;
  Vnew = 0;
  Minv = 0;
  dxdt = 0;
  Vdxdt = 0;
  dxi = 0;
  Fxidxi = 0;
  lambda = 0;
  FetaTlambda = 0;
  etaxi = 0;
  etasv = 0;
  y = 0;
  y_eta = 0;
  Vinvy_eta = 0;
  FetaV = 0;
  permS = 0;
  permU = 0;
  permV = 0;
}

// do it (~ transcription of WWFGO as of ww113)
double OPALFitterGSL::fit()
{



  //
  //           (     )   ^     ^
  //           ( eta )  nmea   |
  //           (     )   v     |
  //   etaxi = (-----)  ---   npar
  //           (     )   ^     |
  //           ( xi  )  nunm   |
  //           (     )   v     v

  //              <- ncon ->
  //             (          )   ^     ^
  //             (   Feta^T )  nmea   |
  //             (          )   v     |
  //  Fetaxi^T = ( -------- )  ---   npar
  //             (          )   ^     |
  //             (   Fxi^T  )  nunm   |
  //             (          )   v     v
  //
  //  Fetaxi are the derivatives of the constraints wrt the fitted parameters, thus A_theta/xi in book


  //
  //            <- nmea ->|<- nunm ->
  //           (          |          )   ^     ^
  //           ( Vetaeta  |  Vetaxi  )  nmea   |
  //           (          |          )   v     |
  //  V =      (----------+----------)  ---   npar
  //           (          |          )   ^     |
  //           (  Vxieta  |  Vxixi   )  nunm   |
  //           (          |          )   v     v
  //
  //  V is the covariance matrix. Before the fit only Vetaeta is non-zero (covariance of measured parameters)

  //
  //                 <- nmea ->|<- nunm ->
  //                (          |         )   ^
  //     dxdt^T =   (   detadt |   dxidt )  nmea
  //                (          |         )   v
  //
  //  dxdt are the partial derivates of the fitted parameters wrt the measured parameters,
  //
  //                 <- nmea ->|<- nunm ->
  //                (          |          )   ^
  //     Vdxdt  =   ( Vdetadt  |  Vdxidt  )  nmea
  //                (          |          )   v
  //
  //  Vdxdt is Vetaeta * dxdt^T, thus Vdxdt[nmea][npar]
  //  both needed for calculation of the full covariance matrix of the fitted parameters

  // cout statements
  int inverr = 0;

  // order parameters etc
  initialize();

  assert(f && (int)f->size == ncon);
  assert(r && (int)r->size == ncon);
  assert(Fetaxi && (int)Fetaxi->size1 == ncon && (int)Fetaxi->size2 == npar);
  assert(S && (int)S->size1 == ncon && (int)S->size2 == ncon);
  assert(Sinv && (int)Sinv->size1 == ncon && (int)Sinv->size2 == ncon);
  assert(nunm == 0 || (SinvFxi && (int)SinvFxi->size1 == ncon && (int)SinvFxi->size2 == nunm));
  assert(SinvFeta && (int)SinvFeta->size1 == ncon && (int)SinvFeta->size2 == nmea);
  assert(nunm == 0 || (W1 && (int)W1->size1 == nunm && (int)W1->size2 == nunm));
  assert(G && (int)G->size1 == nmea && (int)G->size2 == nmea);
  assert(nunm == 0 || (H && (int)H->size1 == nmea && (int)H->size2 == nunm));
  assert(nunm == 0 || (HU && (int)HU->size1 == nmea && (int)HU->size2 == nunm));
  assert(IGV && (int)IGV->size1 == nmea && (int)IGV->size2 == nmea);
  assert(V && (int)V->size1 == npar && (int)V->size2 == npar);
  assert(VLU && (int)VLU->size1 == nmea && (int)VLU->size2 == nmea);
  assert(Vinv && (int)Vinv->size1 == nmea && (int)Vinv->size2 == nmea);
  assert(Vnew && (int)Vnew->size1 == npar && (int)Vnew->size2 == npar);
  assert(Minv && (int)Minv->size1 == npar && (int)Minv->size2 == npar);
  assert(dxdt && (int)dxdt->size1 == npar && (int)dxdt->size2 == nmea);
  assert(Vdxdt  && (int)Vdxdt->size1  == nmea && (int)Vdxdt->size2  == npar);
  assert(nunm == 0 || (dxi && (int)dxi->size == nunm));
  assert(nunm == 0 || (Fxidxi && (int)Fxidxi->size == ncon));
  assert(lambda && (int)lambda->size == ncon);
  assert(FetaTlambda && (int)FetaTlambda->size == nmea);
  assert(etaxi && (int)etaxi->size == npar);
  assert(etasv && (int)etasv->size == npar);
  assert(y && (int)y->size == nmea);
  assert(y_eta && (int)y_eta->size == nmea);
  assert(Vinvy_eta && (int)Vinvy_eta->size == nmea);
  assert(FetaV && (int)FetaV->size1 == ncon && (int)FetaV->size2 == nmea);
  assert(permS && (int)permS->size == ncon);
  assert(nunm == 0 || (permU && (int)permU->size == nunm));
  assert(permV && (int)permV->size == nmea);

  // eta is the part of etaxi containing the measured quantities
  gsl_vector_view eta = gsl_vector_subvector(etaxi, 0, nmea);

  // Feta is the part of Fetaxi containing the measured quantities

  B2DEBUG(1, "==== " << ncon << " " << nmea);

  gsl_matrix_view Feta = gsl_matrix_submatrix(Fetaxi, 0, 0, ncon, nmea);

  gsl_matrix_view Vetaeta   = gsl_matrix_submatrix(V, 0, 0, nmea, nmea);

  for (unsigned int i = 0; i < fitobjects.size(); ++i) {
    for (int ilocal = 0; ilocal < fitobjects[i]->getNPar(); ++ilocal) {
      if (!fitobjects[i]->isParamFixed(ilocal)) {
        int iglobal = fitobjects[i]->getGlobalParNum(ilocal);
        assert(iglobal >= 0 && iglobal < npar);
        gsl_vector_set(etaxi, iglobal, fitobjects[i]->getParam(ilocal));
        if (fitobjects[i]->isParamMeasured(ilocal)) {
          assert(iglobal < nmea);
          gsl_vector_set(y, iglobal, fitobjects[i]->getMParam(ilocal));
        }
        B2DEBUG(0, "etaxi[" << iglobal << "] = " << gsl_vector_get(etaxi, iglobal)
                << " for jet " << i << " and ilocal = " << ilocal);
      }
    }
  }

  /// initialize Fetaxi ( = d F / d eta,xi)
  gsl_matrix_set_zero(Fetaxi);
  for (int k = 0; k < ncon; k++) {
    constraints[k]->getDerivatives(Fetaxi->size2, Fetaxi->block->data + k * Fetaxi->tda);
    if (debug > 1) for (int j = 0; j < npar; j++)
        if (gsl_matrix_get(Fetaxi, k, j) != 0)
          B2INFO("1: Fetaxi[" << k << "][" << j << "] = " << gsl_matrix_get(Fetaxi, k, j));
  }

  // chi2's, step size, # iterations
  double chinew = 0, chit = 0, chik = 0;
  double alph = 1.;
  nit = 0;
  // convergence criteria (as in WWINIT)
  // int nitmax = 20;
  int nitmax = 200;
  double chik0 = 100.;
  double chit0 = 100.;
  double dchikc = 1.0E-3;
  double dchitc = 1.0E-4;
  double dchikt = 1.0E-2;
  double dchik  = 1.05;
  double chimxw = 10000.; // 1000.;
  double almin = 0.05;

  // repeat with or with out smaller steps size
  bool repeat = true;
  bool scut = false;
  bool calcerr = true;

#ifndef FIT_TRACEOFF
  if (tracer) tracer->initialize(*this);
#endif


  // start of iterations
  while (repeat) {
    bool updatesuccess = true;

//*-- If necessary, retry smaller step, same direction
    if (scut) {
      gsl_vector_memcpy(etaxi, etasv);
      updatesuccess = updateFitObjects(etaxi->block->data);
      if (!updatesuccess) {
        std::cerr << "OPALFitterGSL::fit: old parameters are garbage!" << std::endl;
        return -1;
      }


      gsl_matrix_set_zero(Fetaxi);
      for (int k = 0; k < ncon; ++k)  {
        constraints[k]->getDerivatives(Fetaxi->size2, Fetaxi->block->data + k * Fetaxi->tda);
      }
      if (debug > 1) debug_print(Fetaxi, "1: Fetaxi");
    } else {
      gsl_vector_memcpy(etasv, etaxi);
      chik0 = chik;
      chit0 = chit;
    }

    // Get covariance matrix
    gsl_matrix_set_zero(V);

    for (unsigned int ifitobj = 0; ifitobj < fitobjects.size(); ++ifitobj) {
      fitobjects[ifitobj]->addToGlobCov(V->block->data, V->tda);
    }
    if (debug > 1)  debug_print(V, "V");

    gsl_matrix_memcpy(VLU, &Vetaeta.matrix);

    // invert covariance matrix (needed for chi2 calculation later)

    int signum;
    int result;
    result = gsl_linalg_LU_decomp(VLU, permV, &signum);
    B2DEBUG(1, "gsl_linalg_LU_decomp result=" << result);
    if (debug > 3)  debug_print(VLU, "VLU");

    result = gsl_linalg_LU_invert(VLU, permV, Vinv);
    B2DEBUG(1, "gsl_linalg_LU_invert result=" << result);

    if (debug > 2) debug_print(Vinv, "Vinv");

// *-- Evaluate f and S.
    for (int k = 0; k < ncon; ++k) {
      gsl_vector_set(f, k, constraints[k]->getValue());
    }
    if (debug > 1)  debug_print(f, "f");

    // y_eta = y - eta
    gsl_vector_memcpy(y_eta, y);
    gsl_vector_sub(y_eta, &eta.vector);
    // r=f
    gsl_vector_memcpy(r, f);
    // r = 1*Feta*y_eta + 1*r
    gsl_blas_dgemv(CblasNoTrans, 1, &Feta.matrix, y_eta, 1, r);

    if (debug > 1) debug_print(r, "r");

    // S = Feta * V * Feta^T

    //FetaV = 1*Feta*V + 0*FetaV
    //B2DEBUG(2, "Creating FetaV");
    gsl_blas_dsymm(CblasRight, CblasUpper, 1, &Vetaeta.matrix, &Feta.matrix, 0,  FetaV);
    // S = 1 * FetaV * Feta^T + 0*S
    //B2DEBUG(2, "Creating S");
    gsl_blas_dgemm(CblasNoTrans, CblasTrans, 1, FetaV, &Feta.matrix, 0, S);

    if (nunm > 0) {
      // New invention by B. List, 6.12.04:
      // add F_xi * F_xi^T to S, to make method work when some
      // constraints do not depend on any measured parameter

      // Fxi is the part of Fetaxi containing the unmeasured quantities, if any
      gsl_matrix_view Fxi = gsl_matrix_submatrix(Fetaxi,  0, nmea, ncon, nunm);

      //S = 1*Fxi*Fxi^T + 1*S
      gsl_blas_dgemm(CblasNoTrans, CblasTrans, 1, &Fxi.matrix, &Fxi.matrix, 1, S);
    }

    if (debug > 1) debug_print(S, "S");

// *-- Invert S to Sinv; S is destroyed here!
// S is symmetric and positive definite

    gsl_linalg_LU_decomp(S, permS, &signum);
    inverr = gsl_linalg_LU_invert(S, permS, Sinv);

    if (inverr != 0) {
      cerr << "S: gsl_linalg_LU_invert error " << inverr << endl;
      ierr = 7;
      calcerr = false;
      break;
    }

    // Calculate S^1*r here, we will need it
    // Store it in lambda!
    // lambda = 1*Sinv*r + 0*lambda; Sinv is symmetric
    gsl_blas_dsymv(CblasUpper, 1, Sinv, r, 0, lambda);

// *-- Calculate new unmeasured quantities, if any

    if (nunm > 0) {
      // Fxi is the part of Fetaxi containing the unmeasured quantities, if any
      gsl_matrix_view Fxi = gsl_matrix_submatrix(Fetaxi,  0, nmea, ncon, nunm);
      // W1 = Fxi^T * Sinv * Fxi
      // SinvFxi = 1*Sinv*Fxi + 0*SinvFxi
      gsl_blas_dsymm(CblasLeft, CblasUpper, 1, Sinv, &Fxi.matrix, 0,  SinvFxi);
      // W1 = 1*Fxi^T*SinvFxi + 0*W1
      gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1, &Fxi.matrix, SinvFxi, 0, W1);

      if (debug > 1) {
        debug_print(W1, "W1");
        // Check symmetry of W1
        for (int i = 0; i < nunm; ++i) {
          for (int j = 0; j < nunm; ++j) {
            if (abs(gsl_matrix_get(W1, i, j) - gsl_matrix_get(W1, j, i)) > 1E-3 * abs(gsl_matrix_get(W1, i, j) + gsl_matrix_get(W1, j, i)))
              B2INFO("W1[" << i << "][" << j << "] = " << gsl_matrix_get(W1, i, j)
                     << "   W1[" << j << "][" << i << "] = " << gsl_matrix_get(W1, j, i)
                     << "   => diff=" << abs(gsl_matrix_get(W1, i, j) - gsl_matrix_get(W1, j, i))
                     << "   => tol=" << 1E-3 * abs(gsl_matrix_get(W1, i, j) + gsl_matrix_get(W1, j, i)));
          }
        }
      }

      // calculate shift of unmeasured parameters

      // dxi = -alph*W1^-1 * Fxi^T*Sinv*r
      // => dxi is solution of W1*dxi = -alph*Fxi^T*Sinv*r
      // Compute rights hand side first
      // Sinv*r was already calculated and is stored in lambda
      // dxi = -alph*Fxi^T*lambda + 0*dxi

      B2DEBUG(1, "alph = " << alph);
      if (debug > 1) debug_print(lambda, "lambda");
      if (debug > 1) debug_print(&(Fxi.matrix), "Fxi");

      gsl_blas_dgemv(CblasTrans, -alph, &Fxi.matrix, lambda, 0, dxi);

      if (debug > 1) debug_print(dxi, "dxi0");
      if (debug > 1) debug_print(W1, "W1");

      // now solve the system
      // Note added 23.12.04: W1 is symmetric and positive definite,
      // so we can use the Cholesky instead of LU decomposition
      gsl_linalg_cholesky_decomp(W1);
      inverr = gsl_linalg_cholesky_svx(W1, dxi);

      if (debug > 1) debug_print(dxi, "dxi1");


      if (inverr != 0) {
        cerr << "W1: gsl_linalg_cholesky_svx error " << inverr << endl;
        ierr = 8;
        calcerr = false;
        break;
      }

      if (debug > 1) debug_print(dxi, "dxi");

//    *-- And now update unmeasured parameters; xi is a view of etaxi
      // xi is the part of etaxi containing the unmeasured quantities, if any
      gsl_vector_view xi = gsl_vector_subvector(etaxi, nmea, nunm);

      // xi += dxi
      gsl_vector_add(&xi.vector, dxi);

    }

// *-- Calculate new Lagrange multipliers.
    // lambda = Sinv*r + Sinv*Fxi*dxi
    // lambda is already set to Sinv*r, we just need to add Sinv*Fxi*dxi

    if (nunm > 0) {
      // Fxi is the part of Fetaxi containing the unmeasured quantities, if any
      gsl_matrix_view Fxi = gsl_matrix_submatrix(Fetaxi,  0, nmea, ncon, nunm);
      // calculate Fxidxi = 1*Fxi*dxi + 0*Fxidxi
      gsl_blas_dgemv(CblasNoTrans, 1, &Fxi.matrix, dxi, 0, Fxidxi);
      // add to existing lambda: lambda = 1*Sinv*Fxidxi + 1*lambda; Sinv is symmetric
      gsl_blas_dsymv(CblasUpper, 1, Sinv, Fxidxi, 1, lambda);

    }

    if (debug > 1) debug_print(lambda, "lambda");

// *-- Calculate new fitted parameters:
    // eta = y - V*Feta^T*lambda
    gsl_vector_memcpy(&eta.vector, y);
    // FetaTlambda = 1*Feta^T*lambda + 0*FetaTlambda
    gsl_blas_dgemv(CblasTrans, 1, &Feta.matrix, lambda, 0, FetaTlambda);
    // eta = -1*V*FetaTlambda + 1*eta; V is symmetric
    gsl_blas_dsymv(CblasUpper, -1, &Vetaeta.matrix, FetaTlambda, 1, &eta.vector);


    if (debug > 1) debug_print(&eta.vector, "updated eta");

// *-- Calculate constraints and their derivatives.
    // since the constraints ask the fitobjects for their parameters,
    // we need to update the fitobjects first!
    // COULD BE DONE: update also ERRORS! (now only in the very end!)
    updatesuccess = updateFitObjects(etaxi->block->data);

    B2DEBUG(0, "After adjustment of all parameters:\n");
    for (int k = 0; k < ncon; ++k) {
      B2DEBUG(0, "Value of constraint " << k << " = " << constraints[k]->getValue());
    }
    gsl_matrix_set_zero(Fetaxi);
    for (int k = 0; k < ncon; k++) {
      constraints[k]->getDerivatives(Fetaxi->size2, Fetaxi->block->data + k * Fetaxi->tda);
    }
    if (debug > 1)  debug_print(Fetaxi, "2: Fetaxi");


// *-- Calculate new chisq.
    // First, calculate new y-eta
    // y_eta = y - eta
    gsl_vector_memcpy(y_eta, y);
    gsl_vector_sub(y_eta, &eta.vector);
    // Now calculate Vinv*y_eta [ as solution to V* Vinvy_eta = y_eta]
    // Vinvy_eta = 1*Vinv*y_eta + 0*Vinvy_eta; Vinv is symmetric
    gsl_blas_dsymv(CblasUpper, 1, Vinv, y_eta, 0, Vinvy_eta);
    // Now calculate y_eta *Vinvy_eta
    gsl_blas_ddot(y_eta, Vinvy_eta, &chit);

    for (int i = 0; i < nmea; ++i)
      for (int j = 0; j < nmea; ++j) {
        double dchit = (gsl_vector_get(y_eta, i)) *
                       gsl_matrix_get(Vinv, i, j) *
                       (gsl_vector_get(y_eta, j));
        if (dchit != 0)
          B2DEBUG(1, "chit for i,j = " << i << " , " << j << " = "
                  << dchit);
      }

    chik = 0;
    for (int k = 0; k < ncon; ++k) chik += std::abs(2 * gsl_vector_get(lambda, k) * constraints[k]->getValue());
    chinew = chit + chik;

//*-- Calculate change in chisq, and check constraints are satisfied.
    nit++;

    bool sconv = (std::abs(chik - chik0) < dchikc)
                 && (std::abs(chit - chit0) < dchitc * chit)
                 && (chik < dchikt * chit);
    // Second convergence criterium:
    // If all constraints are fulfilled to better than 1E-8,
    // and all parameters have changed by less than 1E-8,
    // assume convergence
    // This criterium assumes that all constraints and all parameters
    // are "of order 1", i.e. their natural values are around 1 to 100,
    // as for GeV or radians
    double eps = 1E-6;
    bool sconv2 = true;
    for (int k = 0; sconv2 && (k < ncon); ++k)
      sconv2 &= (std::abs(gsl_vector_get(f, k)) < eps);
    if (sconv2)
      B2DEBUG(0, "All constraints fulfilled to better than " << eps);

    for (int j = 0; sconv2 && (j < npar); ++j)
      sconv2 &= (std::abs(gsl_vector_get(etaxi, j) - gsl_vector_get(etasv, j)) < eps);
    if (sconv2)
      B2DEBUG(0, "All parameters stable to better than " << eps);
    sconv |= sconv2;

    bool sbad  = (chik > dchik * chik0)
                 && (chik > dchikt * chit)
                 && (chik > chik0 + 1.E-10);

    scut = false;

    if (nit > nitmax) {
// *-- Out of iterations
      repeat = false;
      calcerr = false;
      ierr = 1;
    } else if (sconv && updatesuccess) {
// *-- Converged
      repeat = false;
      calcerr = true;
      ierr = 0;
    } else if (nit > 2 && chinew > chimxw  && updatesuccess) {
// *-- Chi2  crazy ?
      repeat = false;
      calcerr = false;
      ierr = 2;
    } else if ((sbad && nit > 1) || !updatesuccess) {
// *-- ChiK increased, try smaller step
      if (alph == almin) {
        repeat = true;   // false;
        ierr = 3;
      } else {
        alph  =  std::max(almin, 0.5 * alph);
        scut  =  true;
        repeat = true;
        ierr = 4;
      }
    } else {
// *-- Keep going..
      alph  =  std::min(alph + 0.1, 1.);
      repeat = true;
      ierr = 5;
    }

    B2DEBUG(0, "======== NIT = " << nit << ",  CHI2 = " << chinew
            << ",  ierr = " << ierr << ", alph=" << alph);

    for (unsigned int i = 0; i < fitobjects.size(); ++i)
      B2DEBUG(0, "fitobject " << i << ": " << *fitobjects[i]);

#ifndef FIT_TRACEOFF
    if (tracer) tracer->step(*this);
#endif

  }   // end of while (repeat)

// *-- Turn chisq into probability.
  fitprob = (ncon - nunm > 0) ? gsl_cdf_chisq_Q(chinew, ncon - nunm) : 0.5;
  chi2 = chinew;

// *-- End of iterations - calculate errors.

// The result will (ultimately) be stored in Vnew

  gsl_matrix_set_zero(Vnew);
  gsl_matrix_set_zero(Minv);
  if (debug > 2) {
    for (int i = 0; i < npar; ++i) {
      for (int j = 0; j < npar; ++j) {
        B2INFO("Minv[" << i << "," << j << "]=" << gsl_matrix_get(Minv, i, j));
      }
    }
  }

  B2DEBUG(0, "OPALFitterGSL: calcerr = " << calcerr);

  if (calcerr) {

// *-- As a first step, calculate Minv as in 9.4.2 of Benno's book chapter
//                    (in O.Behnke et al "Data Analysis in High Energy Physics")


// *-- Evaluate S and invert.

    if (debug > 2) debug_print(&Vetaeta.matrix, "V");
    if (debug > 2) debug_print(&Feta.matrix, "Feta");

    // CblasRight means C = alpha B A + beta C with symmetric matrix A
    //FetaV[ncon][nmea] = 1*Feta[ncon][nmea]*V[nmea][nmea] + 0*FetaV
    gsl_blas_dsymm(CblasRight, CblasUpper, 1, &Vetaeta.matrix, &Feta.matrix, 0,  FetaV);
    // S[ncon][ncon] = 1 * FetaV[ncon][nmea] * Feta^T[nmea][ncon] + 0*S
    gsl_blas_dgemm(CblasNoTrans, CblasTrans, 1, FetaV, &Feta.matrix, 0, S);


    if (nunm > 0) {
      // New invention by B. List, 6.12.04:
      // add F_xi * F_xi^T to S, to make method work when some
      // constraints do not depend on any measured parameter

      // Fxi is the part of Fetaxi containing the unmeasured quantities, if any
      gsl_matrix_view Fxi = gsl_matrix_submatrix(Fetaxi,  0, nmea, ncon, nunm);
      //S[ncon][ncon] = 1*Fxi[ncon][nunm]*Fxi^T[nunm][ncon] + 1*S[ncon][ncon]
      gsl_blas_dgemm(CblasNoTrans, CblasTrans, 1, &Fxi.matrix, &Fxi.matrix, 1, S);
    }

    if (debug > 2) debug_print(S, "S");

// *-- Invert S, testing for singularity first.
// S is symmetric and positive definite

    int signum;
    gsl_linalg_LU_decomp(S, permS, &signum);
    inverr = gsl_linalg_LU_invert(S, permS, Sinv);

    if (inverr != 0) {
      cerr << "S: gsl_linalg_LU_invert error " << inverr << " in error calculation" << endl;
      ierr = -1;
      return -1;
    }


// *-- Calculate G.
//  (same as W1, but for measured parameters)
// G = Feta^T * Sinv * Feta

    // SinvFeta[ncon][nmea] = 1*Sinv[ncon][ncon]*Feta[ncon][nmea] + 0*SinvFeta
    gsl_blas_dsymm(CblasLeft, CblasUpper, 1, Sinv, &Feta.matrix, 0,  SinvFeta);
    // G[nmea][nmea] = 1*Feta^T[nmea][ncon]*SinvFeta[ncon][nmea] + 0*G
    gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1, &Feta.matrix, SinvFeta, 0, G);

    if (debug > 2) debug_print(G, "G(1)");


    if (nunm > 0) {

// *-- Calculate H

      // Fxi is the part of Fetaxi containing the unmeasured quantities, if any
      gsl_matrix_view Fxi = gsl_matrix_submatrix(Fetaxi,  0, nmea, ncon, nunm);
      // H = Feta^T * Sinv * Fxi
      // SinvFxi[ncon][nunm] = 1*Sinv[ncon][ncon]*Fxi[ncon][nunm] + 0*SinvFxi
      gsl_blas_dsymm(CblasLeft, CblasUpper, 1, Sinv, &Fxi.matrix, 0,  SinvFxi);
      // H[nmea][nunm] = 1*Feta^T[nmea][ncon]*SinvFxi[ncon][nunm] + 0*H
      gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1, &Feta.matrix, SinvFxi, 0, H);

      if (debug > 2) debug_print(H, "H");

// *-- Calculate U**-1 and invert.
//   (same as W1)
//   U is a part of Minv

      gsl_matrix* Uinv = W1;
      gsl_matrix_view U = gsl_matrix_submatrix(Minv, nmea, nmea, nunm, nunm);
      // Uinv = Fxi^T * Sinv * Fxi
      // Uinv[nunm][nunm] = 1*Fxi^T[nunm][ncon]*SinvFxi[ncon][nunm] + 0*W1
      gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1, &Fxi.matrix, SinvFxi, 0, Uinv);

      gsl_linalg_LU_decomp(Uinv, permU, &signum);
      inverr = gsl_linalg_LU_invert(Uinv, permU, &U.matrix);

      if (debug > 2) debug_print(&U.matrix, "U");
      for (int i = 0; i < npar; ++i) {
        for (int j = 0; j < npar; ++j) {
          B2DEBUG(2, "after U Minv[" << i << "," << j << "]=" << gsl_matrix_get(Minv, i, j));
        }
      }

      if (inverr != 0) {
        cerr << "U: gsl_linalg_LU_invert error " << inverr << " in error calculation " << endl;
        ierr = -1;
        return -1;
      }

// *-- Covariance matrix between measured and unmeasured parameters.

//    HU[nmea][nunm] = 1*H[nmea][nunm]*U[nunm][nunm] + 0*HU
      gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1, H, &U.matrix, 0, HU);
//    Vnewetaxi is a view of Vnew
      gsl_matrix_view Minvetaxi = gsl_matrix_submatrix(Minv, 0, nmea, nmea, nunm);
      gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, -1, &Vetaeta.matrix, HU, 0, &Minvetaxi.matrix);
      for (int i = 0; i < npar; ++i) {
        for (int j = 0; j < npar; ++j) {
          B2DEBUG(2, "after etaxi Minv[" << i << "," << j << "]=" << gsl_matrix_get(Minv, i, j));
        }
      }


// *-- Fill in symmetric part:
//    Vnewxieta is a view of Vnew
      gsl_matrix_view Minvxieta = gsl_matrix_submatrix(Minv, nmea, 0, nunm, nmea);
      gsl_matrix_transpose_memcpy(&Minvxieta.matrix, &Minvetaxi.matrix);
      for (int i = 0; i < npar; ++i) {
        for (int j = 0; j < npar; ++j) {
          B2DEBUG(2, "after symmetric: Minv[" << i << "," << j << "]=" << gsl_matrix_get(Minv, i, j));
        }
      }

// *-- Calculate G-HUH^T:
//    G = -1*HU*H^T +1*G
      gsl_blas_dgemm(CblasNoTrans, CblasTrans, -1, HU, H, +1, G);

    }  // endif nunm > 0

// *-- Calculate I-GV.
    // IGV = 1
    gsl_matrix_set_identity(IGV);
    // IGV = -1*G*Vetaeta + 1*IGV
    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, -1, G, &Vetaeta.matrix, 1, IGV);

// *-- And finally error matrix on fitted parameters.
    gsl_matrix_view Minvetaeta = gsl_matrix_submatrix(Minv, 0, 0, nmea, nmea);

    // Vnewetaeta = 1*Vetaeta*IGV + 0*Vnewetaeta
    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1, &Vetaeta.matrix, IGV, 0, &Minvetaeta.matrix);

    for (int i = 0; i < npar; ++i) {
      for (int j = 0; j < npar; ++j) {
        B2DEBUG(2, "complete Minv[" << i << "," << j << "]=" << gsl_matrix_get(Minv, i, j));
      }
    }

// *-- now Minv should be complete, can calculate new covariance matrix Vnew
//       Vnew = (dx / dt)^T  * V_t * dx / dt
//       x are the fitted parameters, t are the measured parameters,
//       V_t is the covariance matrix of the measured parameters
//       thus in OPALFitter variables:  V_t = Vetaeta,  x = (eta, xi) = etaxi
//       d eta / dt and d xi / dt are given by eqns 9.54 and 9.55, respectively,
//       as deta/dt = - Minvetaeta * Fetat and dxi/dt = - Minvxieta * Fetat
//       Fetat is d^2 chi^2 / deta dt = - V^-1, even in presence of soft constraints, since
//       the soft constraints don't depend on the _measured_ parameters t (but on the fitted ones)
//       HERE,  V (and Vinv) do not include the second derivatives of the soft constraints
//       so we can use them right away
//       Note that "F" is used for completely different things:
//       in OPALFitter it is the derivatives of the constraints wrt to the parameters (A in book)
//       in the book, F are the second derivatives of the objective function wrt the parameters


    gsl_matrix_view detadt = gsl_matrix_submatrix(dxdt, 0, 0, nmea, nmea);
    B2DEBUG(3, "after detadt");
    //  Vdxdt is Vetaeta * dxdt^T, thus Vdxdt[nmea][npar]
    gsl_matrix_view Vdetadt = gsl_matrix_submatrix(Vdxdt, 0, 0, nmea, nmea);
    B2DEBUG(3, "after Vdetadt");

    // detadt = - Minvetaeta * Fetat = -1 * Minvetaeta * (-1) * Vinv + 0 * detadt   // replace by symm?
    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1, &Minvetaeta.matrix, Vinv, 0, &detadt.matrix);
    if (debug > 2) debug_print(&detadt.matrix, "deta/dt");

    // Vdetadt = 1 * Vetaeta * detadt^T + 0* Vdetadt
    gsl_blas_dgemm(CblasNoTrans, CblasTrans, 1, &Vetaeta.matrix, &detadt.matrix, 0, &Vdetadt.matrix);   // ok
    if (debug > 2) debug_print(&Vdetadt.matrix, "Vetata * deta/dt");

    gsl_matrix_view Vnewetaeta = gsl_matrix_submatrix(Vnew, 0, 0, nmea, nmea);    //[nmea],[nmea]
    // Vnewetaeta = 1 * detadt * Vdetadt + 0* Vnewetaeta
    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1, &detadt.matrix, &Vdetadt.matrix, 0, &Vnewetaeta.matrix);

    if (debug > 2) debug_print(Vnew, "Vnew after part for measured parameters");


    if (nunm > 0) {

      gsl_matrix_view Minvxieta = gsl_matrix_submatrix(Minv, nmea, 0, nunm, nmea);   //[nunm][nmea]
      B2DEBUG(3, "after Minvxieta");
      if (debug > 2) debug_print(&Minvxieta.matrix, "Minvxieta");

      gsl_matrix_view dxidt = gsl_matrix_submatrix(dxdt, nmea, 0, nunm, nmea);       //[nunm][nmea]
      B2DEBUG(3, "after dxidt");
      // dxidt[nunm][nmea] = - Minvxieta * Fetat = -1 * Minvxieta[nunm][nmea] * Vinv[nmea][nmea] + 0 * dxidt
      gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1, &Minvxieta.matrix, Vinv, 0, &dxidt.matrix);    //ok
      if (debug > 2) debug_print(&dxidt.matrix, "dxi/dt");

      // Vdxdt = V * dxdt^T => Vdxdt[nmea][npar]
      gsl_matrix_view Vdxidt = gsl_matrix_submatrix(Vdxdt, 0, nmea, nmea, nunm);     //[nmea][nunm]
      B2DEBUG(3, "after Vdxidt");
      // Vdxidt = 1 * Vetaeta[nmea][nmea] * dxidt^T[nmea][nunm] + 0* Vdxidt => Vdxidt[nmea][nunm]
      gsl_blas_dgemm(CblasNoTrans, CblasTrans, 1, &Vetaeta.matrix, &dxidt.matrix, 0, &Vdxidt.matrix);   // ok
      if (debug > 2) debug_print(&Vdxidt.matrix, "Vetaeta * dxi/dt^T");

      gsl_matrix_view Vnewetaxi = gsl_matrix_submatrix(Vnew, 0, nmea, nmea, nunm);     //[nmea][nunm]
      gsl_matrix_view Vnewxieta = gsl_matrix_submatrix(Vnew, nmea, 0, nunm, nmea);     //[nunm][nmea]
      gsl_matrix_view Vnewxixi = gsl_matrix_submatrix(Vnew, nmea, nmea, nunm, nunm);   //[nunm][nunm]

      // Vnewxieta[nunm][nmea] = 1 * dxidt[nunm][nmea] * Vdetadt[nmea][nmea] + 0* Vnewxieta
      gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1, &dxidt.matrix, &Vdetadt.matrix, 0, &Vnewxieta.matrix);   // ok
      if (debug > 2) debug_print(Vnew, "Vnew after xieta part");
      // Vnewetaxi[nmea][nunm] = 1 * detadt[nmea][nmea] * Vdxidt[nmea][nunm] + 0* Vnewetaxi
      gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1, &detadt.matrix, &Vdxidt.matrix, 0, &Vnewetaxi.matrix);   // ok
      if (debug > 2) debug_print(Vnew, "Vnew after etaxi part");
      // Vnewxixi[nunm][nunm] = 1 * dxidt[nunm][nmea] * Vdxidt[nmea][nunm] + 0* Vnewxixi
      gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1, &dxidt.matrix, &Vdxidt.matrix, 0, &Vnewxixi.matrix);
      if (debug > 2) debug_print(Vnew, "Vnew after xixi part");
    }

// *-- now we finally have Vnew, fill into fitobjects

    // update errors in fitobjects
    for (unsigned int ifitobj = 0; ifitobj < fitobjects.size(); ++ifitobj) {
      for (int ilocal = 0; ilocal < fitobjects[ifitobj]->getNPar(); ++ilocal) {
        int iglobal = fitobjects[ifitobj]->getGlobalParNum(ilocal);
        for (int jlocal = ilocal; jlocal < fitobjects[ifitobj]->getNPar(); ++jlocal) {
          int jglobal = fitobjects[ifitobj]->getGlobalParNum(jlocal);
          if (iglobal >= 0 && jglobal >= 0)
            fitobjects[ifitobj]->setCov(ilocal, jlocal, gsl_matrix_get(Vnew, iglobal, jglobal));
        }
      }
    }

    // Finally, copy covariance matrix
    if (cov && covDim != nmea + nunm) {
      delete[] cov;
      cov = 0;
    }
    covDim = nmea + nunm;
    if (!cov) cov = new double[covDim * covDim];
    for (int i = 0; i < covDim; ++i) {
      for (int j = 0; j < covDim; ++j) {
        cov[i * covDim + j] = gsl_matrix_get(Vnew, i, j);
      }
    }
    covValid = true;

  } // endif calcerr == true

#ifndef FIT_TRACEOFF
  if (tracer) tracer->finish(*this);
#endif

  return fitprob;

}

bool OPALFitterGSL::initialize()
{
  covValid = false;
  // tell fitobjects the global ordering of their parameters:
  int iglobal = 0;
  // measured parameters first!
  for (unsigned int ifitobj = 0; ifitobj < fitobjects.size(); ++ifitobj) {
    for (int ilocal = 0; ilocal < fitobjects[ifitobj]->getNPar(); ++ilocal) {
      if (fitobjects[ifitobj]->isParamMeasured(ilocal) &&
          !fitobjects[ifitobj]->isParamFixed(ilocal)) {
        fitobjects[ifitobj]->setGlobalParNum(ilocal, iglobal);
        B2DEBUG(0, "Object " << fitobjects[ifitobj]->getName()
                << " Parameter " << fitobjects[ifitobj]->getParamName(ilocal)
                << " is measured, global number " << iglobal);
        ++iglobal;
      }
    }
  }
  nmea = iglobal;
  // now  unmeasured parameters!
  for (unsigned int ifitobj = 0; ifitobj < fitobjects.size(); ++ifitobj) {
    for (int ilocal = 0; ilocal < fitobjects[ifitobj]->getNPar(); ++ilocal) {
      if (!fitobjects[ifitobj]->isParamMeasured(ilocal) &&
          !fitobjects[ifitobj]->isParamFixed(ilocal)) {
        fitobjects[ifitobj]->setGlobalParNum(ilocal, iglobal);
        B2DEBUG(0, "Object " << fitobjects[ifitobj]->getName()
                << " Parameter " << fitobjects[ifitobj]->getParamName(ilocal)
                << " is unmeasured, global number " << iglobal);
        ++iglobal;
      }
    }
  }
  npar = iglobal;
  assert(npar <= NPARMAX);
  nunm = npar - nmea;
  assert(nunm <= NUNMMAX);

  // set number of constraints
  ncon = constraints.size();
  assert(ncon <= NCONMAX);

  ini_gsl_vector(f, ncon);
  ini_gsl_vector(r, ncon);

  ini_gsl_matrix(Fetaxi, ncon, npar);
  ini_gsl_matrix(S, ncon, ncon);
  ini_gsl_matrix(Sinv, ncon, ncon);
  ini_gsl_matrix(SinvFxi, ncon, nunm);
  ini_gsl_matrix(SinvFeta, ncon, nmea);
  ini_gsl_matrix(W1, nunm, nunm);
  ini_gsl_matrix(G, nmea, nmea);
  ini_gsl_matrix(H, nmea, nunm);
  ini_gsl_matrix(HU, nmea, nunm);
  ini_gsl_matrix(IGV, nmea, nmea);
  ini_gsl_matrix(V, npar, npar);
  ini_gsl_matrix(VLU, nmea, nmea);
  ini_gsl_matrix(Vinv, nmea, nmea);
  ini_gsl_matrix(Vnew, npar, npar);
  ini_gsl_matrix(Minv, npar, npar);
  ini_gsl_matrix(dxdt, npar, nmea);
  ini_gsl_matrix(Vdxdt, nmea, npar);

  ini_gsl_vector(dxi, nunm);
  ini_gsl_vector(Fxidxi, ncon);
  ini_gsl_vector(lambda, ncon);
  ini_gsl_vector(FetaTlambda, nmea);

  ini_gsl_vector(etaxi, npar);
  ini_gsl_vector(etasv, npar);
  ini_gsl_vector(y, nmea);
  ini_gsl_vector(y_eta, nmea);
  ini_gsl_vector(Vinvy_eta, nmea);

  ini_gsl_matrix(FetaV, ncon, nmea);

  ini_gsl_permutation(permS, ncon);
  ini_gsl_permutation(permU, nunm);
  ini_gsl_permutation(permV, nmea);

  assert(f && (int)f->size == ncon);
  assert(r && (int)r->size == ncon);
  assert(Fetaxi && (int)Fetaxi->size1 == ncon && (int)Fetaxi->size2 == npar);
  assert(S && (int)S->size1 == ncon && (int)S->size2 == ncon);
  assert(Sinv && (int)Sinv->size1 == ncon && (int)Sinv->size2 == ncon);
  assert(nunm == 0 || (SinvFxi && (int)SinvFxi->size1 == ncon && (int)SinvFxi->size2 == nunm));
  assert(SinvFeta && (int)SinvFeta->size1 == ncon && (int)SinvFeta->size2 == nmea);
  assert(nunm == 0 || (W1 && (int)W1->size1 == nunm && (int)W1->size2 == nunm));
  assert(G && (int)G->size1 == nmea && (int)G->size2 == nmea);
  assert(nunm == 0 || (H && (int)H->size1 == nmea && (int)H->size2 == nunm));
  assert(nunm == 0 || (HU && (int)HU->size1 == nmea && (int)HU->size2 == nunm));
  assert(IGV && (int)IGV->size1 == nmea && (int)IGV->size2 == nmea);
  assert(V && (int)V->size1 == npar && (int)V->size2 == npar);
  assert(VLU && (int)VLU->size1 == nmea && (int)VLU->size2 == nmea);
  assert(Vinv && (int)Vinv->size1 == nmea && (int)Vinv->size2 == nmea);
  assert(Vnew && (int)Vnew->size1 == npar && (int)Vnew->size2 == npar);
  assert(nunm == 0 || (dxi && (int)dxi->size == nunm));
  assert(nunm == 0 || (Fxidxi && (int)Fxidxi->size == ncon));
  assert(lambda && (int)lambda->size == ncon);
  assert(FetaTlambda && (int)FetaTlambda->size == nmea);
  assert(etaxi && (int)etaxi->size == npar);
  assert(etasv && (int)etasv->size == npar);
  assert(y && (int)y->size == nmea);
  assert(y_eta && (int)y_eta->size == nmea);
  assert(Vinvy_eta && (int)Vinvy_eta->size == nmea);
  assert(FetaV && (int)FetaV->size1 == ncon && (int)FetaV->size2 == nmea);
  assert(permS && (int)permS->size == ncon);
  assert(permS && (int)permS->size == ncon);
  assert(nunm == 0 || (permU && (int)permU->size == nunm));
  assert(permV && (int)permV->size == nmea);

  return true;

}

bool OPALFitterGSL::updateFitObjects(double eetaxi[])
{
  // changed etaxi -> eetaxi to avoid clash with class member DJeans
  //bool debug = false;
  bool result = true;
  for (unsigned int ifitobj = 0; ifitobj < fitobjects.size(); ++ifitobj) {
    for (int ilocal = 0; ilocal < fitobjects[ifitobj]->getNPar(); ++ilocal) {
      fitobjects[ifitobj]->updateParams(eetaxi, npar);
//       int iglobal = fitobjects[ifitobj]->getGlobalParNum (ilocal);
//       if (!fitobjects[ifitobj]->isParamFixed (ilocal) && iglobal >= 0) {
//         B2DEBUG(0,"Parameter " << iglobal
//                         << " (" << fitobjects[ifitobj]->getName()
//                         << ": " << fitobjects[ifitobj]->getParamName(ilocal)
//                         << ") set to " << etaxi[iglobal]);
//         result &= fitobjects[ifitobj]->setParam(ilocal, etaxi[iglobal]);
//         etaxi[iglobal] = fitobjects[ifitobj]->getParam(ilocal);
//         B2DEBUG(0, " => " << etaxi[iglobal] );
//       }
    }
  }
  return result;
}

int OPALFitterGSL::getError() const {return ierr;}
double OPALFitterGSL::getProbability() const {return fitprob;}
double OPALFitterGSL::getChi2() const {return chi2;}
int OPALFitterGSL::getDoF() const {return ncon - nunm;}
int OPALFitterGSL::getIterations() const {return nit;}

void OPALFitterGSL::ini_gsl_permutation(gsl_permutation*& p, unsigned int size)
{
  if (p) {
    if (p->size != size) {
      gsl_permutation_free(p);
      if (size > 0) p = gsl_permutation_alloc(size);
      else p = 0;
    }
  } else if (size > 0) p = gsl_permutation_alloc(size);
}

void OPALFitterGSL::ini_gsl_vector(gsl_vector*& v, unsigned int size)
{

  if (v) {
    if (v->size != size) {
      gsl_vector_free(v);
      if (size > 0) v = gsl_vector_alloc(size);
      else v = 0;
    }
  } else if (size > 0) v = gsl_vector_alloc(size);
}

void OPALFitterGSL::ini_gsl_matrix(gsl_matrix*& m, unsigned int size1, unsigned int size2)
{
  if (m) {
    if (m->size1 != size1 || m->size2 != size2) {
      gsl_matrix_free(m);
      if (size1 * size2 > 0) m = gsl_matrix_alloc(size1, size2);
      else m = 0;
    }
  } else if (size1 * size2 > 0) m = gsl_matrix_alloc(size1, size2);
}

void OPALFitterGSL::setDebug(int debuglevel)
{
  debug = debuglevel;
}

void OPALFitterGSL::debug_print(gsl_matrix* m, const char* name)
{
  for (unsigned int  i = 0; i < m->size1; ++i)
    for (unsigned int j = 0; j < m->size2; ++j)
      if (gsl_matrix_get(m, i, j) != 0)
        cout << name << "[" << i << "][" << j << "]=" << gsl_matrix_get(m, i, j) << endl;
}

void OPALFitterGSL::debug_print(gsl_vector* v, const char* name)
{
  for (unsigned int  i = 0; i < v->size; ++i)
    if (gsl_vector_get(v, i) != 0)
      cout << name << "[" << i << "]=" << gsl_vector_get(v, i) << endl;
}

int OPALFitterGSL::getNcon() const {return ncon;}
int OPALFitterGSL::getNsoft() const {return 0;}
int OPALFitterGSL::getNunm() const {return nunm;}
int OPALFitterGSL::getNpar() const {return npar;}
