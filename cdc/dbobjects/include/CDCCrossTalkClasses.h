/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

// Convenience classes used by CDCCrossTalkLibrary DB object

using std::array;

namespace Belle2 {

  /// record to be used to store ASIC info
  struct asicChannel {
    Short_t TDC;  /**< TDC info  */
    Short_t ADC;  /**< ADC info  */
    Short_t TOT;  /**< Time over threshold */
  };

  /// fixed sized array of ASIC channels
  typedef array<asicChannel, 8> asicChannels;

  /// pair ADC, channel
  struct adcChannelPair {
    Short_t ADC;      /**< ADC info */
    Short_t Channel;  /**< Channel number info (between 0 and 8) */
  };

  /// tuple to store ADC,Channel -> 8 asicChannels
  struct adcAsicTuple {
    Short_t ADC;     /**< ADC info */
    Short_t Channel; /**< Channel number info (between 0 and 8) */
    asicChannels record; /**< 8-channel record */
  };


  /// functions to search in the sorted list of tuples
  struct adc_search {
    /// Order accroding to channel/ADC value
    bool operator()(const adcAsicTuple& lhs, const adcAsicTuple& rhs)
    {
      if (lhs.Channel == rhs.Channel) {
        return lhs.ADC < rhs.ADC;
      } else {
        return lhs.Channel < rhs.Channel;
      }
    }

    /// Order accroding to channel/ADC value
    bool operator()(const adcChannelPair& lhs, const adcAsicTuple& rhs)
    {
      if (lhs.Channel == rhs.Channel) {
        return lhs.ADC < rhs.ADC;
      } else {
        return lhs.Channel < rhs.Channel;
      }
    }

    /// Order accroding to channel/ADC value
    bool operator()(const adcAsicTuple& lhs, const adcChannelPair& rhs)
    {
      if (lhs.Channel == rhs.Channel) {
        return lhs.ADC < rhs.ADC;
      } else {
        return lhs.Channel < rhs.Channel;
      }
    }
  };
} // end namespace Belle2
