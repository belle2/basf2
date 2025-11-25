/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <Math/Vector3D.h>
#include <Math/Rotation3D.h>

namespace Belle2 {
  namespace arich {
    //! Computes error function (from Numerical Recipes)
    /*!
      \param x erf function argument
     */
    double Erf(double x);

    //! returns photon direction after refraction.
    /*!
      \param s vector of initial photon direction
      \param n quotient of media refractive indices (n_2/n_1), assumes normal vector of boundary plane in z direction.
    */
    ROOT::Math::XYZVector Refraction(const ROOT::Math::XYZVector s,  const double n);

    //! Computes photon direction after refraction.
    /*!
      \param s vector of initial photon direction
      \param norm  boundary plane normal vector
      \param n quotient of media refractive indices (n_2/n_1)
      \param a photon direction after refraction
    */
    int Refraction(ROOT::Math::XYZVector s, ROOT::Math::XYZVector norm, double n, ROOT::Math::XYZVector& a);

    //! Returns unit vector with polar angle theta and azimuth fi.
    /*!
      \param theta polar angle
      \param fi azumuthal angle
     */
    ROOT::Math::XYZVector setThetaPhi(double theta, double fi);

    //! Returns matrix for transformation from "track" coordinate system to "global" coordinate system.
    /*!
      \param r vector of track direction
     */
    ROOT::Math::Rotation3D TransformFromFixed(ROOT::Math::XYZVector r);

    //! Returns matrix for transformation from global coordinate system to "track" coordinate system.
    /*!
      \param r vector of track direction.
    */
    ROOT::Math::Rotation3D TransformToFixed(ROOT::Math::XYZVector r);

    //! Returns expected emission angle of Cherenkov photon, emitted by particle
    /*!
      \param p momentum of particle
      \param m particles mass
      \param refind medium refractive index
     */
    double  ExpectedCherenkovAngle(double p, double mass, double refind);

    //! integral of Gaussian function over pad square.
    /*! Returns the value of integral of a Gaussian function with width "sigma",
      over the square (pad) with size "aaa". Integrated function is constant
      in one direction and Gaussian in perpendicular direction.

      \param aaa square (pad) size
      \param fi angle between Gaussian direction and pad orientation.
      \param mean distance between pad center and Gaussian peak.
      \param sigma width of a Gaussian function


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

  } // namespace arich
} // namespace Belle2