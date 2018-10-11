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

#ifndef __BASEFITOBJECT_H
#define __BASEFITOBJECT_H

#include <iostream>

#include "analysis/OrcaKinFit/BaseDefs.h"

// Class BaseFitObject
/// Abstract base class for particle objects of kinematic fits
/**
 * This class defines the minimal functionality any fit object must provide.
 * The main task of a fit object is to keep parameters (and errors) that define
 * the four-momentum of a particle and encapsulate the actually chosen
 * parametrisation from the rest of the fitting machinery.
 *
 * Since for the fit a parametrisation distributed like a gaussian is most
 * favorable, different kinds of particles (implying different kinds of
 * measurements!) might require different parametrisations. For each
 * desired parametrisation a concrete class should be derived from this
 * abstract base class. It needs to be able to convert its parameters
 * to E, px, py, pz and to provide the derivatives of E, px, py, pz
 * with respect to the internal parameters.
 *
 * Depending on the type of particle, some or all parameters might
 * be unmeasured (neutrinos!), meaning that they come with a very large
 * and/or unknown error. They are treated differently by the
 * fit algorithm and are thus flagged accordingly.
 *
 * In order to insert its derivatives into the global covariance matrix
 * of all FitObjects in the event, each FitObjects needs to know the
 * position of its parameters in the overall parameter list.
 *
 * THIS iS JUNK!!!! It is done like this in JetFitObject.C,
 * but using measured[i] which is the bool giving the measured/unmeasured
 * status and NOT a bool containing the START VALUES!!!!!
 * From its stored initial parameters and the current fit parameters
 * the FitObject calculates its contribution to the $\chi^2$ of the fit.
 *
 * In its current state, a BaseFitObject has a set of parameters, some
 * of them measured (i.e., they contribute to the \f$\chi^2\f$).
 * These parameters have a local numbering, running from 0 to n-1.
 * Global numbers can be assigned by the BaseFitter using
 * setGlobalParNum.
 *
 * The class WWFitter needs the following routines from BaseFitObject:
 * - BaseFitObject::getNPar
 * - BaseFitObject::getMeasured
 * - BaseFitObject::getGlobalParNum
 * - BaseFitObject::getParam
 * - BaseFitObject::setParam
 * - BaseFitObject::addToGlobCov
 * - BaseFitObject::operator<<
 * - BaseFitObject::setError
 *
 *
 *
 *
 * Author:  Benno List, Jenny Böhme
 * $Date: 2011/03/03 15:03:02 $
 * $Author: blist $
 *
 * \b Changelog:
 * - 30.12.04 BL: Added getCov, setCov
 * - 11.1.05 BL: return type of setParam changed to bool
 */

namespace Belle2 {

  namespace OrcaKinFit {

    class BaseFitObject {
    public:
      /// Default constructor
      BaseFitObject();

      /// Copy constructor
      BaseFitObject(const BaseFitObject& rhs               ///< right hand side
                   );
      /// Assignment
      BaseFitObject& operator= (const BaseFitObject& rhs   ///< right hand side
                               ) ;

      /// Virtual destructor
      virtual ~BaseFitObject();

      /// Return a new copy of itself
      virtual BaseFitObject* copy() const = 0;

      /// Assign from anther object, if of same type
      virtual BaseFitObject& assign(const BaseFitObject& source    ///< The source object
                                   );

      /// Set value and measured flag of parameter i; return: significant change
      virtual bool setParam(int ilocal,          ///< Local parameter number
                            double par_,        ///< New parameter value
                            bool measured_,     ///< New "measured" flag
                            bool fixed_ = false ///< New "fixed" flag
                           );

      /// Set value of parameter ilocal; return: significant change
      virtual bool setParam(int ilocal,     ///< Local parameter number
                            double par_    ///< New parameter value
                           );

      /// Read values from global vector, readjust vector; return: significant change
      virtual bool updateParams(double p[],    ///< The parameter vector
                                int idim      ///< Length of the vector
                               );

      /// Set measured value of parameter ilocal; return: success
      virtual bool setMParam(int ilocal,     ///< Local parameter number
                             double mpar_   ///< New measured parameter value
                            );

      /// Set error of parameter ilocal; return: success
      virtual bool setError(int ilocal,     ///< Local parameter number
                            double err_    ///< New error value
                           );

      /// Set covariance of parameters ilocal and jlocal; return: success
      virtual bool setCov(int ilocal,     ///< Local parameter number
                          int jlocal,    ///< Local parameter number
                          double cov_    ///< New error value
                         );

      /// Set number of parameter ilocal in global list
      /// return true signals OK
      virtual bool setGlobalParNum(int ilocal,   ///< Local parameter number
                                   int iglobal  ///< New global parameter number
                                  );

      /// Fix a parameter (fix=true), or release it (fix=false)
      virtual bool fixParam(int ilocal,     ///< Local parameter number
                            bool fix = true ///< fix if true, release if false
                           );
      /// Release a parameter
      virtual bool releaseParam(int ilocal     ///< Local parameter number
                               )
      { return fixParam(ilocal, false); }

      /// Returns whether parameter is fixed
      virtual bool isParamFixed(int ilocal      ///< Local parameter number
                               ) const;

      /// Get current value of parameter ilocal
      virtual double getParam(int ilocal      ///< Local parameter number
                             ) const;
      /// Get name of parameter ilocal
      virtual const char* getParamName(int ilocal      ///< Local parameter number
                                      ) const = 0;
      /// Get object's name
      //    virtual const char *getName () const { return name ? name : "???";}
      virtual const char* getName() const;  // { return name ? name : "???";}
      /// Set object's name
      virtual void setName(const char* name_);
      /// Get measured value of parameter ilocal
      virtual double getMParam(int ilocal      ///< Local parameter number
                              ) const;
      /// Get error of parameter ilocal
      virtual double getError(int ilocal      ///< Local parameter number
                             ) const;
      /// Get covariance between parameters ilocal and jlocal
      virtual double getCov(int ilocal,     ///< Local parameter number i
                            int jlocal     ///< Local parameter number j
                           ) const;
      /// Get correlation coefficient between parameters ilocal and jlocal
      virtual double getRho(int ilocal,     ///< Local parameter number i
                            int jlocal     ///< Local parameter number j
                           ) const;
      /// Get measured flag for parameter ilocal
      virtual bool isParamMeasured(int ilocal     ///< Local parameter number
                                  ) const;
      /// Get global parameter number of parameter ilocal
      virtual int getGlobalParNum(int ilocal     ///< Local parameter number
                                 ) const;
      /// Get total number of parameters of this FitObject
      virtual int getNPar() const = 0;
      /// Get number of measured parameters of this FitObject
      virtual int getNMeasured() const;
      /// Get number of unmeasured parameters of this FitObject
      virtual int getNUnmeasured() const;
      /// Get number of free parameters of this FitObject
      virtual int getNFree() const;
      /// Get number of fixed parameters of this FitObject
      virtual int getNFixed() const;

      /// Get chi squared from measured and fitted parameters
      virtual double getChi2() const;

      /// Get derivative of chi squared w.r.t. parameter ilocal
      virtual double getDChi2DParam(int ilocal   ///< Local parameter number
                                   ) const ;

      /// Get second derivative of chi squared w.r.t. parameters ilocal1 and ilocal2
      virtual double getD2Chi2DParam2(int ilocal,   ///< Local parameter number i
                                      int jlocal    ///< Local parameter number j
                                     ) const;

      /// print the parameters and errors
      virtual std::ostream& printParams(std::ostream& os   ///< The output stream
                                       ) const;

      /// print the correlation coefficients
      virtual std::ostream& printRhoValues(std::ostream& os   ///< The output stream
                                          ) const;

      /// print the 1st derivatives wrt metaSet 0 (E, px, py, pz)
      virtual std::ostream& print1stDerivatives(std::ostream& os   ///< The output stream
                                               ) const;

      /// print the 2nd derivatives wrt metaSet 0 (E, px, py, pz)
      virtual std::ostream& print2ndDerivatives(std::ostream& os   ///< The output stream
                                               ) const;

      /// print object to ostream
      virtual std::ostream&  print(std::ostream& os        ///< The output stream
                                  ) const = 0;


      /// invalidate any cached quantities
      virtual void invalidateCache() const {cachevalid = false;};
      virtual void updateCache() const = 0;

      // these are the mothods that fill the fitter's matrices/vectors

      /// Add covariance matrix elements to
      /// global covariance matrix of size idim x idim
      virtual void addToGlobCov(double* glcov,   ///< Global covariance matrix
                                int idim     ///< First dimension of global derivative matrix
                               ) const;

      /// Add derivatives of chi squared to global derivative vector
      virtual int addToGlobalChi2DerVector(double* y,    ///< Vector of chi2 derivatives
                                           int idim     ///< Vector size
                                          ) const;

      /// Add 2nd derivatives of chi squared to global derivative matrix
      virtual void addToGlobalChi2DerMatrix(double* M,    ///< Global derivative matrix
                                            int idim     ///< First dimension of global derivative matrix
                                           ) const;

      /// Add derivatives of momentum vector to global derivative vector
      virtual void addToGlobalChi2DerVector(double* y,      ///< Vector of chi2 derivatives
                                            int idim,      ///< Vector size
                                            double lambda, ///< The lambda value
                                            double der[], ///< derivatives of constraint wrt intermediate variables (e.g. 4-vector with dg/dE, dg/dpx, dg/dpy, dg/dpz)
                                            int metaSet  ///< which set of intermediate variables
                                           ) const;

      // seems not to be used DJeans
      //    virtual void addToDerivatives (double der[], int idim,
      //           double factor[], int metaSet) const;

      virtual void addTo1stDerivatives(double M[],
                                       int idim,
                                       double der[],
                                       int kglobal,
                                       int metaSet
                                      ) const;

      virtual void addTo2ndDerivatives(double der2[], int idim, double factor[], int metaSet) const;
      virtual void addTo2ndDerivatives(double M[], int idim,  double lambda, double der[], int metaSet) const;

      // DANIEL added
      // derivatives of intermediate variables wrt object's local parameters
      // these must be implemented by the derived classes for each type of object
      virtual double getFirstDerivative_Meta_Local(int iMeta ,
                                                   int ilocal ,
                                                   int metaSet
                                                  ) const = 0;

      virtual double getSecondDerivative_Meta_Local(int iMeta,
                                                    int ilocal ,
                                                    int jlocal ,
                                                    int metaSet
                                                   ) const = 0;

      virtual void initCov();

      virtual double getError2(double der[], int metaset) const;

      virtual void getDerivatives(double der[], int idim) const = 0;


    protected:
      char* name;
      const static double eps2;

      // DANIEL moved all of this stuff to BaseFitObject
      // it costs some extra memory, since everything has dimension of largest # of parameters
      // but avoid a lot of almost-duplication in the derived classes

      /// Calculate the inverse of the covariance matrix
      virtual bool calculateCovInv() const;

      /// fit parameters
      double par[BaseDefs::MAXPAR];
      /// measured parameters
      double mpar[BaseDefs::MAXPAR];
      /// measured flag
      bool measured[BaseDefs::MAXPAR];
      /// fixed flag
      bool fixed[BaseDefs::MAXPAR];
      /// global paramter number for each parameter
      int globalParNum [BaseDefs::MAXPAR];
      /// local covariance matrix
      double cov [BaseDefs::MAXPAR][BaseDefs::MAXPAR];
      /// inverse pf local covariance matrix
      mutable double covinv [BaseDefs::MAXPAR][BaseDefs::MAXPAR];
      /// flag for valid inverse covariance matrix
      mutable bool covinvvalid;
      /// flag for valid cache
      mutable bool cachevalid;
      // end DANIEL adds

    };

    /** \relates BaseFitObject
     *  \brief Prints out a BaseFitObject, using its print method
     */
    inline std::ostream& operator<< (std::ostream& os,         ///< The output stream
                                     const BaseFitObject& bfo  ///< The object to print
                                    )
    {
      return bfo.print(os);
    }

  }// end OrcaKinFit namespace
} // end Belle2 namespace


#endif // __BASEFITOBJECT_H

