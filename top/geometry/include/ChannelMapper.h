/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>
#include <top/dbobjects/TOPChannelMap.h>

namespace Belle2 {
  namespace TOP {

    /**
     * Provides mapping between electronic channels and pixels
     */
    class ChannelMapper {

    public:
      /**
       * Enum for the number of different quantities etc.
       */
      enum {c_numBoardstacks = 4,   /**< number of boardstacks per TOP module */
            c_numCarrierBoards = 4, /**< number of carrier boards per boardstack */
            c_numAsics = 4,         /**< number of ASIC's per carrier board */
            c_numChannels = 8,      /**< number of channels per ASIC */
            c_numRows = 2,          /**< number of pixel rows per carrier board */
            c_numColumns = 16,      /**< number of pixel columns per carrier board */
            c_numPixelRows = c_numRows * c_numCarrierBoards, /**< per module */
            c_numPixelColumns = c_numColumns * c_numBoardstacks, /**< per module */
            c_numPixels = c_numPixelRows * c_numPixelColumns, /**< per module */
            c_invalidChannelID = c_numPixels, /**< invalid channel ID value */
            c_invalidPixelID = 0 /**< invalid pixel ID value */
           };

      /**
       * constructor
       */
      ChannelMapper();

      /**
       * destructor
       */
      ~ChannelMapper();

      /**
       * Initialize: get mapping from Gearbox
       * @param channelMapping xpath to the mapping
       */
      void initialize(const GearDir& channelMapping);

      /**
       * Checks if mapping is available
       * @return true if available
       */
      bool isAvailable() const {return m_available;}

      /**
       * Checks validity of pixel ID
       * @param pixel pixel ID (1-based)
       * @return true for valid ID
       */
      bool isPixelIDValid(int pixel) const
      {
        unsigned pix = pixel - 1;
        return pix < c_numPixels;
      }

      /**
       * Checks validity of hardware channel ID
       * @param channel hardware channel ID (0-based)
       * @return true for valid ID
       */
      bool isChannelIDValid(unsigned channel) const
      {
        return channel < c_numPixels;
      }

      /**
       * Converts pixel to hardware channel ID (0-based)
       * @param pixel pixel ID (1-based)
       * @return channel ID (or c_invalidChannelID for invalid pixel)
       */
      unsigned getChannelID(int pixel) const;

      /**
       * Splits hardware channel ID into boardstack#, carrier#, asic# and asic channel#
       * @param channel hardware channel ID (0-based) [input]
       * @param boardstack boardstack number (0-based) [output]
       * @param carrier carrier board number (0-based) [output]
       * @param asic ASIC number (0-based) [output]
       * @param chan ASIC channel number (0-based) [output]
       */
      void splitChannelID(unsigned channel,
                          unsigned& boardstack,
                          unsigned& carrier,
                          unsigned& asic,
                          unsigned& chan) const;

      /**
       * Converts hardware channel ID to pixel ID (1-based)
       * @param channel hardware channel ID (0-based)
       * @return pixel ID (or c_invalidPixelID for invalid channel)
       */
      int getPixelID(unsigned channel) const;

      /**
       * Print mappings to terminal screen
       * @param type electronic type
       */
      void print(std::string type = "") const;

      /**
       * test that the conversion and inverse of it gives identity, if not B2ERROR
       */
      void test() const;

    private:

      std::vector<TOPChannelMap> m_mapping; /**< mappings from xml file */
      bool m_available = false; /**< true if mapping available */

      const TOPChannelMap* m_channels[c_numRows][c_numColumns]; /**< conversion array */
      const TOPChannelMap* m_pixels[c_numAsics][c_numChannels]; /**< conversion array */

    };

  } // TOP namespace
} // Belle2 namespace

