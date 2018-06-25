/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/dataobjects/EventMetaData.h>
#include <string>

namespace Belle2 {

  class EventMetaDataSerialization {
  public:
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
  };
}