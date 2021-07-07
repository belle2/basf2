/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/dbobjects/RunInfo.h>
#include <framework/gearbox/Const.h>
#include <framework/gearbox/Unit.h>

#include <iostream>

using namespace std;
using namespace Belle2;


void RunInfo::Print(Option_t*) const
{
  std::cout << " " << std::endl;
  std::cout << "Run Summary" << std::endl;
  std::cout << " Exp " << m_exp << ", Run " << m_run << std::endl;
  std::cout << " Run Type " << m_runType << std::endl;
  std::cout << " Start time " << m_startTime << ", Stop time " << m_stopTime
            << ", Run Length " << m_runLength << ", Trigger Rate " << m_triggerRate << std::endl;
  std::cout << " Received nevents " << m_receivedNevent << " Accepted nevents " << m_acceptedNevent
            << ", Sent nevents " << m_sentNevent << std::endl;

  std::cout << "Detector used " << std::endl;
  for (int i = 0; i < int(m_Belle2Detector.size()); ++i) {
    std::cout << m_Belle2Detector[i] << "  :   " ;
  }
  std::cout << std::endl;


}



