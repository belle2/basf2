/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * Forked from https://github.com/iLCSoft/MarlinKinfit                    *
 *                                                                        *
 * Further information about the fit engine and the user interface        *
 * provided in MarlinKinfit can be found at                               *
 * https://www.desy.de/~blist/kinfit/doc/html/                            *
 * and in the LCNotes LC-TOOL-2009-001 and LC-TOOL-2009-004 available     *
 * from http://www-flc.desy.de/lcnotes/                                   *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef __ISRPHOTONFITOBJECT_H
#define __ISRPHOTONFITOBJECT_H

#include "analysis/OrcaKinFit/ParticleFitObject.h"

namespace Belle2 {
  namespace OrcaKinFit {

    class ISRPhotonFitObject : public ParticleFitObject {
    public:
      ISRPhotonFitObject(double px,           ///< initial px value for photon (fixed)
                         double py,           ///< initial py value for photon (fixed)
                         double pz,           ///< initial pz value for photon
                         double b_,           ///< photon spectrum parametrization
                         double PzMaxB_,      ///< photon spectrum parametrization
                         double PzMinB_ = 0.  ///< photon spectrum parametrization
                        );

      /// Copy constructor
      ISRPhotonFitObject(const ISRPhotonFitObject& rhs               ///< right hand side
                        );
      /// Assignment
      ISRPhotonFitObject& operator= (const ISRPhotonFitObject& rhs); ///< right hand side

      virtual ~ISRPhotonFitObject();

      /// Return a new copy of itself
      virtual ISRPhotonFitObject* copy() const override;

      /// Assign from anther object, if of same type
      virtual ISRPhotonFitObject& assign(const BaseFitObject& source    ///< The source object
                                        ) override;

      /// Get name of parameter ilocal
      virtual const char* getParamName(int ilocal      ///< Local parameter number
                                      ) const override;

      /// Read values from global vector, readjust vector; return: significant change
      virtual bool   updateParams(double p[],    ///< The parameter vector
                                  int idim      ///< Length of the vector
                                 ) override;


      // // these depend on actual parametrisation!
      virtual double getDPx(int ilocal) const override;
      virtual double getDPy(int ilocal) const override;
      virtual double getDPz(int ilocal) const override;
      virtual double getDE(int ilocal) const override;

      virtual double getFirstDerivative_Meta_Local(int iMeta, int ilocal, int metaSet) const override;
      virtual double getSecondDerivative_Meta_Local(int iMeta, int ilocal, int jlocal, int metaSet) const override;

      virtual int getNPar() const override {return NPAR;}

    protected:

      enum {NPAR = 3}; // well, it's actually 1...Daniel should update

      double PgFromPz(double pz);

      void updateCache() const override;

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
