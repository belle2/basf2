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
#include <trg/ecl/dbobjects/TrgEclDatabaseImporter.h>

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
#include "trg/ecl/dbobjects/TRGECLFAMPara.h"
#include "trg/ecl/dbobjects/TRGECLTMMPara.h"
#include "trg/ecl/dbobjects/TRGECLETMPara.h"

#include "trg/ecl/TrgEclMapping.h"

#include <iostream>
#include <fstream>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/gzip.hpp>

using namespace std;
namespace Belle2 {

  TrgEclDatabaseImporter::TrgEclDatabaseImporter(): startExp(0), startRun(0), endExp(-1), endRun(-1)
  {}

  void TrgEclDatabaseImporter::importFAMParameter(std::string InputFileName, std::string InputFileSignalPDF,
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

    TrgEclMapping* _map = new TrgEclMapping();


    //  cout << "Import FAM Parameters  " << endl;
    DBImportArray<TRGECLFAMPara> fampara;
    //  fampara.construct();

    std::vector<int>  FPGAversion;
    std::vector<int>  TCId;
    std::vector<int>  FAMId;
    std::vector<int>  ChannelId;
    std::vector<int>  TEreconstruction;
    std::vector<int>  Threshold;
    std::vector<int>  Conversionfactor;
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
      stream >> FAMId[Id]  >> ChannelId[Id] >> FPGAversion [Id]  >> TEreconstruction[Id] >> Threshold[Id] >>  Conversionfactor[Id] >>
             Toffset[Id]  >> Wavemean[Id]  >> Wavesigma[Id];
      TCId[Id] = _map -> getTCIdFromFAMChannel(FAMId[Id], ChannelId[Id]);
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

    IntervalOfValidity iov(startExp, startRun, endExp, endRun);

    fampara.import(iov);

    delete _map ;
    B2RESULT("FAM parameters are imported to database.");

  }
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
    //  cout << "Import FAM Parameters  " << endl;
    DBImportArray<TRGECLTMMPara> tmmpara;
    //  fampara.construct();

    int FPGAversion;
    stream >> FPGAversion ;
    tmmpara.appendNew(FPGAversion);
    stream.close();
    //Import to DB
    IntervalOfValidity iov(startExp, startRun, endExp, endRun);

    tmmpara.import(iov);

    B2RESULT("TMM parameters are imported to database.");

  }


  void TrgEclDatabaseImporter::importETMParameter(std::string InputFileName)
  {
    std::ifstream stream;
    stream.open(InputFileName.c_str());
    if (!stream) {
      B2ERROR("openFile: " << InputFileName << " *** failed to open");
      return;
    }
    //  cout << "Import FAM Parameters  " << endl;
    DBImportArray<TRGECLETMPara> etmpara;

    int FPGAversion;
    int ADCto100MeV;
    int ELow;
    int EHigh;
    int ELum;
    int FWD2DBhabha[14];
    int BWD2DBhabha[14];
    int Bhabha3DThreshold[2];
    int LowMultiThreshold[4];
    int TriggerLatency;
    int ETMDelay;


    stream >> FPGAversion
           >> ADCto100MeV
           >> ELow
           >> EHigh
           >> ELum
           >> FWD2DBhabha[0]    >> BWD2DBhabha[0]
           >> FWD2DBhabha[1]    >> BWD2DBhabha[1]
           >> FWD2DBhabha[2]    >> BWD2DBhabha[2]
           >> FWD2DBhabha[3]    >> BWD2DBhabha[3]
           >> FWD2DBhabha[4]    >> BWD2DBhabha[4]
           >> FWD2DBhabha[5]    >> BWD2DBhabha[5]
           >> FWD2DBhabha[6]    >> BWD2DBhabha[6]
           >> FWD2DBhabha[7]    >> BWD2DBhabha[7]
           >> FWD2DBhabha[8]    >> BWD2DBhabha[8]
           >> FWD2DBhabha[9]    >> BWD2DBhabha[9]
           >> FWD2DBhabha[10]   >> BWD2DBhabha[10]
           >> FWD2DBhabha[11]   >> BWD2DBhabha[11]
           >> FWD2DBhabha[12]   >> BWD2DBhabha[12]
           >> FWD2DBhabha[13]   >> BWD2DBhabha[13]
           >> Bhabha3DThreshold[0]
           >> Bhabha3DThreshold[1]
           >> LowMultiThreshold[0]
           >> LowMultiThreshold[1]
           >> LowMultiThreshold[2]
           >> LowMultiThreshold[3]
           >> TriggerLatency
           >> ETMDelay;



    etmpara.appendNew(FPGAversion, ADCto100MeV, ELow, EHigh, ELum, FWD2DBhabha, BWD2DBhabha, Bhabha3DThreshold, LowMultiThreshold,
                      TriggerLatency, ETMDelay);

    stream.close();
    //Import to DB
    IntervalOfValidity iov(startExp, startRun, endExp, endRun);

    etmpara.import(iov);

    B2RESULT("ETM Parameters are imported to database.");

  }

  void TrgEclDatabaseImporter::printTCThreshold()
  {

    DBObjPtr<TRGECLFAMPara> para ;

    para->Dump();

  }
}
