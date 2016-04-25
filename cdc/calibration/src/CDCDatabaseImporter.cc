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
#include <cdc/dataobjects/WireID.h>
#include <cdc/dbobjects/CDCChannelMap.h>
#include <cdc/dbobjects/CDCTimeZero.h>
#include <cdc/dbobjects/CDCBadWires.h>
#include <cdc/dbobjects/CDCPropSpeeds.h>
#include <cdc/dbobjects/CDCTimeWalks.h>

#include <iostream>
#include <fstream>

using namespace std;
using namespace Belle2;

void CDCDatabaseImporter::importTimeZero(std::string fileName)
{
  std::ifstream stream;
  stream.open(fileName.c_str());
  if (!stream) {
    B2ERROR("openFile: " << fileName << " *** failed to open");
    return;
  }
  B2INFO(fileName << ": open for reading");

  DBImportArray<CDCTimeZero> tz;

  int iL(0);
  int iC(0);
  float t0(0.);
  int nRead(0);

  while (true) {
    stream >> iL >> iC >> t0;
    if (stream.eof()) break;
    ++nRead;
    WireID wire(iL, iC);
    tz.appendNew(wire, t0);
    //      if (m_debug) {
    //  std::cout << iL << " " << iC << " " << t0 << std::endl;
    //      }
  }
  stream.close();

  if (nRead != nSenseWires) B2FATAL("#lines read-in (=" << nRead << ") is inconsistent with total #sense wires (=" << nSenseWires <<
                                      ") !");

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

void CDCDatabaseImporter::importBadWire(std::string fileName)
{
  std::ifstream stream;
  stream.open(fileName.c_str());
  if (!stream) {
    B2ERROR("openFile: " << fileName << " *** failed to open");
    return;
  }
  B2INFO(fileName << ": open for reading");

  DBImportObjPtr<CDCBadWires> bw;
  bw.construct();

  int iL(0), iC(0), nRead(0);

  while (true) {
    stream >> iL >> iC;
    if (stream.eof()) break;
    ++nRead;
    bw->setWire(WireID(iL, iC));
    //      if (m_debug) {
    //  std::cout << iL << " " << iC << std::endl;
    //      }
  }
  stream.close();

  if (nRead > static_cast<int>(nSenseWires)) B2FATAL("#lines read-in (=" << nRead << ") is larger than #sense wires (=" << nSenseWires
                                                       << ") !");

  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);
  bw.import(iov);
  B2RESULT("BadWire table imported to database.");
}


void CDCDatabaseImporter::importPropSpeed(std::string fileName)
{
  std::ifstream stream;
  stream.open(fileName.c_str());
  if (!stream) {
    B2ERROR("openFile: " << fileName << " *** failed to open");
    return;
  }
  B2INFO(fileName << ": open for reading");

  DBImportObjPtr<CDCPropSpeeds> ps;
  ps.construct();

  int iCL(0), nRead(0);
  float speed(0.);

  while (true) {
    stream >> iCL >> speed;
    if (stream.eof()) break;
    ++nRead;
    ps->setSpeed(speed);
    //    ps->setSpeed(iCL, speed);
    //      if (m_debug) {
    //  std::cout << iCL << " " << value << std::endl;
    //      }
  }
  stream.close();

  if (nRead != MAX_N_SLAYERS) B2FATAL("#lines read-in (=" << nRead << ") is no equal #sense layers (=" << MAX_N_SLAYERS << ") !");

  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);
  ps.import(iov);
  B2RESULT("PropSpeed table imported to database.");
}


void CDCDatabaseImporter::importTimeWalk(std::string fileName)
{
  std::ifstream stream;
  stream.open(fileName.c_str());
  if (!stream) {
    B2ERROR("openFile: " << fileName << " *** failed to open");
    return;
  }
  B2INFO(fileName << ": open for reading");

  DBImportObjPtr<CDCTimeWalks> tw;
  tw.construct();

  unsigned short iBoard(0);
  int nRead(0);
  float coeff(0.);

  while (true) {
    stream >> iBoard >> coeff;
    if (stream.eof()) break;
    ++nRead;
    tw->setTimeWalk(iBoard, coeff);
    //      if (m_debug) {
    //  std::cout << iBoard << " " << coeff << std::endl;
    //      }
  }
  stream.close();

  if (nRead != nBoards) B2FATAL("#lines read-in (=" << nRead << ") is not equal #boards (=" << nBoards << ") !");

  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);
  tw.import(iov);
  B2RESULT("Time-walk coeff. table imported to database.");
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

void CDCDatabaseImporter::printBadWire()
{
  DBObjPtr<CDCBadWires> bw;
  bw->dump();
}

void CDCDatabaseImporter::printPropSpeed()
{
  DBObjPtr<CDCPropSpeeds> ps;
  ps->dump();
}

void CDCDatabaseImporter::printTimeWalk()
{
  DBObjPtr<CDCTimeWalks> tw;
  tw->dump();
}
