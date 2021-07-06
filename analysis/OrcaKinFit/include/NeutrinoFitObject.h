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

#ifndef __NEUTRINOFITOBJECT_H
#define __NEUTRINOFITOBJECT_H

#include "analysis/OrcaKinFit/ParticleFitObject.h"

#include <cmath>

namespace Belle2 {
  namespace OrcaKinFit {

    class NeutrinoFitObject : public ParticleFitObject {
    public:
      NeutrinoFitObject(double E, double theta, double phi,
                        double DE = 1, double Dtheta = 0.1, double Dphi = 0.1);

      /// Copy constructor
      NeutrinoFitObject(const NeutrinoFitObject& rhs               ///< right hand side
                       );
      /// Assignment
      NeutrinoFitObject& operator= (const NeutrinoFitObject& rhs   ///< right hand side
                                   );

      virtual ~NeutrinoFitObject();

      /// Return a new copy of itself
      virtual NeutrinoFitObject* copy() const override;

      /// Assign from anther object, if of same type
      virtual NeutrinoFitObject& assign(const BaseFitObject& source    ///< The source object
                                       ) override;

      /// Get name of parameter ilocal
      virtual const char* getParamName(int ilocal      ///< Local parameter number
                                      ) const override;

      /// Read values from global vector, readjust vector; return: significant change
      virtual bool   updateParams(double p[],    ///< The parameter vector
                                  int idim      ///< Length of the vector
                                 ) override;

      // these depend on actual parametrisation!
      virtual double getDPx(int ilocal) const override;
      virtual double getDPy(int ilocal) const override;
      virtual double getDPz(int ilocal) const override;
      virtual double getDE(int ilocal) const override;

      virtual double getFirstDerivative_Meta_Local(int iMeta, int ilocal , int metaSet) const override;
      virtual double getSecondDerivative_Meta_Local(int iMeta, int ilocal , int jlocal , int metaSet) const override;

      virtual int getNPar() const override {return NPAR;}

    protected:
      void updateCache() const override;

      enum {NPAR = 3};

      mutable double ctheta, stheta, cphi, sphi,
              pt, px, py, pz, dptdE,
              dpxdE, dpydE, dpxdtheta, dpydtheta,
              chi2;

    };

  }// end OrcaKinFit namespace
} // end Belle2 namespace


#endif // __NEUTRINOFITOBJECT_H

