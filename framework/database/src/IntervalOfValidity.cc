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


IntervalOfValidity::IntervalOfValidity(unsigned long experimentLow, unsigned long runLow, unsigned long experimentHigh,
                                       unsigned long runHigh) : m_experimentLow(experimentLow), m_runLow(runLow), m_experimentHigh(experimentHigh), m_runHigh(runHigh)
{
  // consistency checks
  if (!experimentLow && runLow) {
    B2ERROR("A run number of " << runLow << " is given for an undefined low experiment number. Setting run to undefined, too.");
    m_runLow = 0;
  }
  if (!experimentHigh && runHigh) {
    B2ERROR("A run number of " << runHigh << " is given for an undefined high experiment number. Setting run to undefined, too.");
    m_runHigh = 0;
  }
  if (experimentLow && experimentHigh) {
    if ((experimentLow > experimentHigh) || ((experimentLow == experimentHigh) && runHigh && (runLow > runHigh))) {
      B2ERROR("The given lower and higher experiment/run numbers of " << experimentLow << "/" << runLow << " and " << experimentHigh <<
              "/" << runHigh << ", respectively, are in the wrong order, Swapping them.");
      std::swap(experimentLow, experimentHigh);
      std::swap(runLow, runHigh);
    }
  }
}

bool IntervalOfValidity::contains(const EventMetaData& event) const
{
  unsigned long experiment = event.getExperiment();
  unsigned long run = event.getRun();

  // check for empty interval
  if (empty()) return false;

  // check lower bound
  if (m_experimentLow) {
    if (experiment < m_experimentLow) return false;
    if (experiment == m_experimentLow) {
      if (run) {
        if (m_runLow && (run < m_runLow)) return false;
      } else {
        if (m_runLow > 1) return false;
      }
    }
  }

  // check upper bound
  if (m_experimentHigh) {
    if (experiment > m_experimentHigh) return false;
    if ((experiment == m_experimentHigh) && m_runHigh && (run > m_runHigh)) return false;
    if (experiment == m_experimentHigh) {
      if (run) {
        if (m_runHigh && (run > m_runHigh)) return false;
      } else {
        if (m_runHigh > 0) return false;
      }
    }
  }

  return true;
}


