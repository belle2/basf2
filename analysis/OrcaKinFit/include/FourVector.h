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

#ifndef __FOURVECTOR_H
#define __FOURVECTOR_H

#include "analysis/OrcaKinFit/ThreeVector.h"

namespace Belle2 {

  namespace OrcaKinFit {

#include <iostream>
#include <cmath>
#include <cassert>

//  Class FourVector:
/// Yet another four vector class, with metric +---
    /**
     *
     * Author: Benno List
     * Last update: $Date: 2008/02/12 10:19:05 $
     *          by: $Author: blist $
     *
     */
    class FourVector {
    public:
      /// Default constructor
      inline FourVector();
      /// Constructor from the components
      inline FourVector(double E_, double px_, double py_, double pz_);
      /// Constructor from energy and three momentum
      inline FourVector(double E_, const ThreeVector& p_);
      /// Constructor from three momentum and mass
      inline FourVector(const ThreeVector& p_, double m_);
      // automatically generated copy constructor and assignment is fine

      /// Returns the energy / 0 component
      inline double getE()     const;
      /// Returns the x momentum / 1 component
      inline double getPx()    const;
      /// Returns the y momentum / 2 component
      inline double getPy()    const;
      /// Returns the z momentum / 3 component
      inline double getPz()    const;
      /// Returns the mass squared / magnitude squared
      inline double getM2()    const;
      /// Returns the mass / magnitude
      inline double getM()     const;
      /// Returns the mass / magnitude
      inline double getMass()     const;

      /// Returns the momentum squared / magnitude of the three vector squared
      inline double getP2()    const;
      /// Returns the momentum / magnitude of the three vector
      inline double getP()     const;
      /// Returns the transverse momentum squared / magnitude of the 1 and 2 component vector squared
      inline double getPt2()   const;
      /// Returns the transverse momentum / magnitude of the 1 and 2 component vector
      inline double getPt()    const;

      /// Returns the azimuthal angle of the momentum / three vector squared
      inline double getPhi()   const;
      /// Returns the polar angle of the momentum / three vector squared
      inline double getTheta() const;
      /// Returns the pseudo rapidity of the momentum / three vector squared
      inline double getEta() const;

      /// Returns the i'th component (starting from 0=energy)
      inline double getComponent(int i) const;

      inline ThreeVector getBeta() const { assert(E > 0); return (1. / E) * p;};
      inline double getGamma() const { assert(getM() > 0); return getE() / getM();};
      inline ThreeVector getBetaGamma() const { assert(getM() > 0); return (1. / getM() > 0) * p;};

      /// Returns the momentum three vector
      inline const ThreeVector& getThreeVector() const { return p;}

      FourVector& boost(const FourVector& P);
      void decayto(FourVector& d1, FourVector& d2) const;

      inline void setValues(double E_, double px_, double py_, double pz_);

      inline FourVector& operator+= (const FourVector& rhs);
      inline FourVector& operator-= (const FourVector& rhs);

      inline FourVector& operator*= (double rhs);

    private:
      double E;         ///< The energy / 0 component
      ThreeVector p;    ///< The momentum three vector
    };

    FourVector::FourVector()
      : E(0), p()
    {}

    FourVector::FourVector(double E_, double px_, double py_, double pz_)
      : E(E_), p(px_, py_, pz_)
    {}

    FourVector::FourVector(double E_, const ThreeVector& p_)
      : E(E_), p(p_)
    {}

    FourVector::FourVector(const ThreeVector& p_, double m)
      : E(std::sqrt(p_.getP2() + m * m)), p(p_)
    {}

    double FourVector::getE()  const { return E; }
    double FourVector::getPx() const { return p.getPx(); }
    double FourVector::getPy() const { return p.getPy(); }
    double FourVector::getPz() const { return p.getPz(); }

    double FourVector::getPt2() const { return p.getPt2(); }
    double FourVector::getPt()  const { return p.getPt(); }

    double FourVector::getP2() const { return p.getP2(); }
    double FourVector::getP()  const { return p.getP(); }

    double FourVector::getM2()   const { return std::abs(getE() * getE() - getP2()); }
    double FourVector::getM()    const { return std::sqrt(getM2()); }
    double FourVector::getMass() const { return std::sqrt(getM2()); }

    double FourVector::getPhi()   const { return p.getPhi(); }
    double FourVector::getTheta() const { return p.getTheta(); }

    double FourVector::getEta() const { return p.getEta(); }

    double FourVector::getComponent(int i) const
    {
      switch (i) {
        case 1: return getPx();
        case 2: return getPy();
        case 3: return getPz();
      }
      return getE();
    }

    void FourVector::setValues(double E_, double px_, double py_, double pz_)
    {
      E = E_;
      p.setValues(px_, py_, pz_);
    }

    FourVector& FourVector::operator+= (const FourVector& rhs)
    {
      p += rhs.p;
      E += rhs.E;
      return *this;
    }

    FourVector& FourVector::operator-= (const FourVector& rhs)
    {
      p -= rhs.p;
      E -= rhs.E;
      return *this;
    }

    FourVector& FourVector::operator*= (double rhs)
    {
      p *= rhs;
      E *= rhs;
      return *this;
    }

    /**
     * \relates  FourVector
     * \brief Sum of two four vectors
     */
    inline FourVector operator+ (const FourVector& lhs, const FourVector& rhs)
    {
      return FourVector(lhs.getE() + rhs.getE(), lhs.getPx() + rhs.getPx(), lhs.getPy() + rhs.getPy(), lhs.getPz() + rhs.getPz());
    }

    /**
     * \relates  FourVector
     * \brief Difference of two four vectors
     */
    inline FourVector operator- (const FourVector& lhs, const FourVector& rhs)
    {
      return FourVector(lhs.getE() - rhs.getE(), lhs.getPx() - rhs.getPx(), lhs.getPy() - rhs.getPy(), lhs.getPz() - rhs.getPz());
    }

    /**
     * \relates FourVector
     * \brief Negative of a four vector
     */
    inline FourVector operator- (const FourVector& rhs)
    {
      return FourVector(-rhs.getE(), -rhs.getPx(), -rhs.getPy(), -rhs.getPz());
    }

    /**
     * \relates  FourVector
     * \brief Scalar product of a four vector
     */
    inline FourVector operator* (double lhs, const FourVector& rhs)
    {
      return FourVector(lhs * rhs.getE(), lhs * rhs.getPx(), lhs * rhs.getPy(), lhs * rhs.getPz());
    }

    /**
     * \relates  FourVector
     * \brief Scalar product of a four vector
     */
    inline double operator* (const FourVector& lhs, const FourVector& rhs)
    {
      return lhs.getE() * rhs.getE() - lhs.getPx() * rhs.getPx() - lhs.getPy() * rhs.getPy() - lhs.getPz() * rhs.getPz();
    }


    /**
     * \relates  FourVector
     * \brief Prints a four vector
     */
    inline std::ostream& operator<< (std::ostream& os,
                                     const FourVector& rhs
                                    )
    {
      os << "(" << rhs.getE() << ", " << rhs.getPx() << ", " << rhs.getPy() << ", " << rhs.getPz() << ")";
      return os;
    }

  }// end OrcaKinFit namespace
} // end Belle2 namespace

#endif // __FOURVECTOR_H
