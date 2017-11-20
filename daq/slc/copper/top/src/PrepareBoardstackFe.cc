#include "daq/slc/copper/top/B2L_common.h"
#include "daq/slc/copper/top/B2L_defs.h"
#include "daq/slc/copper/top/SCROD_PS_defs.h"
#include "daq/slc/copper/top/PrepareBoardstackFe.h"

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/copper/HSLB.h>
#include <daq/slc/database/DBObject.h>
#include <daq/slc/runcontrol/RCCallback.h>

#include <mgt/copper.h>
#include <mgt/libhslb.h>

#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <sstream>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

inline uint32_t SwapEndianess(const uint32_t& word)
{
  uint32_t out = (uint32_t)(((word & (0xFF000000)) >> 24) | ((word & (0x00FF0000)) >> 8) |
                            ((word & (0x0000FF00)) << 8) | ((word & (0x000000FF)) << 24));
  return out;
}

namespace PrepBoardstackFE {

  using namespace std;
  using namespace Belle2;

  const unsigned pedEvents = 8192;
  const unsigned wordsPedEvent = 512 + 6 + 6;//payload + header + footer
  map<const string, int> registerValueMap;

  void LoadDefaultRegisterValues()
  {
    registerValueMap.insert(pair<const string, int>("pedCalcAverage", 7));
    registerValueMap.insert(pair<const string, int>("cfdPercent", 50));
    registerValueMap.insert(pair<const string, int>("pedestalMeanMin", 600));
    registerValueMap.insert(pair<const string, int>("pedestalMeanMax", 1300));
    registerValueMap.insert(pair<const string, int>("pedestalRmsMin", 40));
    registerValueMap.insert(pair<const string, int>("pedestalRmsMax", 100));
  }
  void UpdateRegisterFromDatabase(Belle2::HSLB&, const Belle2::DBObject& databaseObject) {}

  void InitCallbacks(Belle2::HSLB& hslb, Belle2::RCCallback& callback)
  {
    //callback.add(new NSMVHandlerInt("ScrodfeMode", true, true, 3));
  }

  void PrepareBoardStack(Belle2::HSLB& hslb, const int mode, const unsigned cfdthreshold)
  {
    RCCallback dummyCallback;
    PrepareBoardStack(hslb, dummyCallback, mode, cfdthreshold);
  }

  int PrepareBoardStack(Belle2::HSLB& hslb, Belle2::RCCallback& callback, const int mode,
                        const unsigned cfdthreshold)
  {
    LoadDefaultRegisterValues();
    int nCarriers = Read_Register(hslb, SCROD_PS_carriersDetected, 0, 0);
    if (nCarriers  < 4) {
      cout << "WARNING: less than 4 carriers detected!" << endl;
    }
    if (nCarriers > 4) {
      cout << "ERROR: more than 4 carriers detected!?" << endl;
    }
    Write_Register(hslb, SCROD_PS_pedCalcMode, pedCalcMode_normal, 0, 0);
    Write_Register(hslb, SCROD_PS_pedCalcTimeout, 0x00008000, 0, 0);
    Write_Register(hslb, SCROD_PS_pedCalcNumAvg, registerValueMap["pedCalcAverage"], 0, 0);

    int retries = 2;

    while (retries > 0) {
      int pedstatus = TakePedestals(hslb);
      if (pedstatus == -1) return -1;
      int pedQuality = CheckPedestals(hslb);
      if (pedQuality == 0) {
        cout << "Pedestals passed quality check" << endl;
        break;
      } else if (pedQuality < 0) {
        cout << "Could not get pedestal data. Aborting check!" << endl;
        break;
      } else {
        cout << "Bad Pedestals. Trying pedestal accquisition again." << endl;
      }
      retries--;
    }

    if (retries == 0) cout << "Did not obtain clean pedestals." << endl;

    Write_Register(hslb, SCROD_PS_cfdThreshold, cfdthreshold, 0, 0);
    Write_Register(hslb, SCROD_PS_cfdPercent, registerValueMap["cfdPercent"], 0, 0);
    if (mode > 0) {
      Write_Register(hslb, SCROD_PS_featureExtMode, mode, 0, 0);
      cout << "Feature Extraction Mode = " << featureExtModeList[mode] << endl;
    } else {
      int mode_ext = 3;
      callback.get("ScrodfeMode", mode_ext);
      Write_Register(hslb, SCROD_PS_featureExtMode, mode_ext, 0, 0);
      cout << "Feature Extraction Mode = " << featureExtModeList[mode_ext] << endl;
    }

    return 0;
  }

  int TakePedestals(HSLB& hslb)
  {
    //start taking pedestals
    Write_Register(hslb, SCROD_PS_pedCalcStartStatus, 0xFF, 0, 0);
    cout << "Taking pedestals ..." << endl;
    int opsLeft = 1;
    int statusCode = 0;
    int prevStatusCode = 0;
    int prevOpsLeft = 0;
    while (opsLeft > 0) {
      sleep(3);
      int pedcalstatus = Read_Register(hslb, SCROD_PS_pedCalcStartStatus, 0, 0);
      prevStatusCode = statusCode;
      statusCode = pedcalstatus & ONLINE_PED_CALC_ERROR_CODE_MASK;
      opsLeft = pedcalstatus & ONLINE_PED_CALC_PROGRESS_MASK;
      cout << opsLeft << " Trigs Left -> " << prevStatusCode << endl;

      //handles cases where b2l returns timeout
      if (statusCode != 0 && prevStatusCode != 0) break;
      if (opsLeft == prevOpsLeft) {
        cout << "Checking for stalled calculation..." << endl;
        sleep(2);
        opsLeft = Read_Register(hslb, SCROD_PS_pedCalcStartStatus, 0, 0) & ONLINE_PED_CALC_PROGRESS_MASK;
        if (opsLeft == prevOpsLeft) {
          cout << "Pedestal Calculation is stalled, exiting" << endl;
          return -1;
        }
      }
      prevOpsLeft = opsLeft;
    }
    prevStatusCode = statusCode;
    statusCode = Read_Register(hslb, SCROD_PS_pedCalcStartStatus, 0, 0) & ONLINE_PED_CALC_ERROR_CODE_MASK;
    if (prevStatusCode != statusCode) {
      statusCode = Read_Register(hslb, SCROD_PS_pedCalcStartStatus, 0, 0) & ONLINE_PED_CALC_ERROR_CODE_MASK;
    }
    statusCode = statusCode >> ONLINE_PED_CALC_ERROR_CODE_OFFSET;

    //print any error status codes
    if (statusCode == PC_PASS) cout << "Pedestal Calc success" << endl;
    else if (statusCode == PC_WARN_DATA_ISSUES) {
      int ret = Read_Register(hslb, SCROD_PS_pedCalcErrorMask, 0, 0);
      int badDataMask = ret >> 16;
      int missingDataMask = ret & 0xFFFF;
      cout << "Warning: Data Issues" << endl;
      cout << "bad data on ASIC: " << hex << badDataMask << endl;
      cout << "missing data on ASIC: " << hex << missingDataMask << endl;
    } else if (statusCode == PC_FAIL_PS_HALT) {
      cout << "PS Critical Data Exception, PS needs restart" << endl;
      int ret = Read_Register(hslb, SCROD_PS_dataExceptionAddr, 0, 0);
      cout << "Exception Address: " << ret << endl;
      return -1;
    } else {
      cout << "Ped calc generic fail " << statusCode << endl;
      return -1;
    }

    return 0;
  }

  int CheckPedestals(HSLB& hslb)
  {
    cout << "Start Reading Pedestals" << endl;
    int use_slot = 1 << hslb.get_finid();
    int cprfd = OpenCopper(use_slot, 1);
    if (cprfd < 0) return -1;

    stringstream filename;
    filename << "pedestals_" << (Read_Register(hslb, SCROD_AxiVersion_UserID, 0, 0) >> 8) << ".dat";
    fstream pedestalFile(filename.str().c_str(), std::fstream::out);

    int copperDataOffset = 0;
    uint32_t* rawCopperData = new uint32_t[pedEvents * (wordsPedEvent)];
    unsigned short*  pedestalData = new unsigned short[pedEvents * 1024];
    Write_Register(hslb, SCROD_PS_readbackPedsStatus, 1, 0, 0);
    for (int event = 0; event < 8192; ++event) {
      int readStatus = ReadCopper(cprfd, rawCopperData, copperDataOffset, wordsPedEvent * 4);
      if (readStatus > 0) copperDataOffset += readStatus;
      else continue;
    }
    int dumpPedStatus = DumpPedestal(rawCopperData, pedestalData, pedestalFile);
    if (dumpPedStatus < 0) return -1;
    delete[] rawCopperData;

    int pedQual = CalculatePedestalProperties(pedestalData);

    delete[] pedestalData;
    pedestalFile.close();

    return pedQual;
  }

  int OpenCopper(const int& use_slot, const int& clef_1)
  {
    //copper device ioctl
    int cprfd = open("/dev/copper/copper", O_RDONLY);
    if (cprfd < 0) {
      cout << "can not open copper file handle" << endl;
      return -1;
    }
    if (ioctl(cprfd, CPRIOSET_LEF_WA_FF, &clef_1) < 0 ||
        ioctl(cprfd, CPRIOSET_LEF_WB_FF, &clef_1) < 0 ||
        ioctl(cprfd, CPRIOSET_LEF_WC_FF, &clef_1) < 0 ||
        ioctl(cprfd, CPRIOSET_LEF_WD_FF, &clef_1) < 0) {
      cout << "can not ioctl(LEF_Wx_FF) /dev/copper/copper" << endl;
      return -1;
    }

    //select hslb to use
    int ret = ioctl(cprfd, CPRIOSET_FINESSE_STA, &use_slot, sizeof(use_slot));
    if (ret < 0) {
      cout << "can not ioctl(FINESSE_STA) /dev/copper/copper" << endl;
      return -1;
    }

    //try opening hslb to check availability
    int finfd[4] = {0, 0, 0, 0};
    if (use_slot & 1) finfd[0] = openfn(0, O_RDWR, (char*) "readhs");
    if (use_slot & 2) finfd[1] = openfn(1, O_RDWR, (char*) "readhs");
    if (use_slot & 4) finfd[2] = openfn(2, O_RDWR, (char*) "readhs");
    if (use_slot & 8) finfd[3] = openfn(3, O_RDWR, (char*) "readhs");

    if (finfd[0] < 0 || finfd[1] < 0 || finfd[2] < 0 || finfd[3] < 0) {
      cout << "missing hslb on copper" << endl;
      return -1;
    }

    return cprfd;
  }

  int ReadCopper(const int& cprfd, uint32_t* const rawData, const unsigned& offset, const unsigned& maxReadPerEvent)
  {
    int ret;
    uint32_t header[13];
    int headlen = 0;
    unsigned datasize = 0;
    unsigned datalen = 0;

    //read header
    while (headlen < 13) {
      ret = read(cprfd, (char*)header + headlen, sizeof(header) - headlen);
      if (ret == 0 || (ret < 0 && errno != EINTR && errno != EAGAIN)) break;
      if (ret < 0) continue;
      headlen += ret;
    }
    if (ret < 0) {
      cout << "can not read from copper\n";
      return -1;
    }

    //check header
    if (header[0] != 0x7fff0008) {
      cout << "bad header[0]=" << hex << header[0] << " (!= 0x7fff0008)\n";
      for (int i = 0; i < 13; i++) {
        cout << "header " << i << " = " << hex << header[i] << "\n";
      }
      return -1;
    }
    if (header[7] != 0xfffffafa) {
      cout << "bad header[0]=" << hex << header[7] << " (!= 0xffaa0000)\n";
      return -1;
    }

    datasize = header[8] - 7;
    if (datasize != header[9] + header[10] + header[11] + header[12]) {
      printf("bad header[1]=%08x (!=%x+%x+%x+%x+7)\n", header[8],
             header[9], header[10], header[11], header[12]);
      return -1;
    }
    if (datasize <= 0 || datasize >= 64 * 1024) {
      cout << "bad data size = " << datasize << "\n";
      return -1;
    }
    datasize = (datasize + 3) * 4;

    //read data and footer
    while (datalen < datasize && datalen < maxReadPerEvent) {
      ret = read(cprfd, (char*)rawData + datalen + offset, datasize - datalen);
      if (ret == 0 || (ret < 0 && errno != EINTR && errno != EAGAIN)) break;
      if (ret < 0) continue;
      datalen += ret;
    }
    if (ret < 0) {
      cout << "can not read from copper\n";
      return -1;
    }
    if (datalen > maxReadPerEvent) {
      cout << "too many words in event. Skipping additional words" << datalen << " " << maxReadPerEvent << "\n";
    }

    //check footer
    if (rawData[(datasize + offset) / 4 - 3] != 0xfffff5f5) {
      cout << "bad footer = " << hex << rawData[(datasize + offset) / 4 - 3] << " (!= 0xfffff5f5)\n";
      return -1;
    }
    if (rawData[(datasize + offset) / 4 - 1] != 0x7fff0009) {
      cout << "bad footer = " << hex << rawData[(datasize + offset) / 4 - 1] << " (!= 0x7ffff0009)\n";
      return -1;
    }

    return datalen;
  }

  int DumpPedestal(uint32_t const* const rawData, unsigned short* const pedestalData,
                   std::fstream& pedestalFile)
  {
    const int hslb_hdr_size_words = 6;
    const int hslb_ftr_size_words = 6;
    const unsigned wordsInPacket = 512; //from software/PS7/common/generic_datatypes.h

    for (int event = 0; event < 8192; ++event) {

      //read header
      uint32_t header[hslb_hdr_size_words];
      memcpy(header, rawData, hslb_hdr_size_words);

      //read data
      int eventOffset = (hslb_hdr_size_words + hslb_ftr_size_words + wordsInPacket) * event;
      int arrayOffset = wordsInPacket * event * 2;
      unsigned wordCounter = 0;
      unsigned short doublePed[2];
      while (wordCounter < wordsInPacket) {
        uint32_t tempWord = SwapEndianess(rawData[wordCounter + hslb_hdr_size_words + eventOffset]);
        memcpy(doublePed, &tempWord, sizeof(doublePed));
        pedestalData[2 * wordCounter + arrayOffset] = doublePed[0];
        pedestalData[2 * wordCounter + arrayOffset + 1] = doublePed[1];
        pedestalFile << pedestalData[2 * wordCounter + arrayOffset] << "\n";
        pedestalFile << pedestalData[2 * wordCounter + arrayOffset + 1] << "\n";
        wordCounter++;
      }

      //check the footer
      uint32_t footer[hslb_ftr_size_words];
      //for(int i = hslb_hdr_size_words + wordsInPacket + eventOffset - 5; i < hslb_hdr_size_words + wordsInPacket + eventOffset + 5; ++i)cout<<hex<<rawData[i]<<endl;
      memcpy(footer, rawData + hslb_hdr_size_words + wordsInPacket + eventOffset, sizeof(footer));
      if ((footer[2] >> 16) != 0xFF55) {
        cout << "wrong fotter during peddump " << hex << (footer[2] >> 16) << " " << event << "\n";
        return -1;
      }
    }
    return 0;
  }

  int CalculatePedestalProperties(unsigned short const* const pedestalData)
  {
    const int totalSamplingCells = 4194304;
    double meanPedestal = 0;
    double meanRMS = 0;
    double rmsPedestal = 0;
    double rmsRMS = 0;

    for (int i = 0; i < totalSamplingCells; ++i) {
      meanPedestal += (double)pedestalData[i] / totalSamplingCells;
      meanRMS += (double)pedestalData[i + totalSamplingCells] / totalSamplingCells;
    }
    for (int i = 0; i < totalSamplingCells; ++i) {
      rmsPedestal += (pedestalData[i] - meanPedestal) * (pedestalData[i] - meanPedestal) / totalSamplingCells;
      rmsRMS += (pedestalData[i + totalSamplingCells] - meanRMS) * (pedestalData[i + totalSamplingCells] - meanRMS) / totalSamplingCells;
    }
    rmsPedestal = sqrt(rmsPedestal);
    rmsRMS = sqrt(rmsRMS);
    cout << "Pedestal Quality: " << meanPedestal << " " << meanRMS << " " << rmsPedestal << " " << rmsRMS << endl;

    if (registerValueMap["pedestalMeanMin"] < meanPedestal && registerValueMap["pedestalMeanMax"] > meanPedestal &&
        registerValueMap["pedestalRmsMin"] < rmsPedestal && registerValueMap["pedestalRmsMax"] > rmsPedestal) {
      return 0;
    } else {
      return 1;
    }
  }

}
