/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * PxPyPzE fit object                                                     *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca) (TF)               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef __PXPYPZMFITOBJECT_H
#define __PXPYPZMFITOBJECT_H

#include "analysis/OrcaKinFit/ParticleFitObject.h"

// CLHEP
#include <CLHEP/Matrix/Matrix.h>
#include <CLHEP/Matrix/SymMatrix.h>
#include <CLHEP/Vector/LorentzVector.h>
using namespace CLHEP;

class PxPyPzMFitObject : public ParticleFitObject {
public:

  PxPyPzMFitObject(HepLorentzVector& particle, HepSymMatrix& covmatrix);

  // Copy constructor
  PxPyPzMFitObject(const PxPyPzMFitObject& rhs               ///< right hand side
                  );
  // Assignment
  PxPyPzMFitObject& operator= (const PxPyPzMFitObject& rhs   ///< right hand side
                              );

  virtual ~PxPyPzMFitObject();

  // Return a new copy of itself
  virtual PxPyPzMFitObject* copy() const;

  // Assign from anther object, if of same type
  virtual PxPyPzMFitObject& assign(const BaseFitObject& source    ///< The source object
                                  );

  // Get name of parameter ilocal
  virtual const char* getParamName(int ilocal      ///< Local parameter number
                                  ) const;

  // Read values from global vector, readjust vector; return: significant change
  virtual bool   updateParams(double p[],    ///< The parameter vector
                              int idim      ///< Length of the vector
                             );

  // these depend on actual parametrisation!
  virtual double getDPx(int ilocal) const;
  virtual double getDPy(int ilocal) const;
  virtual double getDPz(int ilocal) const;
  virtual double getDE(int ilocal) const;

  virtual double getFirstDerivative_Meta_Local(int iMeta, int ilocal ,
                                               int metaSet) const;   // derivative of intermediate variable iMeta wrt local parameter ilocal
  virtual double getSecondDerivative_Meta_Local(int iMeta, int ilocal , int jlocal,
                                                int metaSet) const;   // derivative of intermediate variable iMeta wrt local parameter ilocal

  virtual int getNPar() const {return NPAR;}

protected:

  void updateCache() const;

  mutable bool cachevalid;

  mutable double chi2,
          dEdpx, dEdpy, dEdpz,
          dE2dpxdpx, dE2dpxdpy, dE2dpxdpz,
          dE2dpydpy, dE2dpydpz,
          dE2dpzdpz;

  enum {NPAR = 3};

};


#endif // __PXPYPZMFITOBJECT_H
