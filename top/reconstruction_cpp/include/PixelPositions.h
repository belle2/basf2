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
     * Pixel positions and dimensions in module local frame
     */
    class PixelPositions {

    public:

      /**
       * position and size of a pixel
       */
      struct PixelData {
        double xc = 0; /**< position of center in x */
        double yc = 0; /**< position of center in y */
        double Dx = 0; /**< size in x */
        double Dy = 0; /**< size in y */
        int ID = 0; /**< pixel ID */
        unsigned pmtType = 0; /**< PMT type (see TOPPmtObsoleteData::EType for the defined types) */

        /**
         * default constructor
         */
        PixelData()
        {}

        /**
         * almost full constructor
         * @param x position of center in x
         * @param y position of center in y
         * @param a size in x
         * @param b size in y
         */
        PixelData(double x, double y, double a, double b):
          xc(x), yc(y), Dx(a), Dy(b)
        {}
      };


      /**
       * Class constructor
       * @param moduleID slot ID
       */
      explicit PixelPositions(int moduleID);

      /**
       * Returns slot ID
       * @return slot ID
       */
      int getModuleID() const {return m_moduleID;}

      /**
       * Returns number of pixels
       * @return number of pixels
       */
      unsigned getNumPixels() const {return m_pixels.size();}

      /**
       * Returns the number of pixel rows
       * @return number of pixel rows
       */
      unsigned getNumPixelRows() const {return m_NRows;}

      /**
       * Returns the number of pixel columns
       * @return number of pixel columns
       */
      unsigned getNumPixelColumns() const {return m_NColumns;}

      /**
       * Returns pixel data for given pixelID
       * @param pixelID pixel ID (1-based)
       * @return pixel data
       */
      const PixelData& get(int pixelID) const;

      /**
       * Returns pixel data of entire module
       * @return vector of pixel data
       */
      const std::vector<PixelData>& getPixels() const {return m_pixels;}

      /**
       * Transforms pixel row and column to pixel ID
       * Note: for convenience pixel row and column numbering starts with 0 here!
       * @param row pixel row (0-based)
       * @param col pixel column (0-based)
       * @return pixelID (1-based)
       */
      int pixelID(unsigned row, unsigned col) const {return col + row * m_NColumns + 1;}

    private:

      int m_moduleID = 0; /**< slot ID */
      unsigned m_NRows = 0; /**< number of pixel rows */
      unsigned m_NColumns = 0; /**< number of pixel columns */
      std::vector<PixelData> m_pixels; /**< pixel positions and sizes, index = pixelID - 1 */
      PixelData m_invalid; /**< invalid pixel data */

    };

    //--- inline functions ------------------------------------------------------------

    inline const PixelPositions::PixelData& PixelPositions::get(int pixelID) const
    {
      unsigned k = pixelID - 1;
      if (k < m_pixels.size()) return m_pixels[k];
      return m_invalid;
    }

  } // namespace TOP
} // namespace Belle2
