/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/dataobjects/EventMetaData.h>
#include <string>

namespace Belle2 {

  /// Helper class for (de)serializing the event meta data used for the event backup transferral
  class EventMetaDataSerialization {
  public:
    /// Deserialize the event data from a string
    static EventMetaData deserialize(std::string stream)
    {
      size_t pos = stream.find(':');
      int event = atoi(stream.substr(0, pos).c_str());
      stream.erase(0, pos + 1);
      pos = stream.find(':');
      int run = atoi(stream.substr(0, pos).c_str());
      stream.erase(0, pos + 1);
      pos = stream.find(':');
      int experiment = atoi(stream.substr(0, pos).c_str());
      return EventMetaData(event, run, experiment);
    }

    /// Serialize the event meta data to a string
    static std::string serialize(const EventMetaData& eventMetaData)
    {
      return std::to_string(eventMetaData.getEvent()) + ":" +
             std::to_string(eventMetaData.getRun()) + ":" +
             std::to_string(eventMetaData.getExperiment());
    }
  };
}