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
#include <cdc/dbobjects/CDCTimeZeros.h>
#include <cdc/dbobjects/CDCBadWires.h>
#include <cdc/dbobjects/CDCPropSpeeds.h>
#include <cdc/dbobjects/CDCTimeWalks.h>
#include <cdc/dbobjects/CDCXTs.h>

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

  DBImportObjPtr<CDCTimeZeros> tz;
  tz.construct();

  int iL(0);
  int iC(0);
  double t0(0.);
  int nRead(0);

  while (true) {
    stream >> iL >> iC >> t0;
    if (stream.eof()) break;
    ++nRead;
    WireID wire(iL, iC);
    tz->setT0(wire, t0);
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
    B2FATAL("openFile: " << fileName << " *** failed to open");
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
    B2FATAL("openFile: " << fileName << " *** failed to open");
    return;
  }
  B2INFO(fileName << ": open for reading");

  DBImportObjPtr<CDCPropSpeeds> ps;
  ps.construct();

  int iCL(0), nRead(0);
  double speed(0.);

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
    B2FATAL("openFile: " << fileName << " *** failed to open");
    return;
  }
  B2INFO(fileName << ": open for reading");

  DBImportObjPtr<CDCTimeWalks> tw;
  tw.construct();

  unsigned short iBoard(0);
  int nRead(0);
  double coeff(0.);

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


void CDCDatabaseImporter::importXT(std::string fileName)
{
  std::ifstream ifs;
  ifs.open(fileName.c_str());
  if (!ifs) {
    B2FATAL("openFile: " << fileName << " *** failed to open");
    return;
  }
  B2INFO(fileName << ": open for reading");

  DBImportObjPtr<CDCXTs> xt;
  xt.construct();

  int iL, lr;
  const int np = 9; //to be moved to appropriate place...
  double alpha, theta, dummy1, xtc[np];
  double   oldTheta(-999), oldAlpha(-999);
  unsigned noOfThetaPoints(0);
  unsigned noOfAlphaPoints(1); //should start with one for alpha

  //First read to check no.s of theta and alpha points
  double alphaPoints[nAlphaPoints] = {0.};

  int count = 0;
  while (ifs >> iL) {
    //    std::cout << count <<" "<<  ifs.eof() << std::endl;
    ++count;
    //    ifs >> iL >> theta >> alpha >> dummy1 >> lr;
    ifs >> theta >> alpha >> dummy1 >> lr;
    for (int i = 0; i < np - 1; ++i) {
      ifs >> xtc[i];
    }

    if (theta != oldTheta) {
      unsigned short iarg = std::min(noOfThetaPoints, nThetaPoints);
      xt->setThetaPoint(iarg, theta);
      ++noOfThetaPoints;
      oldTheta = theta;
    }

    if (noOfThetaPoints == 1 && alpha != oldAlpha) {
      unsigned short iarg = std::min(noOfAlphaPoints, nAlphaPoints);
      alphaPoints[iarg] = alpha;
      ++noOfAlphaPoints;
      oldAlpha = alpha;
    }
  }

  if (noOfThetaPoints != nThetaPoints) B2FATAL("importXT: Inconsistent no. of theta points ! real= " << noOfThetaPoints << " preset= "
                                                 << nThetaPoints);
  if (noOfAlphaPoints != nAlphaPoints) B2FATAL("importXT: Inconsistent no. of alpha points ! real in file= " << noOfAlphaPoints <<
                                                 " preset= " << nAlphaPoints);

  //set alpha for arg=0;
  xt->setAlphaPoint(0, -90.);
  //sort in order of magnitude
  for (unsigned i = 1; i < nAlphaPoints; ++i) {
    xt->setAlphaPoint(nAlphaPoints - i, alphaPoints[i]);
  }

  //Second read to set all the others
  //  std::cout <<"before rewind" <<" "<< ifs.eof() << std::endl;
  ifs.clear(); //necessary to make the next line work
  ifs.seekg(0, ios_base::beg);
  //  std::cout <<"after  rewind" <<" "<< ifs.eof() << std::endl;
  unsigned nRead = 0;

  while (ifs >> iL) {
    ifs >> theta >> alpha >> dummy1 >> lr;
    for (int i = 0; i < np - 1; ++i) {
      ifs >> xtc[i];
    }
    ++nRead;

    int itheta = 0;
    for (unsigned short i = 0; i < nThetaPoints; ++i) {
      if (theta == xt->getThetaPoint(i)) itheta = i;
    }

    int ialpha = 0;
    for (unsigned short i = 1; i < nAlphaPoints; ++i) {
      if (alpha == xt->getAlphaPoint(i)) ialpha = i;
    }

    for (int i = 0; i < np - 1; ++i) {
      xt->setXTParam(iL, lr, ialpha, itheta, i, xtc[i]);
    }

    if (xt->getXTParam(iL, lr, ialpha, itheta, 1) * xt->getXTParam(iL, lr, ialpha, itheta, 7) < 0.) {
      //      B2WARNING("importXT: xtc[7] sign is inconsistent with xtc[1] sign -> set xtc[7]=0");
      xt->setXTParam(iL, lr, ialpha, itheta, 7, 0.);
    }

    double bound = xt->getXTParam(iL, lr, ialpha, itheta, 6);
    int i = np - 1;
    xtc[i] = xt->getXTParam(iL, lr, ialpha, itheta, 0) + bound
             * (xt->getXTParam(iL, lr, ialpha, itheta, 1) + bound
                * (xt->getXTParam(iL, lr, ialpha, itheta, 2) + bound
                   * (xt->getXTParam(iL, lr, ialpha, itheta, 3) + bound
                      * (xt->getXTParam(iL, lr, ialpha, itheta, 4) + bound
                         * (xt->getXTParam(iL, lr, ialpha, itheta, 5))))));
    xt->setXTParam(iL, lr, ialpha, itheta, i, xtc[i]);

    /*
       cout << iL << " " << alpha << " " << theta << " " << dummy1 << " " << lr;
       for (int i = 0; i < np; ++i) {
       cout << " " << xtc[i];
       }
       cout << endl;
    */

    //convert unit, microsec -> nsec  <- tentative
    xt->MultiplyFactor(iL, lr, ialpha, itheta, 1, 1.e-3);
    xt->MultiplyFactor(iL, lr, ialpha, itheta, 2, 1.e-6);
    xt->MultiplyFactor(iL, lr, ialpha, itheta, 3, 1.e-9);
    xt->MultiplyFactor(iL, lr, ialpha, itheta, 4, 1.e-12);
    xt->MultiplyFactor(iL, lr, ialpha, itheta, 5, 1.e-15);
    xt->MultiplyFactor(iL, lr, ialpha, itheta, 6, 1.e3);
    xt->MultiplyFactor(iL, lr, ialpha, itheta, 7, 1.e-3);
  }

  if (nRead != 2 * (nAlphaPoints - 1) * nThetaPoints * MAX_N_SLAYERS) B2FATAL("importXT: #lines read-in (=" << nRead <<
        ") is inconsistent with 2*18*7 x total #layers (=" << 2 * (nAlphaPoints - 1) * nThetaPoints * MAX_N_SLAYERS << ") !");

  ifs.close();

  //set xt(L/R,alpha=-90deg) = xt(R/L,alpha=90deg)
  for (unsigned iL = 0; iL < MAX_N_SLAYERS; ++iL) {
    for (int lr = 0; lr < 2; ++lr) {
      int lrp = 0;
      if (lr == 0) lrp = 1;
      for (unsigned itheta = 0; itheta < nThetaPoints; ++itheta) {
        for (int i = 0; i < np; ++i) {
          double sgn = -1.;
          if (i == 6) sgn = 1;
          xt->setXTParam(iL, lr, 0, itheta, i, sgn * xt->getXTParam(iL, lrp, 18, itheta, i));
        }
      }
    }
  }

  //set xt(theta= 18) = xt(theta= 40) for the layers >= 20, since xt(theta=18) for these layers are unavailable
  for (unsigned iL = 20; iL < MAX_N_SLAYERS; ++iL) {
    for (int lr = 0; lr < 2; ++lr) {
      for (unsigned ialpha = 0; ialpha < nAlphaPoints; ++ialpha) {
        xt->copyXTParam(iL, lr, ialpha, 1, 0);
      }
    }
  }

  //set xt(theta=130) = xt(theta=120) for the layers >= 37, since xt(theta=130) for these layers are unavailable
  for (unsigned iL = 37; iL < MAX_N_SLAYERS; ++iL) {
    for (int lr = 0; lr < 2; ++lr) {
      for (unsigned ialpha = 0; ialpha < nAlphaPoints; ++ialpha) {
        xt->copyXTParam(iL, lr, ialpha, 4, 5);
      }
    }
  }

  //set xt(theta=149) = xt(theta=130) for the layers >= 13, since xt(theta=149) for these layers are unavailable
  for (unsigned iL = 13; iL < MAX_N_SLAYERS; ++iL) {
    for (int lr = 0; lr < 2; ++lr) {
      for (unsigned ialpha = 0; ialpha < nAlphaPoints; ++ialpha) {
        xt->copyXTParam(iL, lr, ialpha, 5, 6);
      }
    }
  }

  //convert unit
  for (unsigned i = 0; i < nAlphaPoints; ++i) {
    xt->setAlphaPoint(i, xt->getAlphaPoint(i) * M_PI / 180.);
  }
  for (unsigned i = 0; i < nThetaPoints; ++i) {
    xt->setThetaPoint(i, xt->getThetaPoint(i) * M_PI / 180.);
  }

  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);
  xt.import(iov);
  B2RESULT("XT table imported to database.");
}


void CDCDatabaseImporter::importSigma(std::string fileName)
{
  std::ifstream ifs;
  ifs.open(fileName.c_str());
  if (!ifs) {
    B2FATAL("openFile: " << fileName << " *** failed to open");
    return;
  }
  B2INFO(fileName << ": open for reading");

  DBImportObjPtr<CDCSigmas> sgm;
  sgm.construct();

  int iL;
  const int np = nSigmaParams;
  double sigma[np];
  unsigned nRead = 0;

  while (true) {
    ifs >> iL;
    for (int i = 0; i < np; ++i) {
      ifs >> sigma[i];
    }
    if (ifs.eof()) break;

    ++nRead;

    for (int i = 0; i < np; ++i) {
      sgm->setSigmaParam(iL, i, sigma[i]);
    }

    /*    if (m_debug) {
      cout << iL;
      for (int i = 0; i < np; ++i) {
        cout << " " << m_Sigma[iL][i];
      }
      cout << endl;
    }
    */
  }

  ifs.close();

  if (nRead != MAX_N_SLAYERS) B2FATAL("importSigma: #lines read-in (=" << nRead << ") is inconsistent with total #layers (=" <<
                                        MAX_N_SLAYERS << ") !");

  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);
  sgm.import(iov);
  B2RESULT("Sigma table imported to database.");
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

  DBObjPtr<CDCTimeZeros> timeZeros;

  /*  for (const auto& tz : timeZeros) {
    std::cout << tz.getICLayer() << " " << tz.getIWire() << " "
              << tz.getT0() << std::endl;
  }
  */
  timeZeros->dump();

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

void CDCDatabaseImporter::printXT()
{
  DBObjPtr<CDCXTs> xt;
  xt->dump();
}

void CDCDatabaseImporter::printSigma()
{
  DBObjPtr<CDCSigmas> sgm;
  sgm->dump();
}
