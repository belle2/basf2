/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <framework/calibration/RunInfoDatabaseImporter.h>

// framework - Database
#include <framework/database/Database.h>
#include <framework/database/DBArray.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBImportArray.h>
#include <framework/database/DBImportObjPtr.h>

// framework - xml
#include <framework/gearbox/GearDir.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// DB objects
#include <framework/dbobjects/RunInfo.h>

#include <iostream>
#include <fstream>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/gzip.hpp>

using namespace std;
using namespace Belle2;

void RunInfoDatabaseImporter::importRunInfo(std::string fileName)
{
  std::ifstream stream;
  stream.open(fileName.c_str());
  if (!stream) {
    B2ERROR("openFile: " << fileName << " *** failed to open");
    return;
  }
  B2INFO(fileName << ": open for reading");

  DBImportObjPtr<RunInfo> ri;
  ri.construct();

  int iExp(0);
  int iRun(0);
  std::string run_type;
  int run_length(0);
  int nReceived(0);
  int nAccepted(0);
  int nSent(0);
  double trigger_rate(0);
  int iPXD(0);
  int iSVD(0);
  int iCDC(0);
  int iTOP(0);
  int iARICH(0);
  int iECL(0);
  int iKLM(0);

  while (true) {
    stream >> iExp >> iRun >> run_type >> run_length >> nAccepted >> trigger_rate;
    stream >> iPXD >> iSVD >> iCDC >> iTOP >> iARICH >> iECL >> iKLM;
    if (stream.eof()) break;
    ri->setExp(iExp);
    ri->setRun(iRun);
    ri->setRunType(run_type);
    ri->setRunLength(run_length);
    ri->setAcceptedNevent(nAccepted);
    ri->setTriggerRate(trigger_rate);
    ri->setPXD(iPXD);
    ri->setSVD(iSVD);
    ri->setCDC(iCDC);
    ri->setTOP(iTOP);
    ri->setARICH(iARICH);
    ri->setECL(iECL);
    ri->setKLM(iKLM);

    //std::cout << iExp << " " << iRun << " " << run_type << " " << run_length << " " << nAccepted << " " << trigger_rate << std::endl;

  }
  stream.close();

  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);
  ri.import(iov);

  B2RESULT("Run Info table imported to database.");

}

void RunInfoDatabaseImporter::printRunInfo()
{

  DBObjPtr<RunInfo> ri;
  ri->dump();

}

