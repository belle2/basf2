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

////////////////////////////////////////////////////////////////
// Class ThreeVector
//
// Author: Benno List
// Last update: $Date: 2008/02/12 10:19:07 $
//          by: $Author: blist $
//
// Description: class for three-vectors
//
////////////////////////////////////////////////////////////////

#ifndef __THREEVECTOR_H
#define __THREEVECTOR_H

#include <iostream>
#include <cmath>

namespace Belle2 {
  namespace OrcaKinFit {


    class ThreeVector {
    public:
      inline ThreeVector();
      inline ThreeVector(double px_, double py_, double pz_);
      // automatically generated copy constructor and assignment is fine

      inline double getPx()    const;
      inline double getPy()    const;
      inline double getPz()    const;
      inline double getX()     const;
      inline double getY()     const;
      inline double getZ()     const;

      inline double getP2()    const;
      inline double getP()     const;
      inline double getMag()   const;
      inline double getPt2()   const;
      inline double getPt()    const;
      inline double getR()     const;

      inline double getPhi()   const;
      inline double getTheta() const;
      inline double getEta()   const;

      inline double getComponent(int i) const;

      inline ThreeVector& setValues(double px_, double py_, double pz_);

      inline ThreeVector& operator+= (const ThreeVector& rhs);
      inline ThreeVector& operator-= (const ThreeVector& rhs);
      inline ThreeVector& operator*= (double rhs);

    private:
      double px, py, pz;
    };

    ThreeVector::ThreeVector()
      : px(0), py(0), pz(0)
    {}

    ThreeVector::ThreeVector(double px_, double py_, double pz_)
      : px(px_), py(py_), pz(pz_)
    {}

    double ThreeVector::getPx() const { return px; }
    double ThreeVector::getPy() const { return py; }
    double ThreeVector::getPz() const { return pz; }
    double ThreeVector::getX()  const { return px; }
    double ThreeVector::getY()  const { return py; }
    double ThreeVector::getZ()  const { return pz; }

    double ThreeVector::getPt2() const { return px * px + py * py; }
    double ThreeVector::getPt()  const { return std::sqrt(getPt2()); }
    double ThreeVector::getR()   const { return std::sqrt(getPt2()); }

    double ThreeVector::getP2() const { return px * px + py * py + pz * pz; }
    double ThreeVector::getP()  const { return std::sqrt(getP2()); }
    double ThreeVector::getMag()const { return std::sqrt(getP2()); }

    double ThreeVector::getPhi()   const { return std::atan2(py, px); }
    double ThreeVector::getTheta() const { return std::atan2(getPt(), pz); }
    double ThreeVector::getEta() const { return -std::log(std::tan(0.5 * getTheta())); }

    double ThreeVector::getComponent(int i) const
    {
      switch (i) {
        case 0: return getPx();
        case 1: return getPy();
        case 2: return getPz();
      }
      return NAN; // not-a-number, defined in cmath
    }

    ThreeVector& ThreeVector::setValues(double px_, double py_, double pz_)
    {
      px = px_;
      py = py_;
      pz = pz_;
      return *this;
    }


    ThreeVector& ThreeVector::operator+= (const ThreeVector& rhs)
    {
      px += rhs.px;
      py += rhs.py;
      pz += rhs.pz;
      return *this;
    }

    ThreeVector& ThreeVector::operator-= (const ThreeVector& rhs)
    {
      px -= rhs.px;
      py -= rhs.py;
      pz -= rhs.pz;
      return *this;
    }

    ThreeVector& ThreeVector::operator*= (double rhs)
    {
      px *= rhs;
      py *= rhs;
      pz *= rhs;
      return *this;
    }

    inline ThreeVector operator+ (const ThreeVector& lhs, const ThreeVector& rhs)
    {
      return ThreeVector(lhs.getPx() + rhs.getPx(), lhs.getPy() + rhs.getPy(), lhs.getPz() + rhs.getPz());
    }

    inline ThreeVector operator- (const ThreeVector& lhs, const ThreeVector& rhs)
    {
      return ThreeVector(lhs.getPx() - rhs.getPx(), lhs.getPy() - rhs.getPy(), lhs.getPz() - rhs.getPz());
    }

    inline ThreeVector operator- (const ThreeVector& rhs)
    {
      return ThreeVector(-rhs.getPx(), -rhs.getPy(), -rhs.getPz());
    }

    inline double operator* (const ThreeVector& lhs, const ThreeVector& rhs)
    {
      return lhs.getPx() * rhs.getPx() + lhs.getPy() * rhs.getPy() + lhs.getPz() * rhs.getPz();
    }

    inline ThreeVector operator* (double lhs, const ThreeVector& rhs)
    {
      return ThreeVector(lhs * rhs.getPx(), lhs * rhs.getPy(), lhs * rhs.getPz());
    }

    inline std::ostream& operator<< (std::ostream& out, const ThreeVector& v)
    {
      out << "(" << v.getPx() << ", " << v.getPy() << ", " << v.getPz() << ")";
      return out;
    }


  }// end OrcaKinFit namespace
} // end Belle2 namespace


#endif // __THREEVECTOR_H

