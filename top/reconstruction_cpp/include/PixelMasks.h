/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <vector>

namespace Belle2 {
  namespace TOP {

    /**
     * Pixel masks of a single module
     */
    class PixelMasks {

    public:

      /**
       * Constructor: all pixels are turned on (active)
       * @param moduleID slot ID
       */
      explicit PixelMasks(int moduleID);

      /**
       * Sets mask value for a given pixel
       * @param pixelID pixel ID (1-based)
       * @param value true to set it active, false to turn it off
       */
      void set(int pixelID, bool value);

      /**
       * Turns given pixel off
       * @param pixelID pixel ID (1-based)
       */
      void setPixelOff(int pixelID) {set(pixelID, false);}

      /**
       * Returns slot ID
       * @return slot ID
       */
      int getModuleID() const {return m_moduleID;}

      /**
       * Returns number of pixels
       * @return number of pixels
       */
      unsigned getNumPixels() const {return m_masks.size();}

      /**
       * Checks if pixel is active
       * @param pixelID pixel ID (1-based)
       * @return true if active
       */
      bool isActive(int pixelID) const;

      /**
       * Returns pixel masks of entire module
       * @return vector of pixel masks
       */
      const std::vector<bool>& getMasks() const {return m_masks;}

    private:

      int m_moduleID = 0; /**< slot ID */
      std::vector<bool> m_masks; /**< pixel masks (true for active), index = pixelID -1 */

    };

    //--- inline functions ------------------------------------------------------------

    inline void PixelMasks::set(int pixelID, bool value)
    {
      unsigned k = pixelID - 1;
      if (k < m_masks.size()) m_masks[k] = value;
    }

    inline bool PixelMasks::isActive(int pixelID) const
    {
      unsigned k = pixelID - 1;
      if (k < m_masks.size()) return m_masks[k];
      return false;
    }

  } // namespace TOP
} // namespace Belle2


