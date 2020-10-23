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

#include <vector>
#include <cmath>

namespace Belle2 {
  namespace TOP {

    /**
     * A mask for energy masking
     */
    class EnergyMask {

    public:

      /**
       * Constructor
       * @param dyde derivative dy/de [cm/eV]
       * @param dydL derivative dy/dL
       * @param dydx derivative dy/dx
       * @param dy effective PMT pad size in y [cm]
       * @param dL length of particle trajectory within the quartz [cm]
       * @param dx PMT pad size in x [cm]
       * @param dE energy step [eV]
       */
      EnergyMask(double dyde, double dydL, double dydx, double dy, double dL, double dx, double dE);

      /**
       * Sets minimal size of discrete mask
       * @param Nmin minimal size
       */
      static void setNMin(unsigned Nmin) {s_Nmin = Nmin;}

      /**
       * Sets maximal size of discrete mask
       * @param Nmax maximal size
       */
      static void setNMax(unsigned Nmax) {s_Nmax = Nmax;}

      /**
       * Returns energy step size of the discrete mask
       * @return step [eV]
       */
      double getStep() const {return m_dE;}

      /**
       * Returns mask full width
       * @return full width [eV]
       */
      double getFullWidth() const {return m_A + m_B + m_C;}

      /**
       * Returns discrete mask (note: only half of the mask is stored)
       * @return discrete mask
       */
      const std::vector<double>& getMask() const {return m_mask;}

      /**
       * Returns mask value at given energy difference by calling direct calculation of the convolution
       * @param E photon energy difference w.r.t center of mask [eV]
       * @return mask value
       */
      double getMask(double E) const;

      /**
       * Returns mask value at given index from stored discrete mask.
       * If discrete mask is empty it calls direct calculation of convolution.
       * @param i index (all values are valid)
       * @return mask value
       */
      double getMask(int i) const;

      /**
       * Returns mask value with linear interpolation between stored discrete mask elements.
       * If discrete mask is empty it calls direct calculation of convolution.
       * @param i index (all values are valid)
       * @param fract fractional part for linear interpolation (values btw. -0.5 and 0.5)
       * @return mask value
       */
      double getMask(int i, double fract) const;

      /**
       * Returns energy difference w.r.t center of mask
       * @param i index (all values are valid)
       * @return energy difference w.r.t center of mask [eV]
       */
      double getEnergyDifference(int i) {return i * m_dE;}

    private:

      /**
       * Returns a value of convolution of two square distributions at given photon energy
       * using the largest two of full widths (m_A, m_B).
       * @param E photon energy difference w.r.t center of mask [eV]
       * @return convolution value
       */
      double twoSquareConvolution(double E) const;

      /**
       * Returns a value of convolution of three square distributions at given photon energy.
       * @param E photon energy difference w.r.t center of mask [eV]
       * @return convolution value
       */
      double threeSquareConvolution(double E) const;

      /**
       * Returns mask value at given index from stored discrete mask.
       * @param i index (all values are valid)
       * @return mask value or 0 if index is out of array range
       */
      double mask(int i) const;

      double m_dE = 0; /**< energy step */
      double m_Wy = 0; /**< enegy full width of dy */
      double m_A = 0;  /**< the largest energy full width */
      double m_B = 0;  /**< the middle energy full width */
      double m_C = 0;  /**< the smallset energy full width */
      std::vector<double> m_mask; /**< discrete mask (half of) */

      static unsigned s_Nmin; /**< minimal mask size */
      static unsigned s_Nmax; /**< maximal mask size */

    };


    inline double EnergyMask::mask(int i) const
    {
      unsigned k = abs(i);
      if (k < m_mask.size()) return m_mask[k];
      return 0;
    }


  } // namespace TOP
} // namespace Belle2
