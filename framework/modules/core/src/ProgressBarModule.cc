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

using namespace std;
using namespace Belle2;


REG_MODULE(ProgressBar)

ProgressBarModule::ProgressBarModule() : Module(), m_evtNr(0), m_nTotal(0), m_startTime(0), m_lastPrint(0)
{
  setDescription("Display a progress bar and an estimate of remaining time when number of events is known (e.g. reading from file, or -n switch used). The bar uses stderr for output, so it works best when stdout is piped to a file.");
}

void ProgressBarModule::initialize()
{
  m_evtNr = 0;
  m_nTotal = Environment::Instance().getNumberOfEvents();
  m_startTime = (long)(Utils::getClock() / 1e9);
  m_lastPrint = m_startTime;
}

void ProgressBarModule::event()
{
  if (m_nTotal == 0)
    return;

  long clockSec = (long)(Utils::getClock() / 1e9);

  if (clockSec - m_lastPrint > 0) { //every second
    long elapsedSec = clockSec - m_startTime;
    double ratio = double(m_evtNr) / m_nTotal;
    double time_per_event = double(elapsedSec) / m_evtNr;
    int remainingSeconds = (m_nTotal - m_evtNr) * time_per_event;

    if (remainingSeconds >= 0) {
      std::cerr << "\r"; //carriage return

      const int bar_length = 50; //characters for bar
      cerr << '[';
      bool arrow_drawn = false;
      for (int i = 0; i < bar_length; i++) {
        char c = ' ';
        if (double(i) / bar_length < ratio) {
          c = '=';
        } else if (!arrow_drawn) {
          c = '>';
          arrow_drawn = true;
        }
        cerr << c;

      }
      cerr << "] " << int(ratio * 100) << "% ";
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
      cerr << "          " << flush;

      m_lastPrint = clockSec;
    }
  }

  ++m_evtNr;
}

void ProgressBarModule::terminate()
{
  cerr << "\n";
}
