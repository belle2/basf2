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

#ifndef __NEWFITTERGSL_H
#define __NEWFITTERGSL_H

#include "analysis/OrcaKinFit/BaseFitter.h"

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_permutation.h>
#include <gsl/gsl_eigen.h>

namespace Belle2 {

  namespace OrcaKinFit {

// Class NewFitterGSL
/// A kinematic fitter using the Newton-Raphson method to solve the equations
    /**
     * This class implements a kinematic fitter using the Newton-Raphson method
     * to solve the system of equations arising from the Lagrange multiplier
     * method
     *
     * Author: Benno List
     * Last update: $Date: 2011/05/03 13:16:41 $
     *          by: $Author: blist $
     *
     * \b Changelog:
     * - 15.11.2010 First version
     *
     */

    class NewFitterGSL : public BaseFitter {
    public:
      /// Constructor
      NewFitterGSL();
      /// Virtual destructor
      virtual ~NewFitterGSL();

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

      /// Determine best lambda values
      virtual int determineLambdas(gsl_vector* vecxnew,         ///< vector with new lambda values
                                   const gsl_matrix* MatM,     ///< matrix with constraint derivatives
                                   const gsl_vector* vecx,     ///< vector with current x values
                                   gsl_matrix* MatW,           ///< work matrix
                                   gsl_vector* vecw,           ///< work vector
                                   double eps = 0              ///< Singular values < eps*(max(abs(s_i))) are set to 0
                                  );

      /// Calculate 2nd order correction step
      virtual void calc2ndOrderCorr(gsl_vector* vecdxhat,        ///< the correction step
                                    const gsl_vector* vecxnew,  ///< the current state vector (parameters must be set to vecxnew!)
                                    const gsl_matrix* MatM,     ///< The matrix of the last Newton step
                                    gsl_matrix* MatW,     ///< work matrix
                                    gsl_vector* vecw,     ///< work vector
                                    double eps = 0        ///< Singular values < eps*(max(abs(s_i))) are set to 0
                                   );

      /// Calculate null space of constraints; return value is a matrix view of MatW1, with column vectors spanning null(A)
      virtual gsl_matrix_view calcZ(int& rankA,               ///< rank of A (= number of lin. indep. constraints)
                                    gsl_matrix* MatW1,       ///< work matrix, contains Z at the end
                                    gsl_matrix* MatW2,       ///< work matrix
                                    gsl_vector* vecw1,       ///< work vector
                                    gsl_vector* vecw2,       ///< work vector
                                    gsl_permutation* permW,  ///< Work permutation vector
                                    double eps = 0           ///< Singular values < eps*(max(abs(s_i))) are set to 0
                                   );

      /// Calculate reduced Hessian; return value is a matrix view of MatW1 giving the reduced Hessian
      virtual gsl_matrix_view calcReducedHessian(int& rankH,                     ///< dimension of H
                                                 gsl_matrix* MatW1,       ///< work matrix, contains H at the end
                                                 const gsl_vector* vecx,        ///< vector with current x values
                                                 gsl_matrix* MatW2,       ///< work matrix, contains Z at the end
                                                 gsl_matrix* MatW3,       ///< work matrix
                                                 gsl_vector* vecw1,       ///< work vector
                                                 gsl_vector* vecw2,       ///< work vector
                                                 gsl_permutation* permW,  ///< Work permutation vector
                                                 double eps = 0           ///< Singular values < eps*(max(abs(s_i))) are set to 0
                                                );

      virtual gsl_vector_view calcReducedHessianEigenvalues(int& rankH,               ///< dimension of H
                                                            gsl_matrix* MatW1,       ///< work matrix, contains H at the end
                                                            const gsl_vector* vecx,        ///< vector with current x values
                                                            gsl_matrix* MatW2,       ///< work matrix, contains Z at the end
                                                            gsl_matrix* MatW3,       ///< work matrix
                                                            gsl_vector* vecw1,       ///< work vector
                                                            gsl_vector* vecw2,       ///< work vector
                                                            gsl_permutation* permW,  ///< Work permutation vector
                                                            gsl_eigen_symm_workspace* eigenws,  ///< Work space for eigenvalue calculation
                                                            double eps = 0           ///< Singular values < eps*(max(abs(s_i))) are set to 0
                                                           );
    public:
      /// Calculate the chi2
      virtual double calcChi2();

      void fillx(gsl_vector* vecx);
      void fillperr(gsl_vector* vece);

      // Fill matrix MatM, using lambdas from vecx
      void assembleM(gsl_matrix* MatM, const gsl_vector* vecx, bool errorpropagation = false);

      // Fill matrix MatM with 2nd derivative of Lagrangian, using lambdas from vecx
      void assembleG(gsl_matrix* MatM, const gsl_vector* vecx);

      // Scale matrix MatM to MatMscal using errors from vece
      void scaleM(gsl_matrix* MatMscal, const gsl_matrix* MatM, const gsl_vector* vece);

      // Fill vector y, using lambdas from vecx
      void assembley(gsl_vector* vecy, const gsl_vector* vecx);

      // Scale vector vecy to vecyscal using errors from vece
      void scaley(gsl_vector* vecyscal, const gsl_vector* vecy, const gsl_vector* vece);

      // Fill chi2 derivatives into vector y
      int assembleChi2Der(gsl_vector* vecy);

      // Fill vector y with values of constraints
      void addConstraints(gsl_vector* vecy);

      // Fill constraint derivatives into Matrix M
      void assembleConstDer(gsl_matrix* MatM);

      // Calculate Newton step update vector vecdx from current point vecx and errors vece
      int calcNewtonDx(gsl_vector* vecdx,              ///< Result: Update vector dx
                       gsl_vector* vecdxscal,   ///< Result: Update vector dx, scaled
                       gsl_vector* vecx,        ///< Current vector x
                       const gsl_vector* vece,        ///< Current ``error'' set of x
                       gsl_matrix* MatM,        ///< Matrix M
                       gsl_matrix* MatMscal,    ///< Matrix M, scaled
                       gsl_vector* vecy,        ///< Vector y
                       gsl_vector* vecyscal,    ///< Vector y, scaled,
                       gsl_matrix* MatW,        ///< Work matrix
                       gsl_matrix* MatW2,       ///< Work matrix
                       gsl_permutation* permW,  ///< Work permutation vector
                       gsl_vector* vecw         ///< Work vector
                      );

      // Calculate limited step after linesearch
      int calcLimitedDx(double& alpha,                  ///< Output value alpha
                        double& mu,              ///< Value of mu for merit function
                        gsl_vector* vecxnew,     ///< New vector x
                        int imode,               ///< mode: 0=Armijo, 1=Wolfe, 2=Goldstein
                        gsl_vector* vecx,        ///< Current vector x
                        gsl_vector* vecdxhat,    ///< the 2nd order correction step, if any
                        const gsl_vector* vecdx,       ///< Update vector dx
                        const gsl_vector* vecdxscal,   ///< Result: Update vector dx, scaled
                        const gsl_vector* vece,        ///< Error vector e
                        const gsl_matrix* MatM,        ///< Matrix M
                        const gsl_matrix* MatMscal,    ///< Matrix M, scaled
                        gsl_matrix* MatW,        ///< Work matrix
                        gsl_vector* vecw         ///< Work vector w1
                       );

      // Perform a line search
      int doLineSearch(double& alpha,                  ///< Output value alpha
                       gsl_vector* vecxnew,     ///< New vector x
                       int imode,               ///< mode: 0=Armijo, 1=Wolfe, 2=Goldstein
                       double phi0,             ///< Merit function for alpha=0
                       double dphi0,            ///< Directional derivative of merit function for alpha=0
//                   double phiR,             ///< Merit function for given alpha
                       double eta,              ///< Constant for Armijo's rule
                       double zeta,             ///< Constant for Wolfe's or Goldstein's rule
                       double mu,               ///< Value of mu for merit function
                       const gsl_vector* vecx,        ///< Current vector x
                       const gsl_vector* vecdx,       ///< Update vector dx
                       const gsl_vector* vece,        ///< Error vector e
                       gsl_vector* vecw         ///< Work vector w
                      );

      // Calculate mu for the merit function
      double calcMu(const gsl_vector* vecx,            ///< Current vector x
                    const gsl_vector* vece,           ///< Current errors x
                    const gsl_vector* vecdx,          ///< Current step dx
                    const gsl_vector* vecdxscal,      ///< Current step dx, scaled
                    const gsl_vector* xnew,           ///< New vector x
                    const gsl_matrix* MatM,           ///< Current matrix M
                    const gsl_matrix* MatMscal,       ///< Current scaled matrix M
                    gsl_vector* vecw            ///< Work vector w
                   );

      // Calculate the merit function at x
      double meritFunction(double mu,                  ///< Value of mu
                           const gsl_vector* vecx,    ///< Current vector x
                           const gsl_vector* vece     ///< Current errors x
                          );
      // Calculate the directional derivative of the merit function at x
      double meritFunctionDeriv(double mu,                  ///< Value of mu
                                const gsl_vector* vecx,    ///< Current vector x
                                const gsl_vector* vece,    ///< Current errors x
                                const gsl_vector* vecdx,   ///< Current update vector dx
                                gsl_vector* vecw     ///< work vector
                               );

      // Transfer values from vecx to FitObjects
      bool updateParams(gsl_vector* vecx);


      int invertM();

      int calcCovMatrix(gsl_matrix* MatW, gsl_permutation* permW, gsl_vector* vecx);

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

      static void debug_print(const gsl_matrix* m, const char* name);
      static void debug_print(const gsl_vector* v, const char* name);

      // z = x + a y
      static void add(gsl_vector* vecz, const gsl_vector* vecx, double a, const gsl_vector* vecy);

      // Check whether all elements are finite
      static bool isfinite(const gsl_vector* vec);

      // Check whether all elements are finite
      static bool isfinite(const gsl_matrix* mat);

      /// Compute the Moore-Penrose pseudo-inverse A+ of A, using SVD
      static void MoorePenroseInverse(gsl_matrix* Ainv,      ///< Result: m x n matrix A+
                                      gsl_matrix* A,        ///< Input: n x m matrix A, n >= m (is destroyed!)
                                      gsl_matrix* W,        ///< Work matrix, at least m x m
                                      gsl_vector* w,        ///< Work vector w, at least m
                                      double eps = 0        ///< Singular values < eps*(max(abs(s_i))) are set to 0
                                     );
      double calcpTLp(const gsl_vector* vecdx,           ///< Current step dx
                      const gsl_matrix* MatM,           ///< Current matrix M
                      gsl_vector* vecw            ///< Work vector w
                     );

      /// solve system of equations Mscal*dxscal = yscal
      int solveSystem(gsl_vector* vecdxscal,
                      double& detW,
                      const gsl_vector* vecyscal,
                      const gsl_matrix* MatMscal,
                      gsl_matrix* MatW,
                      gsl_matrix* MatW2,
                      gsl_vector* vecw,
                      double epsLU,
                      double epsSV
                     );

      /// solve system of equations Mscal*dxscal = yscal using LU decomposition
      int solveSystemLU(gsl_vector* vecdxscal,
                        double&     detW,
                        const gsl_vector* vecyscal,
                        const gsl_matrix* MatMscal,
                        gsl_matrix* MatW,
                        gsl_vector* vecw,
                        double eps
                       );

      /// solve system of equations Mscal*dxscal = yscal using SVD decomposition
      int solveSystemSVD(gsl_vector* vecdxscal,
//                     double& detW,
                         const gsl_vector* vecyscal,
                         const gsl_matrix* MatMscal,
                         gsl_matrix* MatW,
                         gsl_matrix* MatW2,
                         gsl_vector* vecw,
                         double eps
                        );

    public:
      unsigned int idim;
      gsl_vector* x;
      gsl_vector* xold;
      gsl_vector* xnew;
//     gsl_vector *xbest;
      gsl_vector* dx;
      gsl_vector* dxscal;
//     gsl_vector *grad;
      gsl_vector* y;
      gsl_vector* yscal;
      gsl_vector* perr;
      gsl_vector* v1;
      gsl_vector* v2;
//     gsl_vector *Meval;

      gsl_matrix* M;
      gsl_matrix* Mscal;
      gsl_matrix* W;
      gsl_matrix* W2;
      gsl_matrix* W3;
      // these are only used locally in calcCovMatrix
      gsl_matrix* M1;
      gsl_matrix* M2;
      gsl_matrix* M3;
      gsl_matrix* M4;
      gsl_matrix* M5;
//     gsl_matrix *Mevec;
      gsl_matrix* CC;
      gsl_matrix* CC1;
      gsl_matrix* CCinv;

      gsl_permutation* permW;
      gsl_eigen_symm_workspace* eigenws;
//    gsl_eigen_symmv_workspace *eigenwsv;
      unsigned int eigenwsdim;

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
      bool try2ndOrderCorr;

      int debug;
    };

  }// end OrcaKinFit namespace
} // end Belle2 namespace

#endif // __NEWFITTERGSL_H
