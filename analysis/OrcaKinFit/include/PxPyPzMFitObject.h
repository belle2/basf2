/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef __PXPYPZMFITOBJECT_H
#define __PXPYPZMFITOBJECT_H

#include "analysis/OrcaKinFit/ParticleFitObject.h"

// CLHEP
#include <CLHEP/Matrix/Matrix.h>
#include <CLHEP/Matrix/SymMatrix.h>
#include <CLHEP/Vector/LorentzVector.h>

namespace Belle2 {
  namespace OrcaKinFit {

    class PxPyPzMFitObject : public ParticleFitObject {
    public:

      PxPyPzMFitObject(CLHEP::HepLorentzVector& particle, const CLHEP::HepSymMatrix& covmatrix);

      // Copy constructor
      PxPyPzMFitObject(const PxPyPzMFitObject& rhs               ///< right hand side
                      );
      // Assignment
      PxPyPzMFitObject& operator= (const PxPyPzMFitObject& rhs   ///< right hand side
                                  );

      virtual ~PxPyPzMFitObject();

      // Return a new copy of itself
      virtual PxPyPzMFitObject* copy() const override;

      // Assign from anther object, if of same type
      virtual PxPyPzMFitObject& assign(const BaseFitObject& source    ///< The source object
                                      ) override;

      // Get name of parameter ilocal
      virtual const char* getParamName(int ilocal      ///< Local parameter number
                                      ) const override;

      // Read values from global vector, readjust vector; return: significant change
      virtual bool   updateParams(double p[],    ///< The parameter vector
                                  int idim      ///< Length of the vector
                                 ) override;

      // these depend on actual parametrisation!
      virtual double getDPx(int ilocal) const override;
      virtual double getDPy(int ilocal) const override;
      virtual double getDPz(int ilocal) const override;
      virtual double getDE(int ilocal) const override;

      virtual double getFirstDerivative_Meta_Local(int iMeta, int ilocal ,
                                                   int metaSet) const override;   // derivative of intermediate variable iMeta wrt local parameter ilocal
      virtual double getSecondDerivative_Meta_Local(int iMeta, int ilocal , int jlocal,
                                                    int metaSet) const override;   // derivative of intermediate variable iMeta wrt local parameter ilocal

      virtual int getNPar() const override {return NPAR;}

    protected:

      void updateCache() const override;

      mutable bool cachevalid;

      mutable double chi2,
              dEdpx, dEdpy, dEdpz,
              dE2dpxdpx, dE2dpxdpy, dE2dpxdpz,
              dE2dpydpy, dE2dpydpz,
              dE2dpzdpz;

      enum {NPAR = 3};

    };

  }// end OrcaKinFit namespace
} // end Belle2 namespace



#endif // __PXPYPZMFITOBJECT_H
