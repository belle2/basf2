/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 **************************************************************************/

#include <framework/modules/core/ProgressBarModule.h>

#include <framework/core/Environment.h>
#include <framework/utilities/Utils.h>

#include <iostream>
#include <iomanip>
#include <unistd.h>

using namespace std;
using namespace Belle2;


REG_MODULE(ProgressBar)

ProgressBarModule::ProgressBarModule() : Module(), m_evtNr(0), m_nTotal(0), m_startTime(0), m_lastPrint(0)
{
  setDescription("Display a progress bar and an estimate of remaining time when number of events is known (e.g. reading from file, or -n switch used). "
                 "The bar uses stderr for output, so it works best when stdout is piped to a file.");
}

void ProgressBarModule::initialize()
{
  m_evtNr = 0;
  m_nTotal = Environment::Instance().getNumberOfEvents();
  m_startTime = 0;
  m_progress = 0;
  // Check if we run on a tty or if we are writing to a log file. In the second
  // case we don't use carriage return but just new lines and only update if the progress bar changes.
  m_isTTY = isatty(STDERR_FILENO);
}

void ProgressBarModule::event()
{
  if (m_nTotal == 0)
    return;

  double clockSec = Utils::getClock() / 1e9;

  // To not count intialization time we only print output starting one second
  // after the first event is processed
  if (m_startTime == 0) {
    // first call, let's start here
    m_startTime = clockSec;
    m_lastPrint = m_startTime;
    return;
  }

  if (clockSec - m_lastPrint > 1) { //every second
    double elapsedSec = clockSec - m_startTime;
    double ratio = double(m_evtNr) / m_nTotal;
    double time_per_event = elapsedSec / m_evtNr;
    int remainingSeconds = (int)std::round((m_nTotal - m_evtNr) * time_per_event);

    if (remainingSeconds >= 0) {
      const int bar_length = 50; //characters for bar

      //If we don't write to a tty only update if the progress bar changes
      if (!m_isTTY && int(ratio * bar_length) <= m_progress) return;
      // remember where we were
      m_progress = ratio * bar_length;

      cerr << '[';
      for (int i = 0; i < m_progress; ++i) cerr << '=';
      cerr << '>';
      for (int i = m_progress + 1; i < bar_length; ++i) cerr << ' ';
      cerr << "] " << setw(3) << int(ratio * 100) << "% ";
      if (remainingSeconds > 3600) {
        int hours = remainingSeconds / 3600;
        remainingSeconds %= 3600;
        cerr << hours << "h";
      }
      if (remainingSeconds > 60) {
        int minutes = remainingSeconds / 60;
        remainingSeconds %= 60;
        cerr << minutes << "m";
      }
      cerr << remainingSeconds << "s remaining";
      //some spaces to overwrite other stuff
      cerr << "          ";
      if (m_isTTY) {
        //carriage return to go back to beginning of the line
        cerr << "\r";
      } else {
        //not a terminal, just make it an output line
        cerr << endl;
      }
      // make sure it's printed
      cerr << flush;

      m_lastPrint = clockSec;
    }
  }

  ++m_evtNr;
}

void ProgressBarModule::terminate()
{
  cerr << "\n";
}
