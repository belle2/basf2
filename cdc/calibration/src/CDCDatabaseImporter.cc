/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <cdc/calibration/CDCDatabaseImporter.h>

// framework - Database
#include <framework/database/DBArray.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBImportArray.h>
#include <framework/database/DBImportObjPtr.h>

// framework aux
#include <framework/logging/Logger.h>
// framework timer
#include <framework/utilities/Utils.h>

// DB objects
#include <cdc/dataobjects/WireID.h>
#include <cdc/dbobjects/CDCChannelMap.h>
#include <cdc/dbobjects/CDCTimeZeros.h>
#include <cdc/dbobjects/CDCBadWires.h>
#include <cdc/dbobjects/CDCPropSpeeds.h>
#include <cdc/dbobjects/CDCTimeWalks.h>
#include <cdc/dbobjects/CDCXtRelations.h>
#include <cdc/dbobjects/CDCSpaceResols.h>
#include <cdc/dbobjects/CDCFudgeFactorsForSigma.h>
#include <cdc/dbobjects/CDCDisplacement.h>
#include <cdc/dbobjects/CDCAlignment.h>
#include <cdc/dbobjects/CDCADCDeltaPedestals.h>
#include <cdc/dbobjects/CDCFEElectronics.h>
#include <cdc/dbobjects/CDCEDepToADCConversions.h>
#include <cdc/dbobjects/CDCWireHitRequirements.h>
#include <cdc/dbobjects/CDCCrossTalkLibrary.h>

#include <cdc/geometry/CDCGeometryPar.h>
#include <TFile.h>
#include <TTreeReader.h>
#include <TH1F.h>
#include <iostream>
#include <fstream>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

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


void CDCDatabaseImporter::importFEElectronics(std::string fileName)
{
  std::ifstream stream;
  stream.open(fileName.c_str());
  if (!stream) {
    B2ERROR("openFile: " << fileName << " *** failed to open");
    return;
  }
  B2INFO(fileName << ": open for reading");

  DBImportArray<CDCFEElectronics> cf;

  //  short width, delay, aTh, tThmV, tTheV, l1late;
  short ib, width, delay, aTh, tThmV;

  //  int i=-1;
  while (stream >> ib) {
    //    stream >> delay >> aTh >> tThmV >> tTheV >> l1late;
    stream >> width >> delay >> aTh >> tThmV;
    //    ++i;
    //    std::cout << i <<" "<< width << std::endl;
    //    cf.appendNew(width, delay, aTh, tThmV, tTheV, l1late);
    cf.appendNew(ib, width, delay, aTh, tThmV);
  }
  stream.close();

  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);

  cf.import(iov);

  B2RESULT("FEEElectronics imported to database.");
}


void CDCDatabaseImporter::importEDepToADC(std::string fileName)
{
  std::ifstream stream;
  stream.open(fileName.c_str());
  if (!stream) {
    B2FATAL("openFile: " << fileName << " *** failed to open");
    return;
  }
  B2INFO(fileName << ": open for reading");

  DBImportObjPtr<CDCEDepToADCConversions> etoa;
  etoa.construct();

  unsigned short paramMode(0), nParams(0);
  stream >> paramMode >> nParams;
  etoa->setParamMode(paramMode);

  unsigned short groupId(0);
  stream >> groupId;
  B2INFO(paramMode << " " << nParams << " " << groupId);
  if (groupId > 1) B2FATAL("invalid groupId now !");
  etoa->setGroupID(groupId);

  unsigned short id = 0;
  std::vector<float> coeffs(nParams);
  int nRead = 0;

  while (stream >> id) {
    for (unsigned short i = 0; i < nParams; ++i) {
      stream >> coeffs[i];
    }
    ++nRead;
    etoa->setParams(id, coeffs);
  }
  stream.close();

  unsigned short nId = nSuperLayers;
  if (groupId == 1) {
    nId = MAX_N_SLAYERS;
  } else if (groupId == 2) {
    nId = nSenseWires;
  }
  if (nRead != nId) B2FATAL("#lines read-in (=" << nRead << ") is not equal #ids (=" << nId << ") !");

  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);
  etoa.import(iov);
  B2RESULT("EDep-toADC table imported to database.");
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
  double effi(0.);

  while (true) {
    stream >> iL >> iC >> effi;
    if (stream.eof()) break;
    ++nRead;
    bw->setWire(WireID(iL, iC), effi);
    //      if (m_debug) {
    //  std::cout << iL << " " << iC << " " << effi << std::endl;
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

  float maxSigma;
  ifs >> maxSigma;
  sg->setMaxSpaceResol(maxSigma);

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


void CDCDatabaseImporter::importFFactor(std::string fileName)
{
  std::ifstream stream;
  stream.open(fileName.c_str());
  if (!stream) {
    B2FATAL("openFile: " << fileName << " *** failed to open");
    return;
  }
  B2INFO(fileName << ": open for reading");

  DBImportObjPtr<CDCFudgeFactorsForSigma> etoa;
  etoa.construct();

  unsigned short groupId(0), nParams(0);
  stream >> groupId >> nParams;
  B2INFO(groupId << " " << nParams);
  if (groupId != 0) B2FATAL("invalid groupId now !");
  etoa->setGroupID(groupId);

  unsigned short id = 0;
  std::vector<float> coeffs(nParams);
  int nRead = 0;

  while (stream >> id) {
    for (unsigned short i = 0; i < nParams; ++i) {
      stream >> coeffs[i];
    }
    ++nRead;
    etoa->setFactors(id, coeffs);
  }
  stream.close();

  unsigned short nId = 1;
  if (nRead != nId) B2FATAL("#lines read-in (=" << nRead << ") is not equal #ids (=" << nId << ") !");

  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);
  etoa.import(iov);
  B2RESULT("Fudge factor table imported to database.");
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

void CDCDatabaseImporter::printFEElectronics()
{
  DBArray<CDCFEElectronics> fEElectronics;
  for (const auto& cf : fEElectronics) {
    std::cout << cf.getBoardID() << " " << cf.getWidthOfTimeWindow() << " " << cf.getTrgDelay() << " " << cf.getADCThresh() << " " <<
              cf.getTDCThreshInMV() << std::endl;
  }
  //              << cf.getTDCThreshInMV() << " "
  //              << cf.getTDCThreshInEV() << " "
  //              << cf.getL1TrgLatency() << std::endl;
}

void CDCDatabaseImporter::printEDepToADC()
{
  DBObjPtr<CDCEDepToADCConversions> etoa;
  etoa->dump();
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

void CDCDatabaseImporter::printFFactor()
{
  DBObjPtr<CDCFudgeFactorsForSigma> ff;
  ff->dump();
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

void CDCDatabaseImporter::importCDCWireHitRequirements(const std::string& jsonFileName) const
{

  // Create a property tree
  boost::property_tree::ptree tree;

  try {

    // Load the json file in this property tree.
    B2INFO("Loading json file: " << jsonFileName);
    boost::property_tree::read_json(jsonFileName, tree);

  } catch (boost::property_tree::ptree_error& e) {
    B2FATAL("Error when loading json file: " << e.what());
  }

  DBImportObjPtr<CDCWireHitRequirements> dbWireHitReq;
  dbWireHitReq.construct(tree);

  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);
  dbWireHitReq.import(iov);

  B2RESULT("CDCWireHit requirements imported to database.");
}

void CDCDatabaseImporter::printCDCWireHitRequirements() const
{

  DBObjPtr<CDCWireHitRequirements> dbWireHitReq;
  if (dbWireHitReq.isValid()) {
    dbWireHitReq->dump();
  } else {
    B2WARNING("DBObjPtr<CDCWireHitRequirements> not valid for the current run.");
  }
}

void CDCDatabaseImporter::importCDCCrossTalkLibrary(const std::string& rootFileName) const
{
  DBImportObjPtr<CDCCrossTalkLibrary> dbCDCCrossTalkLibrary;
  dbCDCCrossTalkLibrary.construct();

  TFile fIn = TFile(rootFileName.c_str());
  TTreeReader reader("my_ttree", &fIn);
  TTreeReaderValue<UChar_t> Board(reader, "Board");
  TTreeReaderValue<UChar_t> Channel(reader, "Channel");
  TTreeReaderValue<Short_t> Asic_ADC0(reader, "Asic_ADC0");
  TTreeReaderValue<Short_t> Asic_TDC0(reader, "Asic_TDC0");
  TTreeReaderValue<Short_t> Asic_TOT0(reader, "Asic_TOT0");
  TTreeReaderValue<Short_t> Asic_ADC1(reader, "Asic_ADC1");
  TTreeReaderValue<Short_t> Asic_TDC1(reader, "Asic_TDC1");
  TTreeReaderValue<Short_t> Asic_TOT1(reader, "Asic_TOT1");
  TTreeReaderValue<Short_t> Asic_ADC2(reader, "Asic_ADC2");
  TTreeReaderValue<Short_t> Asic_TDC2(reader, "Asic_TDC2");
  TTreeReaderValue<Short_t> Asic_TOT2(reader, "Asic_TOT2");
  TTreeReaderValue<Short_t> Asic_ADC3(reader, "Asic_ADC3");
  TTreeReaderValue<Short_t> Asic_TDC3(reader, "Asic_TDC3");
  TTreeReaderValue<Short_t> Asic_TOT3(reader, "Asic_TOT3");
  TTreeReaderValue<Short_t> Asic_ADC4(reader, "Asic_ADC4");
  TTreeReaderValue<Short_t> Asic_TDC4(reader, "Asic_TDC4");
  TTreeReaderValue<Short_t> Asic_TOT4(reader, "Asic_TOT4");
  TTreeReaderValue<Short_t> Asic_ADC5(reader, "Asic_ADC5");
  TTreeReaderValue<Short_t> Asic_TDC5(reader, "Asic_TDC5");
  TTreeReaderValue<Short_t> Asic_TOT5(reader, "Asic_TOT5");
  TTreeReaderValue<Short_t> Asic_ADC6(reader, "Asic_ADC6");
  TTreeReaderValue<Short_t> Asic_TDC6(reader, "Asic_TDC6");
  TTreeReaderValue<Short_t> Asic_TOT6(reader, "Asic_TOT6");
  TTreeReaderValue<Short_t> Asic_ADC7(reader, "Asic_ADC7");
  TTreeReaderValue<Short_t> Asic_TDC7(reader, "Asic_TDC7");
  TTreeReaderValue<Short_t> Asic_TOT7(reader, "Asic_TOT7");

  while (reader.Next()) {
    asicChannels record{
      asicChannel{*Asic_TDC0, *Asic_ADC0, *Asic_TOT0},
      asicChannel{*Asic_TDC1, *Asic_ADC1, *Asic_TOT1},
      asicChannel{*Asic_TDC2, *Asic_ADC2, *Asic_TOT2},
      asicChannel{*Asic_TDC3, *Asic_ADC3, *Asic_TOT3},
      asicChannel{*Asic_TDC4, *Asic_ADC4, *Asic_TOT4},
      asicChannel{*Asic_TDC5, *Asic_ADC5, *Asic_TOT5},
      asicChannel{*Asic_TDC6, *Asic_ADC6, *Asic_TOT6},
      asicChannel{*Asic_TDC7, *Asic_ADC7, *Asic_TOT7}
    };
    // Determine ADC of the signal
    UChar_t asicCh = *Channel % 8;
    Short_t ADC = record[asicCh].ADC;
    dbCDCCrossTalkLibrary->addAsicRecord(asicCh, ADC, record);
  }

  // Now also get the x-talk probability
  double probs[8196];
  TH1F* prob;
  fIn.GetObject("ProbXTalk", prob);
  for (size_t a = 1; a <= 8196; a += 1) {
    probs[a - 1] = prob->GetBinContent(a);
  }
  fIn.Close();
  dbCDCCrossTalkLibrary->setPCrossTalk(probs);

  dbCDCCrossTalkLibrary->dump(0);
  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);
  dbCDCCrossTalkLibrary.import(iov);
  B2RESULT("CDCCrossTalkLibrary requirements imported to database.");
}

void CDCDatabaseImporter::printCDCCrossTalkLibrary() const
{
  DBObjPtr<CDCCrossTalkLibrary> dbCDCCrossTalkLib;
  if (dbCDCCrossTalkLib.isValid()) {
    dbCDCCrossTalkLib->dump(1);
  } else {
    B2ERROR("DBObjPtr<CDCCrossTalkLibrary> not valid for the current run.");
  }
}

void CDCDatabaseImporter::testCDCCrossTalkLibrary(bool spotChecks) const
{
  DBObjPtr<CDCCrossTalkLibrary> dbCDCCrossTalkLib;

  if (dbCDCCrossTalkLib.isValid()) {

    if (! spotChecks) {
      B2INFO("Performing CDCCrossTalkLibrary checks");
      auto timer = new Utils::Timer("CDCCrossTalkLibrary checks took"); // use "new" to avoid cpp-check warning
      int counter = 0;
      int size = 0;
      for (Short_t ADC = 0; ADC < 8196; ADC += 1) {
        for (Short_t channel = 0; channel < 48; channel += 1) {
          for (size_t rep = 0; rep < 100; rep += 1) {
            auto xtalk = dbCDCCrossTalkLib->getLibraryCrossTalk(channel, 4999, ADC, 5, 0, false);
            counter += 1;
            size += xtalk.size();
          }
        }
      }
      B2INFO("CDCCrossTalkLibrary called " << counter << " times. Total number of cross talk hits " << size);
      delete timer;
      return;
    }


    Short_t ADC_spot_checks[5] = {2, 100, 500, 1000, 5000};
    for (auto ADC :  ADC_spot_checks) {
      B2INFO("CHECK ADC=" << ADC);

      size_t NRep = ADC < 50 ? 100 : 10;
      for (size_t rep = 0; rep < NRep; rep += 1) {
        auto xtalk = dbCDCCrossTalkLib->getLibraryCrossTalk(0, 4999, ADC, 5, 0, true);
        B2INFO("Size = " << xtalk.size());
        for (auto [channel, rec] : xtalk) {
          B2INFO("Channel:" << channel << " TDC,ADC,TOT:" << rec.TDC << "," << rec.ADC << "," << rec.TOT);
        }
      }
    }
  } else {
    B2ERROR("DBObjPtr<CDCCrossTalkLibrary> not valid for the current run.");
  }
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

