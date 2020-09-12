/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tracking, CDC group                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
