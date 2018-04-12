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

#ifndef __NEWTONFITTERGSL_H
#define __NEWTONFITTERGSL_H

#include "analysis/OrcaKinFit/BaseFitter.h"

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_permutation.h>
#include <gsl/gsl_eigen.h>

// Class NewtonFitterGSL
/// A kinematic fitter using the Newton-Raphson method to solve the equations

class NewtonFitterGSL : public BaseFitter {
public:
  /// Constructor
  NewtonFitterGSL();
  /// Virtual destructor
  virtual ~NewtonFitterGSL();

  /// The fit method, returns  the fit probability
  virtual double fit();

  /// Get the error code of the last fit: 0=OK, 1=failed
  virtual int getError() const;

  /// Get the fit probability of the last fit
  virtual double getProbability() const;
  /// Get the chi**2 of the last fit
  virtual double getChi2() const;
  /// Get the number of degrees of freedom of the last fit
  virtual int    getDoF() const;
  /// Get the number of iterations of the last fit
  virtual int  getIterations() const;

  /// Get the number of hard constraints of the last fit
  virtual int    getNcon() const;

  /// Get the number of soft constraints of the last fit
  virtual int    getNsoft() const;

  /// Get the number of all parameters of the last fit
  virtual int    getNpar() const;

  /// Get the number of unmeasured parameters of the last fit
  virtual int    getNunm() const;

  /// Initialize the fitter
  virtual bool initialize();

  /// Set the Debug Level
  virtual void setDebug(int debuglevel);

protected:
  /// Calculate the chi2
  virtual double calcChi2();

  /// Calculate the vector dx to update the parameters; returns fail code, 0=OK
  int calcDx();

  /// Calculate the vector dx to update the parameters; returns fail code, 0=OK
  int calcDxSVD();

  /// Print a Matrix M and a vector y of dimension idim
  void printMy(double M[], double y[], int idim);

  bool updateParams(gsl_vector* xnew);
  void fillxold();
  void fillperr();
  int calcM(bool errorpropagation = false);
  int calcy();
  int optimizeScale();
  int invertM();

  void calcCovMatrix();

  double meritFunction(double mu);
  double meritFunctionDeriv();

  enum {NPARMAX = 50, NCONMAX = 10, NUNMMAX = 10};

  int npar;      ///< total number of parameters
  int ncon;      ///< total number of hard constraints
  int nsoft;     ///< total number of soft constraints
  int nunm;      ///< total number of unmeasured parameters
  int ierr;      ///< Error status
  int nit;       ///< Number of iterations

  double fitprob;   ///< fit probability
  double chi2;      ///< final chi2

  static void ini_gsl_permutation(gsl_permutation*& p, unsigned int size);
  static void ini_gsl_vector(gsl_vector*& v, int unsigned size);
  static void ini_gsl_matrix(gsl_matrix*& m, int unsigned size1, unsigned int size2);

  static void debug_print(gsl_matrix* m, const char* name);
  static void debug_print(gsl_vector* v, const char* name);

private:
  unsigned int idim;
  gsl_vector* x;
  gsl_vector* xold;
  gsl_vector* xbest;
  gsl_vector* dx;
  gsl_vector* dxscal;
  gsl_vector* grad;
  gsl_vector* y;
  gsl_vector* yscal;
  gsl_vector* perr;
  gsl_vector* v1;
  gsl_vector* v2;
  gsl_vector* Meval;

  gsl_matrix* M;
  gsl_matrix* Mscal;
  gsl_matrix* M1;
  gsl_matrix* M2;
  gsl_matrix* M3;
  gsl_matrix* M4;
  gsl_matrix* M5;
  gsl_matrix* Mevec;
  gsl_matrix* CC;
  gsl_matrix* CC1;
  gsl_matrix* CCinv;

  gsl_permutation* permM;
  gsl_eigen_symmv_workspace* ws;
  unsigned int wsdim;

  double chi2best;
  double chi2new;
  double chi2old;
  double fvalbest;
  double scale;
  double scalebest;
  double stepsize;
  double stepbest;
  enum {NITMAX = 100};
  double scalevals[NITMAX];
  double fvals[NITMAX];

  int imerit;

  int debug;
};

#endif // __NEWTONFITTERGSL_H
