/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
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

#ifndef __ISRPHOTONFITOBJECT_H
#define __ISRPHOTONFITOBJECT_H

#include "analysis/OrcaKinFit/ParticleFitObject.h"

namespace Belle2 {
  namespace OrcaKinFit {

    class ISRPhotonFitObject : public ParticleFitObject {
    public:
      ISRPhotonFitObject(double px, double py, double pz,                   /// initial values for photon (p_x,p_y fix)
                         double b_, double PzMaxB_, double PzMinB_ = 0.);  /// photon spectrum parametrization (see above)

      /// Copy constructor
      ISRPhotonFitObject(const ISRPhotonFitObject& rhs               ///< right hand side
                        );
      /// Assignment
      ISRPhotonFitObject& operator= (const ISRPhotonFitObject& rhs); ///< right hand side

      virtual ~ISRPhotonFitObject();

      /// Return a new copy of itself
      virtual ISRPhotonFitObject* copy() const;

      /// Assign from anther object, if of same type
      virtual ISRPhotonFitObject& assign(const BaseFitObject& source    ///< The source object
                                        );

      /// Get name of parameter ilocal
      virtual const char* getParamName(int ilocal      ///< Local parameter number
                                      ) const;

      /// Read values from global vector, readjust vector; return: significant change
      virtual bool   updateParams(double p[],    ///< The parameter vector
                                  int idim      ///< Length of the vector
                                 );


      // // these depend on actual parametrisation!
      virtual double getDPx(int ilocal) const;
      virtual double getDPy(int ilocal) const;
      virtual double getDPz(int ilocal) const;
      virtual double getDE(int ilocal) const;

      virtual double getFirstDerivative_Meta_Local(int iMeta, int ilocal , int metaSet) const;
      virtual double getSecondDerivative_Meta_Local(int iMeta, int ilocal , int jlocal, int metaSet) const;

      virtual int getNPar() const {return NPAR;}

    protected:

      enum {NPAR = 3}; // well, it's actually 1...Daniel should update

      virtual double PgFromPz(double pz);

      void updateCache() const;

      mutable bool cachevalid;

      mutable double pt2, p2, p, pz,
              dpx0, dpy0, dpz0, dE0, dpx1, dpy1, dpz1, dE1,
              dpx2, dpy2, dpz2, dE2, d2pz22, d2E22,
              chi2,
              b, PzMinB, PzMaxB, dp2zFact;
    };

  }// end OrcaKinFit namespace
} // end Belle2 namespace

#endif // __ISRPHOTONFITOBJECT_H
