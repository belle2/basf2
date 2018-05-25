/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Karim, Vishal
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/dbobjects/RunInfo.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>
#include <framework/gearbox/Unit.h>



using namespace std;
using namespace Belle2;


void RunInfo::Print()
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



