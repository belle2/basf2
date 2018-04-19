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

#ifndef __PARTICLEFITOBJECT_H
#define __PARTICLEFITOBJECT_H

#include "analysis/OrcaKinFit/BaseFitObject.h"
#include "analysis/OrcaKinFit/FourVector.h"


// Class ParticleFitObject
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
 * In its current state, a ParticleFitObject has a set of parameters, some
 * of them measured (i.e., they contribute to the \f$\chi^2\f$).
 * These parameters have a local numbering, running from 0 to n-1.
 * Global numbers can be assigned by the BaseFitter using
 * setGlobalParNum.
 *
 * Author: Benno List, Jenny List
 * $Date: 2009/09/02 13:10:57 $
 * $Author: blist $
 *
 * \b Changelog:
 * - 30.12.04 BL: Added getCov, setCov
 *                addToGlobCov, getDChi2DParam, getDChi2DParam2, addToGlobalDerMatrix implemented
 */

namespace Belle2 {

  namespace OrcaKinFit {


    class ParticleFitObject: public BaseFitObject {
    public:
      /// Default constructor
      ParticleFitObject();


      /// Copy constructor
      ParticleFitObject(const ParticleFitObject& rhs               ///< right hand side
                       );

      /// Assignment
      ParticleFitObject& operator= (const ParticleFitObject& rhs   ///< right hand side
                                   );

      /// Virtual destructor
      virtual ~ParticleFitObject();

      /// Assign from anther object, if of same type
      virtual ParticleFitObject& assign(const BaseFitObject& source    ///< The source object
                                       );

      /// Set mass of particle; return=success
      virtual bool setMass(double mass_);
      /// Get mass of particle
      virtual double getMass() const;

      /// print the four-momentum (E, px, py, pz)
      virtual std::ostream& print4Vector(std::ostream& os    ///< The output stream
                                        ) const;

      virtual FourVector getFourMomentum() const;

      /// Return E
      virtual double getE() const;
      /// Return px
      virtual double getPx() const;
      /// Return py
      virtual double getPy() const;
      /// Return pz
      virtual double getPz() const;

      /// Return p (momentum)
      virtual double getP() const;
      /// Return p (momentum) squared
      virtual double getP2() const;
      /// Return pt (transverse momentum)
      virtual double getPt() const;
      /// Return pt (transverse momentum)  squared
      virtual double getPt2() const;

      /// Return d p_x / d par_ilocal (derivative of px w.r.t. local parameter ilocal)
      virtual double getDPx(int ilocal               ///< Local parameter number
                           ) const = 0;
      /// Return d p_y / d par_ilocal (derivative of py w.r.t. local parameter ilocal)
      virtual double getDPy(int ilocal               ///< Local parameter number
                           ) const = 0;
      /// Return d p_z / d par_ilocal (derivative of pz w.r.t. local parameter ilocal)
      virtual double getDPz(int ilocal               ///< Local parameter number
                           ) const = 0;
      /// Return d E / d par_ilocal (derivative of E w.r.t. local parameter ilocal)
      virtual double getDE(int ilocal                ///< Local parameter number
                          ) const = 0;

      virtual void getDerivatives(double der[], int idim) const;

      /// Add  numerically determined derivatives of chi squared to global covariance matrix
      virtual void addToGlobalChi2DerMatrixNum(double* M,    ///< Global covariance matrix
                                               int idim,    ///< First dimension of global covariance matrix
                                               double eps   ///< Parameter variation
                                              );

      /// Add numerically determined derivatives of chi squared to global derivative vector
      virtual void addToGlobalChi2DerVectorNum(double* y,     ///< Vector of chi2 derivatives
                                               int idim,     ///< Vector size
                                               double eps    ///< Parameter variation
                                              );

      /// print object to ostream
      virtual std::ostream& print(std::ostream& os     ///< The output stream
                                 ) const;

      void test1stDerivatives();
      void test2ndDerivatives();

      /// Evaluates numerically the 1st derivative  of chi2 w.r.t. a parameter
      double num1stDerivative(int ilocal,   ///< Local parameter number
                              double eps   ///< variation of  local parameter
                             );

      /// Evaluates numerically the 2nd derivative of chi2 w.r.t. 2 parameters
      double num2ndDerivative(int ilocal1,  ///< 1st local parameter number
                              double eps1, ///< variation of 1st local parameter
                              int ilocal2, ///< 1st local parameter number
                              double eps2  ///< variation of 2nd local parameter
                             );

      virtual double getChi2() const;

    protected:
      /// mass of particle
      double mass;

      mutable FourVector fourMomentum;

      // this is to flag phi angle, for example
      double paramCycl[BaseDefs::MAXPAR];

    };

  }// end OrcaKinFit namespace
} // end Belle2 namespace

#endif // __PARTICLEFITOBJECT_H

