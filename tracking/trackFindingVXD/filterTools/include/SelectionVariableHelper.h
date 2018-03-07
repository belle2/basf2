/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <math.h>

#include <framework/geometry/B2Vector3.h>
#include <boost/math/special_functions/sign.hpp>
#include <framework/core/FrameworkExceptions.h>


namespace Belle2 {

  /** contains a collection of functions and related stuff needed for SelectionVariables implementing 2-, 3- and 4-hitfilters.
   * Template PointType expects a class having the functions X(), Y() and Z() returning Datatype.
   * Template DataType should behave like a primitive like double or float. */
  template<class PointType, class DataType>
  struct SelVarHelper {


    /** is factor containing speed of light (c),
     * the magnetic field (b) and the scaling factor s for conversion of meter in cm : c*b/100 = c*b*s.
     * The standard value assumes a magnetic field of 1.5 Tesla.
     * Value can be changed using the resetMagneticField-Function, where a new value for the magnetic field in Tesla has to be passed.
     * TODO WARNING hardcoded value!  */
    static DataType s_MagneticFieldFactor; //  = 1.5 * 0.00299710;


    /** Overrides standard-Setup for magnetic field.
     *
     * If no value is given, magnetic field is assumed to be Belle2-Detector standard of 1.5T.
     * pT[GeV/c] = 0.299710*B[T]*r[m] = 0.299710*B[T]*r[cm]/100 = 0.00299710B[T]*r[cm].
     */
    static void resetMagneticField(DataType magneticFieldStrength = 1.5) { SelVarHelper::s_MagneticFieldFactor = magneticFieldStrength * 0.00299710; }


    /** returns the perpendicular magnitude squared of given pointType. */
    static DataType calcPerp2(const PointType& aHit)
    { return (aHit.X() * aHit.X() + aHit.Y() * aHit.Y()); }


    /** returns the perpendicular magnitude of given pointType. */
    static DataType calcPerp(const PointType& aHit) { return sqrt(calcPerp2(aHit)); }


    /** returns the magnitude squared of given pointType. */
    static DataType calcMag2(const PointType& aHit)
    { return (aHit.X() * aHit.X() + aHit.Y() * aHit.Y() + aHit.Z() * aHit.Z()); }


    /** returns the magnitude of given pointType. */
    static DataType calcMag(const PointType& aHit) { return sqrt(calcMag2(aHit)); }


    /** returns scalar product of given two pointTypes. */
    static DataType calcDot(const PointType& aHit, const PointType& bHit)
    { return (aHit.X() * bHit.X() + aHit.Y() * bHit.Y() + aHit.Z() * bHit.Z()); }

    /** calculates the estimation of the transverse momentum of given radius using defined strength of magnetic field, returning unit: GeV/c. */
    static DataType calcPt(DataType radius, DataType field = SelVarHelper::s_MagneticFieldFactor)
    {
      // TODO WARNING hardcoded value, is there a quasi-global value for such cases (this case, minimal accepted radius)
      // if (fabs(radius) < 0.0000001) { throw (Circle_too_small() << radius << 0.0000001); }

      //WARNING: does not care for negative radius!!
      return field * radius;
    } // return unit: GeV/c

  };

  template<typename PointType, typename DataType> DataType SelVarHelper<PointType, DataType>::s_MagneticFieldFactor = 1.5 *
      0.00299710;

}
