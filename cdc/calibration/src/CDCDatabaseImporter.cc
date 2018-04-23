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
#include <cdc/dbobjects/CDCXtRelations.h>
#include <cdc/dbobjects/CDCSpaceResols.h>
#include <cdc/dbobjects/CDCDisplacement.h>
#include <cdc/dbobjects/CDCAlignment.h>
#include <cdc/dbobjects/CDCADCDeltaPedestals.h>

#include <cdc/geometry/CDCGeometryPar.h>

#include <iostream>
#include <fstream>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/gzip.hpp>

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
    //    ps->setSpeed(speed);
    ps->setSpeed(iCL, speed);
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

  unsigned short mode(0), nParams(0);
  stream >> mode >> nParams;
  tw->setTwParamMode(mode);

  unsigned short iBoard(0);
  std::vector<float> coeffs(nParams);
  int nRead(0);

  while (stream >> iBoard) {
    for (unsigned short i = 0; i < nParams; ++i) {
      stream >> coeffs[i];
    }
    ++nRead;
    tw->setTimeWalkParams(iBoard, coeffs);
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
  DBImportObjPtr<CDCXtRelations> xt;
  xt.construct();

  //read alpha bins
  //  std::ifstream ifs;
  //  ifs.open(fileName.c_str());
  boost::iostreams::filtering_istream ifs;
  if ((fileName.rfind(".gz") != string::npos) && (fileName.length() - fileName.rfind(".gz") == 3)) {
    ifs.push(boost::iostreams::gzip_decompressor());
  }
  ifs.push(boost::iostreams::file_source(fileName));
  if (!ifs) {
    B2FATAL("openFile: " << fileName << " *** failed to open");
  }
  B2INFO(fileName << ": open for reading");

  const double degrad = M_PI / 180.;
  const double raddeg = 180. / M_PI;

  unsigned short nAlphaBins = 0;
  if (ifs >> nAlphaBins) {
    if (nAlphaBins == 0 || nAlphaBins > maxNAlphaPoints) B2FATAL("Fail to read alpha bins !");
  } else {
    B2FATAL("Fail to read alpha bins !");
  }
  std::array<float, 3> alpha3;
  for (unsigned short i = 0; i < nAlphaBins; ++i) {
    for (unsigned short j = 0; j < 3; ++j) {
      ifs >> alpha3[j];
      alpha3[j] *= degrad;
    }
    xt->setAlphaBin(alpha3);
  }

  //read theta bins
  unsigned short nThetaBins = 0;
  if (ifs >> nThetaBins) {
    if (nThetaBins == 0 || nThetaBins > maxNThetaPoints) B2FATAL("Fail to read theta bins !");
  } else {
    B2FATAL("Fail to read theta bins !");
  }
  std::array<float, 3> theta3;

  for (unsigned short i = 0; i < nThetaBins; ++i) {
    for (unsigned short j = 0; j < 3; ++j) {
      ifs >> theta3[j];
      theta3[j] *= degrad;
    }
    xt->setThetaBin(theta3);
  }


  //read xt params.
  /*  std::ifstream ifs;
  ifs.open(fileName.c_str());
  if (!ifs) {
    B2FATAL("openFile: " << fileName << " *** failed to open");
    return;
  }
  B2INFO(fileName << ": open for reading");
  */
  short xtParamMode, np;
  unsigned short iCL, iLR;
  const unsigned short npx = nXTParams - 1;
  double xtc[npx];
  double theta, alpha, dummy1;
  unsigned nRead = 0;

  ifs >> xtParamMode >> np;
  if (xtParamMode < 0 || xtParamMode > 1) B2FATAL("Invalid xt param mode read !");
  if (np <= 0 || np > npx) B2FATAL("No. of xt-params. outside limits !");

  xt->setXtParamMode(xtParamMode);

  const double epsi = 0.1;

  while (ifs >> iCL) {
    ifs >> theta >> alpha >> dummy1 >> iLR;
    for (int i = 0; i < np; ++i) {
      ifs >> xtc[i];
    }
    ++nRead;

    int ialpha = -99;
    for (unsigned short i = 0; i < nAlphaBins; ++i) {
      if (fabs(alpha - xt->getAlphaBin(i)[2]*raddeg) < epsi) {
        ialpha = i;
        break;
      }
    }
    if (ialpha < 0) B2FATAL("alphas in xt.dat are inconsistent !");

    int itheta = -99;
    for (unsigned short i = 0; i < nThetaBins; ++i) {
      if (fabs(theta - xt->getThetaBin(i)[2]*raddeg) < epsi) {
        itheta = i;
        break;
      }
    }
    if (itheta < 0) B2FATAL("thetas in xt.dat are inconsistent !");

    //    std::vector<float> xtbuff = std::vector<float>(np);
    std::vector<float> xtbuff;
    for (int i = 0; i < np; ++i) {
      xtbuff.push_back(xtc[i]);
    }
    //    std::cout <<"iCL,iLR,ialpha,itheta= " << iCL <<" "<< iLR <<" "<< ialpha <<" "<< itheta << std::endl;
    xt->setXtParams(iCL, iLR, ialpha, itheta, xtbuff);
  }

  //  ifs.close();
  boost::iostreams::close(ifs);

  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);
  xt.import(iov);
  B2RESULT("XT table imported to database.");
}

void CDCDatabaseImporter::importSigma(std::string fileName)
{
  DBImportObjPtr<CDCSpaceResols> sg;
  sg.construct();

  //read alpha bins
  std::ifstream ifs;
  ifs.open(fileName.c_str());
  if (!ifs) {
    B2FATAL("openFile: " << fileName << " *** failed to open");
  }
  B2INFO(fileName << ": open for reading");

  const double degrad = M_PI / 180.;
  const double raddeg = 180. / M_PI;

  unsigned short nAlphaBins = 0;
  if (ifs >> nAlphaBins) {
    if (nAlphaBins == 0 || nAlphaBins > maxNAlphaPoints) B2FATAL("Fail to read alpha bins !");
  } else {
    B2FATAL("Fail to read alpha bins !");
  }
  std::array<float, 3> alpha3;
  for (unsigned short i = 0; i < nAlphaBins; ++i) {
    for (unsigned short j = 0; j < 3; ++j) {
      ifs >> alpha3[j];
      alpha3[j] *= degrad;
    }
    sg->setAlphaBin(alpha3);
  }

  //read theta bins
  unsigned short nThetaBins = 0;
  if (ifs >> nThetaBins) {
    if (nThetaBins == 0 || nThetaBins > maxNThetaPoints) B2FATAL("Fail to read theta bins !");
  } else {
    B2FATAL("Fail to read theta bins !");
  }
  std::array<float, 3> theta3;

  for (unsigned short i = 0; i < nThetaBins; ++i) {
    for (unsigned short j = 0; j < 3; ++j) {
      ifs >> theta3[j];
      theta3[j] *= degrad;
    }
    sg->setThetaBin(theta3);
  }


  //read sigma params.
  short sgParamMode, np;
  unsigned short iCL, iLR;
  const unsigned short npx = nSigmaParams;
  double sgm[npx];
  double theta, alpha;
  unsigned nRead = 0;

  ifs >> sgParamMode >> np;
  if (sgParamMode < 0 || sgParamMode > 1) B2FATAL("Invalid sigma param mode read !");
  if (np <= 0 || np > npx) B2FATAL("No. of sgm-params. outside limits !");

  sg->setSigmaParamMode(sgParamMode);

  const double epsi = 0.1;

  while (ifs >> iCL) {
    ifs >> theta >> alpha >> iLR;
    for (int i = 0; i < np; ++i) {
      ifs >> sgm[i];
    }
    ++nRead;

    int ialpha = -99;
    for (unsigned short i = 0; i < nAlphaBins; ++i) {
      if (fabs(alpha - sg->getAlphaBin(i)[2]*raddeg) < epsi) {
        ialpha = i;
        break;
      }
    }
    if (ialpha < 0) B2FATAL("alphas in sigma.dat are inconsistent !");

    int itheta = -99;
    for (unsigned short i = 0; i < nThetaBins; ++i) {
      if (fabs(theta - sg->getThetaBin(i)[2]*raddeg) < epsi) {
        itheta = i;
        break;
      }
    }
    if (itheta < 0) B2FATAL("thetas in sigma.dat are inconsistent !");

    //    std::vector<float> sgbuff = std::vector<float>(np);
    std::vector<float> sgbuff;
    for (int i = 0; i < np; ++i) {
      sgbuff.push_back(sgm[i]);
    }
    //    std::cout <<"iCL,iLR,ialpha,itheta= " << iCL <<" "<< iLR <<" "<< ialpha <<" "<< itheta << std::endl;
    sg->setSigmaParams(iCL, iLR, ialpha, itheta, sgbuff);
  }

  ifs.close();

  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);
  sg.import(iov);
  B2RESULT("Sigma table imported to database.");
}


void CDCDatabaseImporter::importDisplacement(std::string fileName)
{
  //read alpha bins
  //  std::ifstream ifs;
  //  ifs.open(fileName.c_str());
  boost::iostreams::filtering_istream ifs;
  if ((fileName.rfind(".gz") != string::npos) && (fileName.length() - fileName.rfind(".gz") == 3)) {
    ifs.push(boost::iostreams::gzip_decompressor());
  }
  ifs.push(boost::iostreams::file_source(fileName));
  if (!ifs) {
    B2FATAL("openFile: " << fileName << " *** failed to open");
  }
  B2INFO(fileName << ": open for reading");

  DBImportArray<CDCDisplacement> disp;

  int iL(0), iC(0);
  const int np = 3;
  double back[np], fwrd[np];
  double tension = 0.;
  unsigned nRead = 0;

  while (true) {
    ifs >> iL >> iC;
    for (int i = 0; i < np; ++i) {
      ifs >> back[i];
    }
    for (int i = 0; i < np; ++i) {
      ifs >> fwrd[i];
    }
    ifs >> tension;

    if (ifs.eof()) break;

    ++nRead;
    WireID wire(iL, iC);
    TVector3 fwd(fwrd[0], fwrd[1], fwrd[2]);
    TVector3 bwd(back[0], back[1], back[2]);
    disp.appendNew(wire, fwd, bwd, tension);
  }

  if (nRead != nSenseWires) B2FATAL("CDCDatabaseimporter::importDisplacement: #lines read-in (=" << nRead <<
                                      ") is inconsistent with total #sense wires (=" << nSenseWires << ") !");

  //  ifs.close();
  boost::iostreams::close(ifs);

  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);
  disp.import(iov);
  B2RESULT("Wire displasement table imported to database.");
}


void CDCDatabaseImporter::importWirPosAlign(std::string fileName)
{
  //  std::ifstream ifs;
  //  ifs.open(fileName.c_str());
  boost::iostreams::filtering_istream ifs;
  if ((fileName.rfind(".gz") != string::npos) && (fileName.length() - fileName.rfind(".gz") == 3)) {
    ifs.push(boost::iostreams::gzip_decompressor());
  }
  ifs.push(boost::iostreams::file_source(fileName));
  if (!ifs) {
    B2FATAL("openFile: " << fileName << " *** failed to open");
    return;
  }
  B2INFO(fileName << ": open for reading");

  DBImportObjPtr<CDCAlignment> al;
  al.construct();

  int iL(0), iC(0);
  const int np = 3;
  double back[np], fwrd[np], tension;
  unsigned nRead = 0;

  while (true) {
    ifs >> iL >> iC;
    for (int i = 0; i < np; ++i) {
      ifs >> back[i];
    }
    for (int i = 0; i < np; ++i) {
      ifs >> fwrd[i];
    }
    ifs >> tension;
    if (ifs.eof()) break;

    ++nRead;
    WireID wire(iL, iC);

    for (int i = 0; i < np; ++i) {
      al->set(wire, CDCAlignment::wireBwdX,  back[0]);
      al->set(wire, CDCAlignment::wireBwdY,  back[1]);
      al->set(wire, CDCAlignment::wireBwdZ,  back[2]);
      al->set(wire, CDCAlignment::wireFwdX, fwrd[0]);
      al->set(wire, CDCAlignment::wireFwdY, fwrd[1]);
      al->set(wire, CDCAlignment::wireFwdZ, fwrd[2]);
    }
    al->set(wire, CDCAlignment::wireTension, tension);
  }

  if (nRead != nSenseWires) B2FATAL("CDCDatabaseimporter::importWirPosAlign: #lines read-in (=" << nRead <<
                                      ") is inconsistent with total #sense wires (=" << nSenseWires << ") !");

  //  ifs.close();
  boost::iostreams::close(ifs);

  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);
  al.import(iov);
  B2RESULT("Wire alignment table imported to database.");
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
  DBObjPtr<CDCXtRelations> xt;
  xt->dump();
}

void CDCDatabaseImporter::printSigma()
{
  DBObjPtr<CDCSpaceResols> sgm;
  sgm->dump();
}

void CDCDatabaseImporter::printDisplacement()
{
  DBArray<CDCDisplacement> displacements;
  for (const auto& disp : displacements) {
    B2INFO(disp.getICLayer() << " " << disp.getIWire() << " "
           << disp.getXBwd() << " " << disp.getYBwd() << " " << disp.getZBwd() <<  " "
           << disp.getXFwd() << " " << disp.getYFwd() << " " << disp.getZFwd() << " " << disp.getTension());
  }
}

void CDCDatabaseImporter::printWirPosAlign()
{
  DBObjPtr<CDCAlignment> al;
  al->dump();
}

void CDCDatabaseImporter::printWirPosMisalign()
{
  DBObjPtr<CDCMisalignment> mal;
  mal->dump();
}


void CDCDatabaseImporter::importADCDeltaPedestal(std::string fileName)
{
  std::ifstream stream;
  stream.open(fileName.c_str());
  if (!stream.is_open()) {
    B2ERROR("openFile: " << fileName << " *** failed to open");
    return;
  }
  B2INFO(fileName << ": open for reading");

  DBImportObjPtr<CDCADCDeltaPedestals> dbPed;
  dbPed.construct();

  int iB(0);
  int iC(0);
  float ped(0);
  int nRead(0);
  int sample(0);

  while (true) {
    if (nRead == 0) {
      stream >> sample;
    } else {
      stream >> iB >> iC >> ped;
    }
    if (stream.eof()) break;
    if (nRead == 0) {
      if (sample == 0) {
        B2FATAL("sample window is zero !");
      }
      dbPed->setSamplingWindow(sample);
    } else {
      dbPed->setPedestal(iB, iC, ped);
    }
    ++nRead;

  }
  stream.close();

  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);
  dbPed.import(iov);

  B2RESULT("ADC delta pedestal table imported to database.");
}

void CDCDatabaseImporter::importADCDeltaPedestal()
{

  DBImportObjPtr<CDCADCDeltaPedestals> dbPed;
  dbPed.construct();

  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);
  dbPed.import(iov);

  B2RESULT("ADC delta pedestal w/ zero  imported to database.");
}

void CDCDatabaseImporter::printADCDeltaPedestal()
{

  DBObjPtr<CDCADCDeltaPedestals> dbPed;
  dbPed->dump();
}

//Note; the following function is no longer needed
#if 0
void CDCDatabaseImporter::importWirPosMisalign(std::string fileName)
{
  std::ifstream ifs;
  ifs.open(fileName.c_str());
  if (!ifs) {
    B2FATAL("openFile: " << fileName << " *** failed to open");
    return;
  }
  B2INFO(fileName << ": open for reading");

  DBImportObjPtr<CDCMisalignment> mal;
  mal.construct();

  int iL(0), iC(0);
  const int np = 3;
  double back[np], fwrd[np], tension;
  unsigned nRead = 0;

  while (true) {
    ifs >> iL >> iC;
    for (int i = 0; i < np; ++i) {
      ifs >> back[i];
    }
    for (int i = 0; i < np; ++i) {
      ifs >> fwrd[i];
    }
    ifs >> tension;
    if (ifs.eof()) break;

    ++nRead;
    WireID wire(iL, iC);

    for (int i = 0; i < np; ++i) {
      mal->set(wire, CDCMisalignment::wireBwdX,  back[0]);
      mal->set(wire, CDCMisalignment::wireBwdY,  back[1]);
      mal->set(wire, CDCMisalignment::wireBwdZ,  back[2]);
      mal->set(wire, CDCMisalignment::wireFwdX, fwrd[0]);
      mal->set(wire, CDCMisalignment::wireFwdY, fwrd[1]);
      mal->set(wire, CDCMisalignment::wireFwdZ, fwrd[2]);
    }
    mal->set(wire, CDCMisalignment::wireTension, tension);
  }

  if (nRead != nSenseWires) B2FATAL("CDCDatabaseimporter::importWirPosMisalign: #lines read-in (=" << nRead <<
                                      ") is inconsistent with total #sense wires (=" << nSenseWires << ") !");

  ifs.close();

  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);
  mal.import(iov);
  B2RESULT("Wire misalignment table imported to database.");
}
#endif

