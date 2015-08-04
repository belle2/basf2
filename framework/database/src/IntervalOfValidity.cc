/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
      std::swap(experimentLow, experimentHigh);
      std::swap(runLow, runHigh);
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


bool IntervalOfValidity::contains(const EventMetaData& event) const
{
  int experiment = (int) event.getExperiment();
  int run = (int) event.getRun();

  // check for empty interval
  if (empty()) return false;

  // check lower bound
  if (checkLowerBound(experiment, run) < 0) return false;
  if (checkUpperBound(experiment, run) > 0) return false;
  return true;
}

IntervalOfValidity IntervalOfValidity::overlap(const IntervalOfValidity& iov) const
{
  IntervalOfValidity emptyResult;
  if (empty() || iov.empty()) return emptyResult;

  IntervalOfValidity result(*this);
  if (checkLowerBound(iov.m_experimentLow, iov.m_runLow) > 0) {
    result.m_experimentLow = iov.m_experimentLow;
    result.m_runLow = iov.m_runLow;
  }
  if (checkUpperBound(iov.m_experimentHigh, iov.m_runHigh) < 0) {
    result.m_experimentHigh = iov.m_experimentHigh;
    result.m_runHigh = iov.m_runHigh;
  }

  // check for no overlap
  int runLow = result.m_runLow;
  if (runLow < 0) runLow = 0;
  if (result.checkUpperBound(result.m_experimentLow, runLow) > 0) return emptyResult;

  return result;
}


namespace Belle2 {

  std::istream& operator>> (std::istream& input, IntervalOfValidity& iov)
  {
    iov = IntervalOfValidity();
    if (input.eof()) return input;

    char c;
    std::string str[4];
    int index = 0;
    while (!input.eof()) {
      c = input.get();
      if ((index == 0) && (c == ' ')) continue;
      if ((index == 3) && ((c == ' ') || (c == '\n'))) break;
      if (c == ',') {
        index++;
        if (index == 4) break;
      } else {
        str[index] += c;
      }
    }
    iov.m_experimentLow = stoi(str[0]);
    iov.m_runLow = stoi(str[1]);
    iov.m_experimentHigh = stoi(str[2]);
    iov.m_runHigh = stoi(str[3]);

    return input;
  }

  std::ostream& operator<< (std::ostream& output, const IntervalOfValidity& iov)
  {
    output << iov.m_experimentLow << "," << iov.m_runLow << "," << iov.m_experimentHigh << "," << iov.m_runHigh;

    return output;
  }

}
