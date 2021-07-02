/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/database/IntervalOfValidity.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>
#include <utility>

using namespace Belle2;


IntervalOfValidity::IntervalOfValidity(int experimentLow, int runLow, int experimentHigh,
                                       int runHigh) : m_experimentLow(experimentLow), m_runLow(runLow), m_experimentHigh(experimentHigh), m_runHigh(runHigh)
{
  // consistency checks
  if ((experimentLow < 0) && (runLow >= 0)) {
    B2ERROR("A run number of " << runLow << " is given for an undefined low experiment number. Setting run to undefined, too.");
    m_runLow = -1;
  }
  if ((experimentHigh < 0) && (runHigh >= 0)) {
    B2ERROR("A run number of " << runHigh << " is given for an undefined high experiment number. Setting run to undefined, too.");
    m_runHigh = -1;
  }
  if ((experimentLow >= 0) && (experimentHigh >= 0)) {
    if ((experimentLow > experimentHigh) || ((experimentLow == experimentHigh) && (runHigh >= 0) && (runLow > runHigh))) {
      B2ERROR("The given lower and higher experiment/run numbers of " << experimentLow << "/" << runLow << " and " << experimentHigh <<
              "/" << runHigh << ", respectively, are in the wrong order, Swapping them.");
      std::swap(m_experimentLow, m_experimentHigh);
      std::swap(m_runLow, m_runHigh);
    }
  }
}


int IntervalOfValidity::checkLowerBound(int experiment, int run) const
{
  // check for empty interval
  if (empty()) return false;

  if ((m_experimentLow == experiment) && (m_runLow == run)) return 0;
  if (m_experimentLow < 0) return 1;
  if (experiment < m_experimentLow) return -1;
  if (experiment == m_experimentLow) {
    if (run < m_runLow) return -1;
  }
  return 1;
}

int IntervalOfValidity::checkUpperBound(int experiment, int run) const
{
  // check for empty interval
  if (empty()) return false;

  if ((m_experimentHigh == experiment) && (m_runHigh == run)) return 0;
  if (m_experimentHigh < 0) return -1;
  if ((experiment < 0) || (experiment > m_experimentHigh)) return 1;
  if (experiment == m_experimentHigh) {
    if ((m_runHigh >= 0) && ((run > m_runHigh) || (run < 0))) return 1;
  }
  return -1;
}

void IntervalOfValidity::makeValid()
{
  if (m_experimentLow < 0) m_runLow = -1;
  if (m_experimentHigh < 0) m_runHigh = -1;
  if ((m_experimentLow >= 0) && (m_experimentHigh >= 0)) {
    if ((m_experimentLow > m_experimentHigh) || ((m_experimentLow == m_experimentHigh) && (m_runHigh >= 0) && (m_runLow > m_runHigh))) {
      m_experimentLow = m_runLow = m_experimentHigh = m_runHigh = -1;
    }
  }
}


bool IntervalOfValidity::contains(const EventMetaData& event) const
{
  auto experiment = (int) event.getExperiment();
  auto run = (int) event.getRun();

  // check for empty interval
  if (empty()) return false;

  // check lower bound
  if (checkLowerBound(experiment, run) < 0) return false;
  if (checkUpperBound(experiment, run) > 0) return false;
  return true;
}

IntervalOfValidity IntervalOfValidity::overlap(const IntervalOfValidity& iov) const
{
  if (empty() || iov.empty()) return IntervalOfValidity();

  IntervalOfValidity result(*this);
  if (checkLowerBound(iov.m_experimentLow, iov.m_runLow) > 0) {
    result.m_experimentLow = iov.m_experimentLow;
    result.m_runLow = iov.m_runLow;
  }
  if (checkUpperBound(iov.m_experimentHigh, iov.m_runHigh) < 0) {
    result.m_experimentHigh = iov.m_experimentHigh;
    result.m_runHigh = iov.m_runHigh;
  }

  result.makeValid();
  return result;
}

bool IntervalOfValidity::trimOverlap(IntervalOfValidity& iov, bool trimOlder)
{
  if (!overlaps(iov)) return true;

  bool thisOlder = checkLowerBound(iov.m_experimentLow, iov.m_runLow) >= 0;
  IntervalOfValidity& older = (thisOlder) ? *this : iov;
  IntervalOfValidity& younger = (thisOlder) ? iov : *this;

  if (trimOlder) {
    // check for the case where trimming is not possible because the interval would be split
    if (older.checkUpperBound(younger.m_experimentHigh, younger.m_runHigh) < 0) return false;

    older.m_experimentHigh = younger.m_experimentLow;
    older.m_runHigh = younger.m_runLow - 1;
    if (older.m_runHigh < 0) {
      older.m_experimentHigh--;
      older.m_runHigh = -1;
    }
    older.makeValid();
  } else {
    younger.m_experimentLow = older.m_experimentHigh;
    younger.m_runLow = older.m_runHigh + 1;
    if (younger.m_runLow == 0) {
      younger.m_experimentLow++;
    }
    younger.makeValid();
  }

  return true;
}


namespace Belle2 {

  std::istream& operator>> (std::istream& input, IntervalOfValidity& iov)
  {
    iov = IntervalOfValidity();
    if (!input.good()) {
      throw std::runtime_error("cannot read from stream");
    }

    std::string str[4];
    int index = 0;
    while (input.good()) {
      auto c = input.peek();
      if (c == EOF) break;
      if (((c == ' ') || (c == '\n') || (c == '\t'))) {
        //ignore whitespace in the beginning
        if (index == 0 && str[0].empty()) {
          input.get();
          continue;
        }
        //and stop parsing otherwise
        break;
      }
      if (c == ',') {
        index++;
        if (index == 4) break;
      } else {
        str[index] += c;
      }
      input.get();
    }
    if (index != 3) {
      throw std::invalid_argument("IoV needs to be four values (firstExp,firstRun,finalExp,finalRun)");
    }
    try {
      iov.m_experimentLow = stoi(str[0]);
      iov.m_runLow = stoi(str[1]);
      iov.m_experimentHigh = stoi(str[2]);
      iov.m_runHigh = stoi(str[3]);
    } catch (std::invalid_argument& e) {
      throw std::invalid_argument("experiment and run numbers must be integers");
    }

    return input;
  }

  std::ostream& operator<< (std::ostream& output, const IntervalOfValidity& iov)
  {
    output << iov.m_experimentLow << "," << iov.m_runLow << "," << iov.m_experimentHigh << "," << iov.m_runHigh;

    return output;
  }

}
