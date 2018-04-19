/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Used with permission from ILC                                          *
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

#ifndef __OPALFITTERGSL_H
#define __OPALFITTERGSL_H

#include<vector>
#include "analysis/OrcaKinFit/BaseFitter.h"

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_permutation.h>

namespace Belle2 {

  namespace OrcaKinFit {

    class BaseFitObject;
    class BaseConstraint;
    /** Description of the fit algorithm and interface:
     *
     * The OPALFitter object holds a set (fitobjects) of BaseFitObject
     * objects,
     * which represent the objects (particles, jets) whose fourvectors
     * shall be fitted.
     * It also holds a set (constraints) of BaseConstraint objects
     * that represent the constraints (momentum or mass constraints).
     *
     * OPALFitter::initialize goes over the list of fit objects and
     * determines the number of parameters (measured and unmeasured),
     * and assigns global numbers to them.
     *
     * OPALFitter::fit first initializes the global parameter numbering
     * using OPALFitter::initialize.
     *
     * Then it initializes vectors etaxi (\f$ \eta\xi\f$)  and y (\f$ \vec y\f$)
     * with the current  parameter values.
     *
     * Next it initializes the matrix Feta that represents
     * \f$d \vec F / d \vec \eta\xi\f$
     *
     * Used methods in OPALFitter::initialize:
     * - BaseFitObject::getNPar
     * - BaseFitObject::getMeasured
     * - BaseFitObject::setGlobalParNum
     *
     * Used methods in OPALFitter::updateFitObjects:
     * - BaseFitObject::getGlobalParNum
     * - BaseFitObject::setParam
     *
     * Used methods in OPALFitter::fit:
     * - BaseFitObject::getNPar
     * - BaseFitObject::getGlobalParNum
     * - BaseFitObject::getParam
     * - BaseFitObject::addToGlobCov
     * - BaseFitObject::operator<<
     * - BaseFitObject::setError
     * - BaseConstraint::getDerivatives
     * - BaseConstraint::getValue
     *
     * Interaction between Constraints and Particles
     *
     * The Fitter does not care how the constraints and BaseFitObject objects
     * interact. The constraints are expressed in terms of four-vector
     * components of the BaseFitObjects.
     * A constraint object keeps a set of BaseFitObject objects, and, using
     * the chain rule, calculates its derivatives w.r.t. the global parameters.
     *
     */

    class OPALFitterGSL : public BaseFitter {
    public:
      OPALFitterGSL();
      virtual ~OPALFitterGSL();
      virtual double fit();

      /// Return error code
      /** Error code meanings:
       *  - 0: No error
       *  - 1: out of iterations
       *  - 2: crazy chi^2
       *  - 3: minimum step size reached, chiK still increasing
       *  - 4: (step size decreased)
       *  - 5: (keep going)
       */
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

      virtual bool initialize();

      /// Set the Debug Level
      virtual void setDebug(int debuglevel);

    protected:


      virtual bool updateFitObjects(double etaxi[]);
      enum {NPARMAX = 50, NCONMAX = 20, NUNMMAX = 20};

      int npar;      // total number of parameters
      int nmea;      // total number of measured parameters
      int nunm;      // total number of unmeasured parameters
      int ncon;      // total number of constraints
      int ierr;      // Error status
      int nit;       // Number of iterations

      double fitprob;   // fit probability
      double chi2;      // final chi2

      static void ini_gsl_permutation(gsl_permutation*& p, unsigned int size);
      static void ini_gsl_vector(gsl_vector*& v, int unsigned size);
      static void ini_gsl_matrix(gsl_matrix*& m, int unsigned size1, unsigned int size2);

      static void debug_print(gsl_matrix* m, const char* name);
      static void debug_print(gsl_vector* v, const char* name);

    private:
      gsl_vector* f;
      gsl_vector* r;
      gsl_matrix* Fetaxi;
      gsl_matrix* S;
      gsl_matrix* Sinv;
      gsl_matrix* SinvFxi;
      gsl_matrix* SinvFeta;
      gsl_matrix* W1;
      gsl_matrix* G;
      gsl_matrix* H;
      gsl_matrix* HU;
      gsl_matrix* IGV;
      gsl_matrix* V;
      gsl_matrix* VLU;
      gsl_matrix* Vinv;
      gsl_matrix* Vnew;
      gsl_matrix* Minv;
      gsl_matrix* dxdt;
      gsl_matrix* Vdxdt;
      gsl_vector* dxi;
      gsl_vector* Fxidxi;
      gsl_vector* lambda;
      gsl_vector* FetaTlambda;
      gsl_vector* etaxi;
      gsl_vector* etasv;
      gsl_vector* y;
      gsl_vector* y_eta;
      gsl_vector* Vinvy_eta;
      //    gsl_matrix *Feta; //<-- DJeans: not used/needed
      gsl_matrix* FetaV;

      gsl_permutation* permS; ///< Helper permutation vector
      gsl_permutation* permU; ///< Helper permutation vector
      gsl_permutation* permV; ///< Helper permutation vector

      int debug;

    };

  }// end OrcaKinFit namespace
} // end Belle2 namespace

#endif // __OPALFITTERGSL_H
