/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <trg/ecl/dbobjects/TrgEclDatabaseImporter.h>

// framework - Database
#include <framework/database/DBObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBImportArray.h>
#include <framework/database/DBImportObjPtr.h>

// framework aux
#include <framework/logging/Logger.h>

// DB objects
#include "trg/ecl/dbobjects/TRGECLFAMPara.h"
#include "trg/ecl/dbobjects/TRGECLTMMPara.h"
#include "trg/ecl/dbobjects/TRGECLETMParameters.h"
#include "trg/ecl/dbobjects/TRGECLBadRun.h"

#include "trg/ecl/TrgEclMapping.h"

#include <iostream>
#include <fstream>
#include <map>

using namespace std;
using namespace Belle2;

TrgEclDatabaseImporter::TrgEclDatabaseImporter():
  m_startExp(0), m_startRun(0), m_endExp(-1), m_endRun(-1)
{}

void TrgEclDatabaseImporter::importFAMParameter(std::string InputFileName,
                                                std::string InputFileSignalPDF,
                                                std::string InputFileNoise)
{
  std::ifstream stream;
  stream.open(InputFileName.c_str());
  if (!stream) {
    B2ERROR("openFile: " << InputFileName << " *** failed to open");
    return;
  }
  std::ifstream stream1;
  stream1.open(InputFileSignalPDF.c_str());
  if (!stream1) {
    B2ERROR("openFile: " << InputFileSignalPDF << " *** failed to open");
    return;
  }

  std::ifstream stream2;
  stream2.open(InputFileNoise.c_str());
  if (!stream2) {
    B2ERROR("openFile: " << InputFileNoise << " *** failed to open");
    return;
  }

  TrgEclMapping* m_map = new TrgEclMapping();

  DBImportArray<TRGECLFAMPara> fampara;
  //  fampara.construct();

  std::vector<int>  FPGAversion;
  std::vector<int>  TCId;
  std::vector<int>  FAMId;
  std::vector<int>  ChannelId;
  std::vector<int>  TEreconstruction;
  std::vector<int>  Threshold;
  std::vector<double>  Conversionfactor;
  std::vector<int>  Toffset;
  std::vector<int>  Wavemean;
  std::vector<int>  Wavesigma;
  FPGAversion.clear();
  TCId.clear();
  FAMId.clear();
  ChannelId.clear();
  TEreconstruction.clear();
  Threshold.clear();
  Conversionfactor.clear();
  Toffset.clear();
  Wavemean.clear();
  Wavesigma.clear();

  FPGAversion.resize(624, 0);
  TCId.resize(624, 0);
  FAMId.resize(624, 0);
  ChannelId.resize(624, 0);
  TEreconstruction.resize(624, 0);
  Threshold.resize(624, 0);
  Conversionfactor.resize(624, 0);
  Toffset.resize(624, 0);

  Wavemean.resize(624, 0);
  Wavesigma.resize(624, 0);


  std::vector<std::vector<double> > SignalPDF;
  SignalPDF.clear();
  std::vector<std::vector<double> >  NoiseCovarianceMatrix;
  NoiseCovarianceMatrix.clear();
  SignalPDF.resize(624, std::vector<double>(8, 0));
  NoiseCovarianceMatrix.resize(624, std::vector<double>(78, 0));

  int Id = 0;
  while (!stream.eof()) {
    stream >> FAMId[Id]  >> ChannelId[Id] >> FPGAversion [Id]
           >> TEreconstruction[Id] >> Threshold[Id] >>  Conversionfactor[Id]
           >> Toffset[Id]  >> Wavemean[Id]  >> Wavesigma[Id];
    TCId[Id] = m_map->getTCIdFromFAMChannel(FAMId[Id], ChannelId[Id]);
    Id++;
  }
  stream.close();
  Id = 0;
  int line = 0;

  while (!stream1.eof()) {
    stream1 >> SignalPDF[Id][line];
    line++;
    if (line == 8) {
      line = 0;
      Id++;
    }

  }
  stream1.close();

  Id = 0;
  line = 0;
  while (!stream2.eof()) {
    stream2 >> NoiseCovarianceMatrix[Id][line];
    line++;
    if (line == 78) {
      line = 0;
      Id++;
    }

  }
  stream2.close();


  //Import to DB
  for (int iTCId = 0; iTCId < 624; iTCId++) {
    fampara.appendNew(FPGAversion[iTCId],
                      TCId[iTCId],
                      FAMId[iTCId],
                      ChannelId[iTCId],
                      TEreconstruction[iTCId],
                      Conversionfactor[iTCId],
                      Toffset[iTCId],
                      Threshold[iTCId],
                      Wavemean[iTCId],
                      Wavesigma[iTCId],
                      SignalPDF[iTCId],
                      NoiseCovarianceMatrix[iTCId]
                     );
  }

  IntervalOfValidity iov(m_startExp, m_startRun, m_endExp, m_endRun);

  fampara.import(iov);

  delete m_map ;
  B2RESULT("FAM parameters are imported to database.");

}
//
//
//
void TrgEclDatabaseImporter::importTMMParameter(std::string InputFileName)
{
  std::ifstream stream;
  stream.open(InputFileName.c_str());
  if (!stream) {
    B2ERROR("openFile: " << InputFileName << " *** failed to open");
    return;
  }

  DBImportArray<TRGECLTMMPara> tmmpara;

  int FPGAversion;
  stream >> FPGAversion ;
  tmmpara.appendNew(FPGAversion);
  stream.close();
  //Import to DB
  IntervalOfValidity iov(m_startExp, m_startRun, m_endExp, m_endRun);

  tmmpara.import(iov);

  B2RESULT("TMM parameters are imported to database.");

}
//
//
//
void TrgEclDatabaseImporter::importETMParameter(std::string InputFileName)
{

  std::ifstream stream;
  stream.open(InputFileName.c_str());
  if (!stream) {
    B2ERROR("openFile: " << InputFileName << " *** failed to open");
    return;
  }

  // get data from InputFileName
  std::vector<string> v_par_name;
  std::vector<double> v_par_value;
  string str_line;
  int cnt_par = 0;
  while (getline(stream, str_line)) {
    if (str_line.find("#")) {
      std::stringstream sss;
      sss << str_line;
      int    tmp_id;
      char   tmp_name[100];
      double tmp_value;
      if (sscanf(sss.str().data(),
                 "%i%99s%lf",
                 &tmp_id, tmp_name, &tmp_value) == 3) {
        string str_tmp_name = string(tmp_name);
        v_par_name.push_back(str_tmp_name);
        v_par_value.push_back(tmp_value);
        cnt_par++;
      }
    }
  }
  stream.close();

  B2INFO("[TrgEclDatabaseImporter] The number of parameters in "
         << InputFileName
         << " = "
         << cnt_par);

  DBImportObjPtr<TRGECLETMParameters> etmpara;
  etmpara.construct();

  etmpara->setnpar(cnt_par);

  int cnt_spare = 0;
  for (int iii = 0; iii < 300; iii++) {
    std::string tmp_parname;
    double tmp_parvalue = -1000;
    if (iii < cnt_par) {
      tmp_parname  = v_par_name[iii];
      tmp_parvalue = v_par_value[iii];
    } else {
      std::stringstream sss;
      sss << "spare" << std::setfill('0') << std::setw(3) << cnt_spare;
      tmp_parname  = sss.str();
      cnt_spare++;
    }
    etmpara->setparMap(tmp_parname, tmp_parvalue);
  }

  //Import to DB
  IntervalOfValidity iov(m_startExp, m_startRun, m_endExp, m_endRun);

  etmpara.import(iov);

  B2RESULT("ETM Parameters are imported to database.");

}
//
//
//
void TrgEclDatabaseImporter::importBadRunNumber(std::string InputFileName)
{
  std::ifstream stream;
  stream.open(InputFileName.c_str());
  if (!stream) {
    B2ERROR("openFile: " << InputFileName << " *** failed to open");
    return;
  }

  DBImportArray<TRGECLBadRun> badrun;

  int BadRunNumber;
  while (!stream.eof()) {

    stream >> BadRunNumber ;
    badrun.appendNew(BadRunNumber);
  }
  stream.close();

  //Import to DB
  IntervalOfValidity iov(m_startExp, m_startRun, m_endExp, m_endRun);

  badrun.import(iov);

  B2RESULT("BadRunList are imported to database.");

}
//
//
//
void TrgEclDatabaseImporter::printTCThreshold()
{

  DBObjPtr<TRGECLFAMPara> para ;

  para->Dump();

}
