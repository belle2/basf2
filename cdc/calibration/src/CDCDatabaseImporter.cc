/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Makoto Uchida                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <cdc/calibration/CDCDatabaseImporter.h>

// framework - Database
#include <framework/database/Database.h>
#include <framework/database/DBArray.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBImportArray.h>

// framework - xml
#include <framework/gearbox/GearDir.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// DB objects
#include <cdc/dbobjects/CDCChannelMap.h>
#include <cdc/dbobjects/CDCTimeZero.h>
#include <cdc/dataobjects/WireID.h>

#include <iostream>
#include <fstream>

using namespace std;
using namespace Belle2;

void CDCDatabaseImporter::importTimeZero()
{
  DBImportArray<CDCTimeZero> tz;
  for (int cl = 0; cl < 56; ++cl) {
    for (int w = 0; w < 384; ++w) {
      int sl = cl > 7 ? static_cast<int>((cl - 8) / 6) + 1 : 0;
      int l = cl > 7 ? static_cast<int>((cl - 8) % 6) : cl;
      const WireID wire = WireID(sl, l, w);
      const float t0 = m_cdcgp.getT0(wire);
      tz.appendNew(wire, t0);
    }
  }

  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);
  tz.import(iov);

  B2RESULT("Time zero table imported to database.");

}
void CDCDatabaseImporter::importChannelMap(std::string fileName)
{
  std::ifstream stream;
  stream.open(fileName.c_str());
  if (!stream) {
    B2ERROR("openFile: " << fileName << " *** failed to open");
    return;
  }
  B2INFO(fileName << ": open for reading");

  DBImportArray<CDCChannelMap> cm;

  int isl;
  int il;
  int iw;
  int iBoard;
  int iCh;

  while (!stream.eof()) {
    stream >>  isl >> il >> iw >> iBoard >> iCh;
    cm.appendNew(isl, il, iw, iBoard, iCh);
  }
  stream.close();

  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);

  cm.import(iov);

  B2RESULT("Channel map imported to database.");

}

void CDCDatabaseImporter::printChannelMap()
{

  DBArray<CDCChannelMap> channelMaps;

  for (const auto& cm : channelMaps) {
    std::cout << cm.getISuperLayer() << " " << cm.getILayer()
              << " " << cm.getIWire() << " "
              << cm.getBoardID() << " " << cm.getBoardChannel() << std::endl;
  }

}

void CDCDatabaseImporter::printTimeZero()
{

  DBArray<CDCTimeZero> timeZeros;

  for (const auto& tz : timeZeros) {
    std::cout << tz.getILayer() << " " << tz.getIWire() << " "
              << tz.getT0() << std::endl;
  }

}

