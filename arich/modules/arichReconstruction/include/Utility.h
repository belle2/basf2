/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj, Rok Pestotnik                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHUTILITY_HH
#define ARICHUTILITY_HH

#include <vector>
#include <stdlib.h>
#include <TVector3.h>
#include <TRotation.h>

namespace Belle2 {
  //! Computes error function (from Numerical Recipes)
  double Erf(double x);

  //! returns photon direction after refraction.
  //! s is initial photon direction, n is quotient of media refractive indices
  //! (n_2/n_1), assumes normal vector of boundary plane in z direction.
  TVector3 Refraction(const TVector3 s,  const double n);

  //! Computes photon direction after refraction (vector a).
  //! s is initial photon direction, norm is boundary plane normal vector,
  //! n is quotient of media refractive indices.

  int Refraction(TVector3 s, TVector3 norm, double n, TVector3 &a);

  //! Returns unit vector with polar angle theta and azimuth fi.
  TVector3 setThetaPhi(double theta, double fi);

  //! Returns matrix for transformation from "track" coordinate system to "global" coordinate system.
  //! Vector r is the track direction.
  TRotation TransformFromFixed(TVector3 r);

  //! Returns matrix for transformation from global coordinate system to "track" coordinate system.
  //! Vector r is the track direction.
  TRotation TransformToFixed(TVector3 r);

  //! Returns expected emission angle of Cherenkov photon, emitted by particle
  //! with momentum "p" and mass "mass" in medium with refractive index "refind".
  double  ExpectedCherenkovAngle(double p, double mass, double refind);

  //! Returns the value of integral of a Gaussian function with width "sigma",
  //! over the square (pad) with size "aaa". Integrated function is constant
  //! in one direction and Gaussian in perpendicular direction.
  //! "phi" is the angle between square orientation and "Gaussian" direction,
  //! and mean is the distance between square center and Gaussian peak in
  //! this direction.
  /*
                                /  /  /
                             øøøøøøøøø
                             ø/  /  /ø
                             ø  /  / ø
                            /ø /  /  ø
                           / ø/  /   ø
                          /  øøøøøøøøø
                         /  /  /
                           _
                          / \ Gauss
                       __/   \_
  */
  double SquareInt(double aaa, double fi, double mean, double sigma);

}

#endif
