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
            c_invalidChannel = c_numPixels, /**< value of invalid channel number */
            c_invalidPixelID = 0 /**< value of invalid pixel ID */
           };

      /**
       * Enum for electornic types
       */
      enum EType {c_unknown = 0,
                  c_IRS3B   = 1,
                  c_IRSX    = 2
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
       * Return electornic type (see enum)
       * @return type
       */
      EType getType() const {return m_type;}

      /**
       * Return electornic name
       * @return name
       */
      std::string getName() const {return m_typeName;}

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
       * Checks validity of hardware channel number
       * @param channel hardware channel number (0-based)
       * @return true for valid ID
       */
      bool isChannelValid(unsigned channel) const
      {
        return channel < c_numPixels;
      }

      /**
       * Converts pixel to hardware channel number (0-based)
       * @param pixel pixel ID (1-based)
       * @return channel number (or c_invalidChannel for invalid pixel)
       */
      unsigned getChannel(int pixel) const;

      /**
       * Returns hardware channel number (0-based)
       * @param boardstack boardstack number (0-based)
       * @param carrier carrier board number (0-based)
       * @param asic ASIC number (0-based)
       * @param chan ASIC channel number (0-based)
       * @return channel number (or c_invalidChannel for invalid pixel)
       */
      unsigned getChannel(unsigned boardstack,
                          unsigned carrier,
                          unsigned asic,
                          unsigned chan) const
      {
        return chan + c_numChannels * (asic + c_numAsics *
                                       (carrier + c_numCarrierBoards * boardstack));
      }

      /**
       * Splits hardware channel number into boardstack, carrier, asic and asic channel
       * @param channel hardware channel number (0-based) [input]
       * @param boardstack boardstack number (0-based) [output]
       * @param carrier carrier board number (0-based) [output]
       * @param asic ASIC number (0-based) [output]
       * @param chan ASIC channel number (0-based) [output]
       */
      void splitChannelNumber(unsigned channel,
                              unsigned& boardstack,
                              unsigned& carrier,
                              unsigned& asic,
                              unsigned& chan) const;

      /**
       * Converts hardware channel number to pixel ID (1-based)
       * @param channel hardware channel number (0-based)
       * @return pixel ID (or c_invalidPixelID for invalid channel number)
       */
      int getPixelID(unsigned channel) const;

      /**
       * Print mappings to terminal screen
       * @param type electronic type
       */
      void print() const;

      /**
       * test that the conversion and inverse of it gives identity, if not B2ERROR
       */
      void test() const;

    private:

      std::string m_typeName;    /**< electronic type name */
      EType m_type = c_unknown;  /**< electornic type number */
      bool m_available = false; /**< true if mapping available */
      std::vector<TOPChannelMap> m_mapping; /**< mappings from xml file */

      const TOPChannelMap* m_channels[c_numRows][c_numColumns]; /**< conversion array */
      const TOPChannelMap* m_pixels[c_numAsics][c_numChannels]; /**< conversion array */

    };

  } // TOP namespace
} // Belle2 namespace

