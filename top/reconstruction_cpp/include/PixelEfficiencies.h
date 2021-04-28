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

namespace Belle2 {
  namespace TOP {

    /**
     * Pixel relative efficiencies of a single module
     */
    class PixelEfficiencies {

    public:

      /**
       * Constructor: all relative efficiencies are set to 1
       * @param moduleID slot ID
       */
      explicit PixelEfficiencies(int moduleID);

      /**
       * Sets relative efficiency for a given pixel
       * @param pixelID pixel ID (1-based)
       * @param value relative efficiency
       */
      void set(int pixelID, double value);

      /**
       * Returns slot ID
       * @return slot ID
       */
      int getModuleID() const {return m_moduleID;}

      /**
       * Returns number of pixels
       * @return number of pixels
       */
      unsigned getNumPixels() const {return m_efficiencies.size();}

      /**
       * Returns pixel relative efficinecy
       * @param pixelID pixel ID (1-based)
       * @return relative efficiency
       */
      double get(int pixelID) const;

      /**
       * Returns pixel relative efficiencies of entire module
       * @return vector of relative efficiencies
       */
      const std::vector<double>& getEfficiencies() const {return m_efficiencies;}

    private:

      int m_moduleID = 0; /**< slot ID */
      std::vector<double> m_efficiencies; /**< pixel relative efficiencies, index = pixelID - 1 */

    };

    //--- inline functions ------------------------------------------------------------

    inline void PixelEfficiencies::set(int pixelID, double value)
    {
      unsigned k = pixelID - 1;
      if (k < m_efficiencies.size()) m_efficiencies[k] = value;
    }

    inline double PixelEfficiencies::get(int pixelID) const
    {
      unsigned k = pixelID - 1;
      if (k < m_efficiencies.size()) return m_efficiencies[k];
      return 0;
    }

  } // namespace TOP
} // namespace Belle2



