//
// Created by abaur on 28.05.18.
//

#pragma once

#include <chrono>
#include <string>
#include <framework/logging/LogMethod.h>

namespace Belle2 {

  class UniqueEventId {
  public:
    UniqueEventId(int event, int run, int experiment, std::chrono::time_point<std::chrono::system_clock> timestamp, int worker = -1)
    {
      m_event = event;
      m_run = run;
      m_experiment = experiment;
      m_timestamp = timestamp;
      m_worker = worker;
    }

    UniqueEventId(std::string stream)
    {
      size_t pos = stream.find(':');
      m_event = atoi(stream.substr(0, pos).c_str());
      stream.erase(0, pos + 1);
      pos = stream.find(':');
      m_run = atoi(stream.substr(0, pos).c_str());
      stream.erase(0, pos + 1);
      pos = stream.find(':');
      m_experiment = atoi(stream.substr(0, pos).c_str());
      stream.erase(0, pos + 1);
      m_timestamp = std::chrono::system_clock::now();//atoi(stream.c_str());

    }


    int getEvt() const {return m_event;}
    int getRun() const {return m_run;}
    int getExperiment() const {return m_experiment;}
    std::chrono::time_point<std::chrono::system_clock> getTimestamp() const {return m_timestamp;}
    int getWorker() const {return m_worker;}

    std::string getSerial() const
    {
      return std::to_string(m_event) + ":" + std::to_string(m_run)
             + ":" + std::to_string(m_experiment) + ":" + std::to_string(m_timestamp.time_since_epoch().count());
    }


    // operators for map
    bool operator< (const UniqueEventId& evtId) const
    {
      if (this->m_event < evtId.m_event
          || (this->m_event == evtId.m_event && (this->m_run < evtId.m_run
                                                 || (this->m_run == evtId.m_run && (this->m_experiment < evtId.m_experiment))))) {
        return true;
      } else {return false;}
    }
    bool operator== (const UniqueEventId& evtId) const
    {
      if (this->m_event == evtId.m_event
          && this->m_run == evtId.m_run
          && this->m_experiment == evtId.m_experiment) {
        return  true;
      } else {return false;}
    }

  private:
    unsigned int m_event;
    unsigned int m_run;
    unsigned int m_experiment;
    std::chrono::time_point<std::chrono::system_clock> m_timestamp;
    int m_worker = -1;
  };
}
