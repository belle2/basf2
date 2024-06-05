/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <cmath>
#include <Math/Vector3D.h>


namespace Belle2 {
  namespace TOP {

    /** commonly used functions */
    namespace func {

      /**
       * unfold a coordinate.
       * @param x true position
       * @param nx signed number of reflections
       * @param A size for unfolding
       * @return unfolded coordinate (position of image)
       */
      inline double unfold(double x, int nx, double A)
      {
        if (nx % 2 == 0) return (nx * A + x);
        else return (nx * A - x);
      }


      /**
       * unfold a direction.
       * @param kx true direction component
       * @param nx signed number of reflections
       * @return unfolded direction component
       */
      inline double unfold(double kx, int nx)
      {
        if (nx % 2 == 0) return kx;
        else return -kx;
      }


      /**
       * fold a coordinate (inverse of unfold).
       * @param xu unfolded coordinate (position of image)
       * @param A size for folding
       * @param x true position [out]
       * @param kx true direction component [in/out]
       * @param nx signed number of reflections [out]
       */
      inline void fold(double xu, double A, double& x, double& kx, int& nx)
      {
        nx = lround(xu / A);
        x = xu - nx * A;
        if (nx % 2 != 0) {
          x = -x;
          kx = -kx;
        }
      }

      /**
       * Performs a clip on x w.r.t xmi and xma
       * @param x true coordinate
       * @param Nx signed number of reflections
       * @param A size for folding/unfolding
       * @param xmi lower limit on unfolded coordinate x
       * @param xma upper limit on unfolded coordinate x
       * @return clipped x
       */
      inline double clip(double x, int Nx, double A, double xmi, double xma)
      {
        x = unfold(x, Nx, A);
        x = std::max(std::min(x, xma), xmi) - Nx * A;
        if (Nx % 2 != 0) x = -x;
        return x;
      }

      /**
       * Returns number of even numbers in the range given by arguments
       * @param j1 first number
       * @param j2 last number (exclusive)
       * @return number of even numbers
       */
      inline int getNumOfEven(int j1, int j2)
      {
        return (j2 - j1 + 1 - std::abs(j1) % 2) / 2;
      }

      /**
       * Returns angle within 0 and 2PI
       * @param angle angle
       * @return angle within 0 and 2PI
       */
      inline double within2PI(double angle)
      {
        angle = fmod(angle, 2 * M_PI);
        if (angle < 0) angle += 2 * M_PI;
        return angle;
      }

      /**
       * Replacement for a function TVector3::RotateUz which is not implemented in GenVector classes.
       * @param vec vector to be rotated from frame S' [in] to frame S [out]
       * @param z_Axis vector representing z-axis of the frame S' expressed in the coordinates of frame S
       */
      inline void rotateUz(ROOT::Math::XYZVector& vec, const ROOT::Math::XYZVector& z_Axis)
      {
        auto zAxis = z_Axis.Unit();
        double cth = zAxis.Z();
        double sth = sqrt(1 - cth * cth);
        if (sth == 0) {
          if (cth < 0) vec = -vec;
          return;
        }
        double cfi = zAxis.X() / sth;
        double sfi = zAxis.Y() / sth;
        // rotation by theta around y then by phi around z
        double x = cth * vec.X() + sth * vec.Z();
        double y = vec.Y();
        double z = -sth * vec.X() + cth * vec.Z();
        vec.SetX(cfi * x - sfi * y);
        vec.SetY(sfi * x + cfi * y);
        vec.SetZ(z);
      }


    } // func
  } // TOP
} // Belle2
