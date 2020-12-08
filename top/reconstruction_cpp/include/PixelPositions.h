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
     * Pixel positions and sizes in module local frame
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
        unsigned pmtType = 0; /**< PMT type (see TOPPmtObsoleteData::EType for the defined types) */

        /**
         * default constructor
         */
        PixelData()
        {}

        /**
         * full constructor
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
       * Class default constructor
       */
      PixelPositions()
      {}

      /**
       * Class full constructor
       * @param moduleID slot ID
       */
      explicit PixelPositions(int moduleID);

      /**
       * Returns slot ID
       * @return slot ID
       */
      int getModuleID() const {return m_moduleID;}

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
       * Returns pixel data of a given pixelID
       * @param pixelID pixel ID (1-based)
       */
      const PixelData& get(int pixelID) const;

      /**
       * Returns pixel data of a given pixel row and column
       * Note: for convenience pixel row and column numbering starts with 0 here!
       * @param row pixel row (0-based)
       * @param col pixel column (0-based)
       */
      const PixelData& get(unsigned row, unsigned col) const;

    private:

      int m_moduleID = 0; /**< slot ID */
      unsigned m_NRows = 0; /**< number of pixel rows */
      unsigned m_NColumns = 0; /**< number of pixel columns */
      std::vector<PixelData> m_pixels; /**< pixel positions and sizes, index = pixelID - 1 */
      PixelData m_invalid; /**< invalid pixel data */

    };

  } // namespace TOP
} // namespace Belle2
