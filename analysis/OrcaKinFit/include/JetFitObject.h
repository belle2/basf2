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

#ifndef __JETFITOBJECT_H
#define __JETFITOBJECT_H

#include "analysis/OrcaKinFit/ParticleFitObject.h"

namespace Belle2 {

  namespace OrcaKinFit {

// Class JetFitObject
/// Class for jets with (E, eta, phi) in kinematic fits
    /**
     *
     * Author: Jenny List, Benno List
     * $Date: 2011/03/03 15:03:02 $
     * $Author: blist $
     *
     * \b Changelog:
     * - 30.12.04 BL: addToGlobCov, getDChi2DParam, getDChi2DParam2, addToGlobalChi2DerMatrix
     *            moved up to ParticleFitObject,
     *            getParamName implemented
     */
    class JetFitObject : public ParticleFitObject {
    public:
      JetFitObject(double E, double theta, double phi,
                   double DE, double Dtheta, double Dphi,
                   double m = 0);

      /// Copy constructor
      JetFitObject(const JetFitObject& rhs               ///< right hand side
                  );
      /// Assignment
      JetFitObject& operator= (const JetFitObject& rhs   ///< right hand side
                              );

      virtual ~JetFitObject();

      /// Return a new copy of itself
      virtual JetFitObject* copy() const;

      /// Assign from anther object, if of same type
      virtual JetFitObject& assign(const BaseFitObject& source    ///< The source object
                                  );

      /// Get name of parameter ilocal
      virtual const char* getParamName(int ilocal      ///< Local parameter number
                                      ) const;

      /// Read values from global vector, readjust vector; return: significant change
      virtual bool   updateParams(double p[],    ///< The parameter vector
                                  int idim      ///< Length of the vector
                                 );

      virtual int getNPar() const {return NPAR;}

      // these depend on actual parametrisation!

      virtual double getDPx(int ilocal) const;
      virtual double getDPy(int ilocal) const;
      virtual double getDPz(int ilocal) const;
      virtual double getDE(int ilocal) const;

      virtual double getCov(int
                            ilocal,     ///< Local parameter number i
                            int jlocal     ///< Local parameter number j
                           ) const ;

      /// Get error of parameter ilocal
      virtual double getError(int ilocal      ///< Local parameter number
                             ) const;


      /// add derivatives to vector der of size idim
      /// pxfact*dpx/dx_i + pyfact*dpy/dx_i + pzfact*dpz/dx_i + efact*dE/dx_i
//    virtual void   addToDerivatives (double der[],      ///< Derivatives vector, length idim
//                                     int idim,          ///< Length of derivatives vector
//                                     double efact=0,    ///< Factor for dE/dx_i
//                                     double pxfact=0,   ///< Factor for dpx/dx_i
//                                     double pyfact=0,   ///< Factor for dpy/dx_i
//                                     double pzfact=0    ///< Factor for dpz/dx_i
//                                     ) const;

      // daniel's new method
      // derivatives of intermediate variable wrt local variable
      virtual double getFirstDerivative_Meta_Local(int iMeta, int ilocal , int metaSet) const;
      virtual double getSecondDerivative_Meta_Local(int iMeta, int ilocal , int jlocal , int metaSet) const;

      /// Get chi squared from measured and fitted parameters
      //    virtual double getChi2() const;

    protected:

      enum {NPAR = 3};

      void updateCache() const;

      mutable double ctheta, stheta, cphi, sphi,
              p2, p, pt, px, py, pz, dpdE, dptdE,
              dpxdE, dpydE, dpzdE, dpxdtheta, dpydtheta,
              chi2;
      // d2pdE2, d2ptsE2;

      /// Adjust E, theta and phi such that E>=m, 0<=theta<=pi, -pi <= phi < pi; returns true if anything was changed
      static bool adjustEThetaPhi(double& m, double& E, double& theta, double& phi);

      /// Calculate chi2
      //    double calcChi2 () const;

    };


  }// end OrcaKinFit namespace
} // end Belle2 namespace


#endif // __JETFITOBJECT_H

