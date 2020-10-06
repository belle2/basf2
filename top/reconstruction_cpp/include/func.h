/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

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


    } // func
  } // TOP
} // Belle2

