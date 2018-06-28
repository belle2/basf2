/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * ECLCrystalLocalRunCalib                                                *
 *                                                                        *
 * This class is designed to store ECL local run calibration              *
 * results to database (separately for amplitude and time).               *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Gribanov Sergei    (S.S.Gribanov@inp.nsk.su)             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
// ECL
#include <ecl/dbobjects/ECLCrystalLocalRunCalib.h>
using namespace Belle2;
// Constructor.
ECLCrystalLocalRunCalib::ECLCrystalLocalRunCalib(bool isNegAmpl):
  ECLCrystalCalib::ECLCrystalCalib(),
  c_isNegAmpl(isNegAmpl),
  m_exp(0), m_run(0),
  m_numberOfEvents(0)
{
}
// Destructor.
ECLCrystalLocalRunCalib::~ECLCrystalLocalRunCalib()
{
}
// Set total number of events.
void ECLCrystalLocalRunCalib::setNumberOfEvents(const int& numberOfEvents)
{
  m_numberOfEvents = numberOfEvents;
}
// Set numbers of accepted events for each cell id.
void ECLCrystalLocalRunCalib::setNumbersOfAcceptedEvents(
  const std::vector<int>& numberOfAcceptedEvents)
{
  m_numberOfAcceptedEvs = numberOfAcceptedEvents;
}
// Get total number of events.
int ECLCrystalLocalRunCalib::getNumberOfEvents() const
{
  return m_numberOfEvents;
}
// Get numbers of accepted events.
const std::vector<int>& ECLCrystalLocalRunCalib::getNumbersOfAcceptedEvents() const
{
  return m_numberOfAcceptedEvs;
}
// Set experiment and run numbers.
void
ECLCrystalLocalRunCalib::setExpRun(const int& exp, const int& run)
{
  m_exp = exp;
  m_run = run;
}
// Get experiment number.
int ECLCrystalLocalRunCalib::getExp() const
{
  return m_exp;
}
// Get run number.
int ECLCrystalLocalRunCalib::getRun() const
{
  return m_run;
}
// Check presence of negative amplitudes.
bool ECLCrystalLocalRunCalib::isNegAmpl() const
{
  return c_isNegAmpl;
}
