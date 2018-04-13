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

#ifndef __TWOVECTOR_H
#define __TWOVECTOR_H

#include <iostream>
#include <cmath>

namespace Belle2 {

  namespace OrcaKinFit {

    class TwoVector {
    public:
      inline TwoVector();
      inline TwoVector(double x_, double y_);
      // automatically generated copy constructor and assignment is fine

      inline double getX()     const;
      inline double getY()     const;

      inline double getMag2()    const;
      inline double getMag()   const;

      inline double getPhi()   const;

      inline double getComponent(int i) const;

      inline TwoVector& setValues(double x_, double y_);

      inline TwoVector& operator+= (const TwoVector& rhs);
      inline TwoVector& operator-= (const TwoVector& rhs);
      inline TwoVector& operator*= (double rhs);

    private:
      double x, y;
    };

    TwoVector::TwoVector()
      : x(0), y(0)
    {}

    TwoVector::TwoVector(double x_, double y_)
      : x(x_), y(y_)
    {}

    double TwoVector::getX()  const { return x; }
    double TwoVector::getY()  const { return y; }

    double TwoVector::getMag2() const { return x * x + y * y; }
    double TwoVector::getMag()const { return std::sqrt(getMag2()); }

    double TwoVector::getPhi()   const { return std::atan2(y, x); }

    double TwoVector::getComponent(int i) const
    {
      switch (i) {
        case 0: return getX();
        case 1: return getY();
      }
      return NAN; // not-a-number, defined in cmath
    }

    TwoVector& TwoVector::setValues(double x_, double y_)
    {
      x = x_;
      y = y_;
      return *this;
    }


    TwoVector& TwoVector::operator+= (const TwoVector& rhs)
    {
      x += rhs.x;
      y += rhs.y;
      return *this;
    }

    TwoVector& TwoVector::operator-= (const TwoVector& rhs)
    {
      x -= rhs.x;
      y -= rhs.y;
      return *this;
    }

    TwoVector& TwoVector::operator*= (double rhs)
    {
      x *= rhs;
      y *= rhs;
      return *this;
    }

    inline TwoVector operator+ (const TwoVector& lhs, const TwoVector& rhs)
    {
      return TwoVector(lhs.getX() + rhs.getX(), lhs.getY() + rhs.getY());
    }

    inline TwoVector operator- (const TwoVector& lhs, const TwoVector& rhs)
    {
      return TwoVector(lhs.getX() - rhs.getX(), lhs.getY() - rhs.getY());
    }

    inline TwoVector operator- (const TwoVector& rhs)
    {
      return TwoVector(-rhs.getX(), -rhs.getY());
    }

    inline double operator* (const TwoVector& lhs, const TwoVector& rhs)
    {
      return lhs.getX() * rhs.getX() + lhs.getY() * rhs.getY();
    }

    inline TwoVector operator* (double lhs, const TwoVector& rhs)
    {
      return TwoVector(lhs * rhs.getX(), lhs * rhs.getY());
    }

    inline std::ostream& operator<< (std::ostream& out, const TwoVector& v)
    {
      out << "(" << v.getX() << ", " << v.getY() << ")";
      return out;
    }

  }// end OrcaKinFit namespace
} // end Belle2 namespace

#endif /* #ifndef __TWOVECTOR_H */


