/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <variant>
#include <deque>
#include <chrono>
#include <map>
#include <numeric>
#include <string>

namespace Belle2 {
  /**
   * Base class for the ZMQ connection to help monitor
   * and log certain values. Which and when to monitor/log must be defined
   * by the derived classes, this base class only gives helpers
   * to store/add/average/measure time and most importantly
   * convert all stored monitoring values to a JSON string
   * on request (useful e.g. for monitoring purposes).
   *
   * Please note, that in all cases you need to call the corresponding monitor function
   * by yourself. There is no "registering" of monitor values, values will only be stored
   * on actively calling a monitor function (e.g. log).
   */
  class ZMQLogger {
  public:
    /// Convert the stored monitoring values to a JSON string ready for sending out via a message.
    virtual std::string getMonitoringJSON() const;

    /// Store a value under a certain key. Different types of values can be stored, namely long, double or string. Mixtures are not allowed for a given key.
    template<class AClass>
    void log(const std::string& key, const AClass& value);
    /// Increment the value with the given key (only numerical values). If not present, set to 1.
    void increment(const std::string& key);
    /// Decrement the value with the given key (only numerical values). If not present, set to -1.
    void decrement(const std::string& key);
    /// Instead of storeing the double value directly under the given key, store the average of the last MAX_SIZE values.
    template<size_t MAX_SIZE = 100>
    void average(const std::string& key, double value);
    /// Store the information how much time has elapsed since the last call to this function (with the same key) and store the rate (averaged over MAX_SIZE calls)
    template<size_t AVERAGE_SIZE = 2000>
    void timeit(const std::string& key);
    /// Store the current time as a string under the given key
    void logTime(const std::string& key);

  private:
    /// Internal storage of all stored values
    std::map<std::string, std::variant<long, double, std::string>> m_monitoring;
    /// Internal storage of the previous values when calculating averages
    std::map<std::string, std::deque<double>> m_averages;
    /// Internal storage how often the timeit function for a given key was called
    std::map<std::string, unsigned long> m_eventNumber;
    /// Time of the last call to timeit for the given key, when it has reached MAX_SIZE
    std::map<std::string, std::chrono::system_clock::time_point> m_timestamp;

    /// Visitor Helper for converting a variant value into a JSON string
    struct toJSON {
      /// just stringify longs
      std::string operator()(long value);
      /// just stringify doubles
      std::string operator()(double value);
      /// escape strings if needed and surround by "
      std::string operator()(const std::string& value);
    };

    /// Visitor Helper for incrementing a numerical value
    struct Incrementor {
      /// +1 for longs
      void operator()(long& value);
      /// +1 for doubles
      void operator()(double& value);
      /// Makes no sense here, just leave it as it is
      void operator()(std::string&);
    };

    /// Visitor Helper for decrementing a numerical value
    struct Decrementor {
      /// -1 for longs
      void operator()(long& value);
      /// -1 for doubles
      void operator()(double& value);
      /// Makes no sense here, just leave it as it is
      void operator()(std::string&);
    };
  };

  template<class AClass>
  void ZMQLogger::log(const std::string& key, const AClass& value)
  {
    m_monitoring[key] = value;
  }

  template<size_t MAX_SIZE>
  void ZMQLogger::average(const std::string& key, double value)
  {
    m_averages[key].push_back(value);
    if (m_averages[key].size() > MAX_SIZE) {
      m_averages[key].pop_front();
    }
    log(key, std::accumulate(m_averages[key].begin(), m_averages[key].end(), 0.0) / m_averages[key].size());
  }

  template<size_t AVERAGE_SIZE>
  void ZMQLogger::timeit(const std::string& key)
  {
    if (m_eventNumber[key] % AVERAGE_SIZE == 0) {
      auto current = std::chrono::system_clock::now();
      double elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(
                         current - m_timestamp[key]).count();

      log(key, AVERAGE_SIZE / elapsed);

      m_timestamp[key] = current;
      m_eventNumber[key] = 0;

      auto displayTime = std::chrono::system_clock::to_time_t(current);
      log(key + "_last_measurement", std::ctime(&displayTime));
    }
    m_eventNumber[key]++;
  }
}
