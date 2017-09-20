#include "daq/slc/copper/top/B2L_common.h"
#include "daq/slc/copper/top/B2L_defs.h"
#include "daq/slc/copper/top/ConfigureBoardStack.h"

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/copper/HSLB.h>
#include <daq/slc/database/DBObject.h>
#include <daq/slc/runcontrol/RCCallback.h>

#include <fstream>
#include <iterator>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <ctime>



/**
 *Returns database strings for given scrod, carrier, asic
 */
std::string GetDBString(const std::string& registerName, const unsigned scrod,
                        const unsigned carrier, const unsigned asic)
{
  std::stringstream ss;
  ss << "Top" << scrod << ".Carrier" << carrier << ".Asic" << asic << "." << registerName;
  return ss.str();
}

/**
 *Returns database strings for given scrod, carrier
 */
std::string GetDBString(const std::string& registerName, const unsigned scrod,
                        const unsigned carrier)
{
  std::stringstream ss;
  ss << "Top" << scrod << ".Carrier" << carrier << "." << registerName;
  return ss.str();
}

template<typename Out>
void split(const std::string& s, char delim, Out result)
{
  std::stringstream ss;
  ss.str(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    *(result++) = item;
  }
}

/**
 *Split string at delimiter and return vector with substrings
 */
std::vector<std::string> split(const std::string& s, char delim)
{
  std::vector<std::string> elems;
  split(s, delim, std::back_inserter(elems));
  return elems;
}

/**
 *Construct path to threshold/feedback file for given scrod id
 */
std::string ConstructFilePathString(const std::string& prefix, const std::string& postfix, const int scrodID)
{
  std::stringstream ss;
  ss << prefix << scrodID << postfix;
  return ss.str();
}

namespace ConfigBoardstack {

  using namespace std;
  using namespace Belle2;

  bool useExternalLogFile = false;
  fstream externalLogFile;
  map<string, int> registerValueMap;

  //thresholds and feadback values read from files
  map<int, ThresholdData> thresholdData;
  map<int, int> feadbackData;

  void LoadDefaultRegisterValues(Belle2::HSLB& hslb)
  {
    unsigned scrod = hslb.get_finid();
    for (unsigned carrier = 0; carrier < 4; ++carrier) {
      registerValueMap.insert(pair<string, int>(GetDBString("CarrierVPED", scrod, carrier), 0x5c28));
      //asic specific
      for (unsigned asic = 0; asic < 4; ++asic) {
        registerValueMap.insert(pair<string, int>(GetDBString("ITBias", scrod, carrier, asic), 1300));
        registerValueMap.insert(pair<string, int>(GetDBString("VBias", scrod, carrier, asic), 1100));
        registerValueMap.insert(pair<string, int>(GetDBString("VBias2", scrod, carrier, asic), 950));
        registerValueMap.insert(pair<string, int>(GetDBString("Vofs1", scrod, carrier, asic), 1500));
        registerValueMap.insert(pair<string, int>(GetDBString("Vofs2", scrod, carrier, asic), 2000));
        registerValueMap.insert(pair<string, int>(GetDBString("WBias", scrod, carrier, asic), 1300));
        registerValueMap.insert(pair<string, int>(GetDBString("TBias", scrod, carrier, asic), 1300));
        registerValueMap.insert(pair<string, int>(GetDBString("WBiasDouble", scrod, carrier, asic), 900));
        registerValueMap.insert(pair<string, int>(GetDBString("CMPBias2", scrod, carrier, asic), 737));
        registerValueMap.insert(pair<string, int>(GetDBString("PUBias", scrod, carrier, asic), 3112));
        registerValueMap.insert(pair<string, int>(GetDBString("CMPBias", scrod, carrier, asic), 1000));
        registerValueMap.insert(pair<string, int>(GetDBString("SBBias", scrod, carrier, asic), 1300));
        registerValueMap.insert(pair<string, int>(GetDBString("Vdischarge", scrod, carrier, asic), 0));
        registerValueMap.insert(pair<string, int>(GetDBString("Isel", scrod, carrier, asic), 2200));
        registerValueMap.insert(pair<string, int>(GetDBString("DBBias", scrod, carrier, asic), 1300));
        registerValueMap.insert(pair<string, int>(GetDBString("VTrimt", scrod, carrier, asic), 4090));
        registerValueMap.insert(pair<string, int>(GetDBString("QBias", scrod, carrier, asic), 1300));
        registerValueMap.insert(pair<string, int>(GetDBString("VQbuff", scrod, carrier, asic), 1300));
        registerValueMap.insert(pair<string, int>(GetDBString("A1", scrod, carrier, asic), 0));
        registerValueMap.insert(pair<string, int>(GetDBString("VadjP", scrod, carrier, asic), 2700));
        registerValueMap.insert(pair<string, int>(GetDBString("VAPBuff", scrod, carrier, asic), 3500));
        registerValueMap.insert(pair<string, int>(GetDBString("VadjN", scrod, carrier, asic), 1450));
        registerValueMap.insert(pair<string, int>(GetDBString("VANbuff", scrod, carrier, asic), 3500));
        registerValueMap.insert(pair<string, int>(GetDBString("VANbuffStart", scrod, carrier, asic), 1300));
        registerValueMap.insert(pair<string, int>(GetDBString("WR_SYNC_LE", scrod, carrier, asic), 0));
        registerValueMap.insert(pair<string, int>(GetDBString("WR_SYNC_TE", scrod, carrier, asic), 30));
        registerValueMap.insert(pair<string, int>(GetDBString("SSPin_LE", scrod, carrier, asic), 92));
        registerValueMap.insert(pair<string, int>(GetDBString("SSPin_TE", scrod, carrier, asic), 10));
        registerValueMap.insert(pair<string, int>(GetDBString("S1_LE", scrod, carrier, asic), 38));
        registerValueMap.insert(pair<string, int>(GetDBString("S1_TE", scrod, carrier, asic), 86));
        registerValueMap.insert(pair<string, int>(GetDBString("S2_LE", scrod, carrier, asic), 120));
        registerValueMap.insert(pair<string, int>(GetDBString("S2_TE", scrod, carrier, asic), 20));
        registerValueMap.insert(pair<string, int>(GetDBString("Phase_LE", scrod, carrier, asic), 45));
        registerValueMap.insert(pair<string, int>(GetDBString("Phase_TE", scrod, carrier, asic), 85));
        registerValueMap.insert(pair<string, int>(GetDBString("WR_STRB_LE", scrod, carrier, asic), 95));
        registerValueMap.insert(pair<string, int>(GetDBString("WR_STRB_TE", scrod, carrier, asic), 17));
        registerValueMap.insert(pair<string, int>(GetDBString("TPGreg", scrod, carrier, asic), 1026));
        registerValueMap.insert(pair<string, int>(GetDBString("LD_RD_ADDR", scrod, carrier, asic), 2048));
        registerValueMap.insert(pair<string, int>(GetDBString("LOAD_SS", scrod, carrier, asic), 0));
        registerValueMap.insert(pair<string, int>(GetDBString("JAM_SS", scrod, carrier, asic), 1));
        registerValueMap.insert(pair<string, int>(GetDBString("CLR_SYNC", scrod, carrier, asic), 1));
        registerValueMap.insert(pair<string, int>(GetDBString("CatchSpy", scrod, carrier, asic), 1));
        registerValueMap.insert(pair<string, int>(GetDBString("VTrimtOp", scrod, carrier, asic), 1500));
        registerValueMap.insert(pair<string, int>(GetDBString("dTrimOp", scrod, carrier, asic), 2000));
      }
    }
  }

  void UpdateRegisterFromDatabase(const DBObject& databaseObject)
  {
    for (map<string, int>::iterator it = registerValueMap.begin(); it != registerValueMap.end(); ++it) {
      vector<string> dbSubStrings = split(it->first, '.');
      if (dbSubStrings.size() == 3) { //carrier register
        if (databaseObject.hasObject(dbSubStrings[0])) {
          const DBObject& top_dbObj(databaseObject(dbSubStrings[0]));
          if (top_dbObj.hasObject(dbSubStrings[1])) {
            const DBObject& carrier_dbObj(top_dbObj(dbSubStrings[1]));
            if (carrier_dbObj.hasValue(dbSubStrings[2])) {
              registerValueMap[it->first] = carrier_dbObj.getInt(dbSubStrings[2]);
            }
          }
        }
      } else if (dbSubStrings.size() == 4) { //asic register
        if (databaseObject.hasObject(dbSubStrings[0])) {
          const DBObject& top_dbObj(databaseObject(dbSubStrings[0]));
          if (top_dbObj.hasObject(dbSubStrings[1])) {
            const DBObject& carrier_dbObj(top_dbObj(dbSubStrings[1]));
            if (carrier_dbObj.hasObject(dbSubStrings[2])) {
              const DBObject& asic_dbObj(carrier_dbObj(dbSubStrings[2]));
              if (asic_dbObj.hasValue(dbSubStrings[3])) {
                registerValueMap[it->first] = asic_dbObj.getInt(dbSubStrings[3]);
              }
            }
          }
        }
      } else continue;
    }
  }

  void PrintRegisterValueMap()
  {
    cout << "Current values for registers\n\n";
    for (map<string, int>::const_iterator it = registerValueMap.begin(); it != registerValueMap.end(); ++it) {
      cout << it->first << " " << it->second << "\n";
    }
  }

  /****************************** Configuration Functions *********************/
  void ConfigureBoardStack(Belle2::HSLB& hslb)
  {
    useExternalLogFile = true;
    time_t now = time(0);
    tm* ltm = localtime(&now);
    char filename[128];
    sprintf(filename, "logs/BS%d_log_%d%d%d.txt", hslb.get_finid(), ltm->tm_hour, ltm->tm_mon, 1900 + ltm->tm_year);
    externalLogFile.open(filename, std::fstream::out);
    /*
    if (!externalLogFile.is_open()) {
      printf("error opening log file\n");
      return;
    }
    */
    externalLogFile << "Boardstack " << hslb.get_finid() << endl;
    Belle2::RCCallback dummyCallback;
    /*take care that map always exists for standalone applications
      for nsm2 application do LoadDefaultRegisterValues + UpdateRegisterFromDatabase
      from cprcontrold */
    LoadDefaultRegisterValues(hslb);
    ConfigureBoardStack(hslb, dummyCallback);
  }

  void ConfigureBoardStack(Belle2::HSLB& hslb, Belle2::RCCallback& callback)
  {
    Write_Register(hslb, SCROD_AxiCommon_fanoutResetL, 0, 0, 0);
    unsigned numberOfCarriers = GetNumberOfCarriers(hslb.get_finid());
    //read threshold and feadback data file to maps
    int scrod_id = Read_Register(hslb, SCROD_AxiVersion_UserID, 0, 0) >> 8;
    //string thresholdfile = ConstructFilePathString("/home/webert/uh-svn-repo/trunk/software/b2l/scripts_dev/thresholds/SCROD", ".dat",
    string thresholdfile = ConstructFilePathString("/home/usr/b2daq/b2top/thresholds/SCROD", ".dat",
                                                   scrod_id);
    thresholdData = ReadThreshold(thresholdfile);
    //string FBFile = ConstructFilePathString("/home/webert/uh-svn-repo/trunk/software/b2l/scripts_dev/sstoutFB/SCROD", ".fb", scrod_id);
    string FBFile = ConstructFilePathString("/home/usr/b2daq/b2top/sstoutFB/SCROD", ".fb", scrod_id);
    feadbackData = ReadFeadbackValue(FBFile);
    //configure the carriers
    for (unsigned carrier = 0; carrier < numberOfCarriers; ++carrier) {
      ConfigureCarrier(hslb, callback, carrier);
    }
    string vname = StringUtil::form("top[%d].", hslb.get_finid());
    callback.set(vname + "configured", 1);
  }

  void ConfigureCarrier(Belle2::HSLB& hslb, Belle2::RCCallback& callback, const unsigned carrier)
  {
    int scrod_id = Read_Register(hslb, SCROD_AxiVersion_UserID, 0, 0) / 16;
    int value = scrod_id + carrier;
    Write_Register(hslb, CARRIER_AxiCommon_irsxDataSelect, value, carrier, 0);
    //power on asics
    int status = JuiceCarrier(hslb, carrier, MapC(carrier));
    if (useExternalLogFile) {
      externalLogFile << "ASICs power status for carrier " << carrier << " is " << status << endl;
    } else {
      callback.log(LogFile::DEBUG, "power status for carrier %d is %d", carrier, status);
      if (status != 0) callback.log(LogFile::WARNING, "Error during powering of carrier %d", carrier);
    }

    //set vPed values
    string dbstring = GetDBString("CarrierVPED", hslb.get_finid(), carrier);
    Write_Register(hslb, CARRIER_PS_vPed0, registerValueMap[dbstring], carrier, 0);
    Write_Register(hslb, CARRIER_PS_vPed1, registerValueMap[dbstring], carrier, 0);
    Write_Register(hslb, CARRIER_PS_vPed2, registerValueMap[dbstring], carrier, 0);
    Write_Register(hslb, CARRIER_PS_vPed3, registerValueMap[dbstring], carrier, 0);

    //configure the asics
    for (int asic = 0; asic < 4; ++asic) {
      ConfigureAsic(hslb, callback, carrier, asic);
      cout << "Verifying DLL at the end." << endl;
      VerifyDLLAsic(hslb, callback, carrier, asic);
      string vname = StringUtil::form("top[%d].carrier[%d].asic[%d].configured", hslb.get_finid(), carrier, asic);
    }
    string vname = StringUtil::form("top[%d].carrier[%d].configured", hslb.get_finid(), carrier);
    callback.set(vname, 1);
  }

  void ConfigureAsic(Belle2::HSLB& hslb, Belle2::RCCallback& callback, const unsigned carrier,
                     const unsigned asic)
  {
    unsigned scrod = hslb.get_finid();
    cout << "==========================================================================================" << endl;
    cout << "Starting configuration of BS " << hslb.get_finid() << " carrier " << carrier << " asic " << asic << endl;
    cout << "==========================================================================================" << endl;
    if (useExternalLogFile) {
      externalLogFile << "Starting configuration of BS " << hslb.get_finid() << " carrier " << carrier << " asic " << asic << endl;
    } else {
      callback.log(LogFile::DEBUG, "Starting configuration of BS %d / carrier %d / ASIC %d",
                   hslb.get_finid(), carrier, asic);
      callback.log(LogFile::DEBUG, "Writing common IRSX registers");
    }

    Write_Register(hslb, CARRIER_IRSX_regLoadPeriod, 0, carrier, asic);
    Write_Register(hslb, CARRIER_IRSX_regLatchPeriod, 4, carrier, asic);
    JuiceASIC(hslb, carrier, asic, true);
    //set dt Values
    for (unsigned i = 0; i < 128; ++i) {
      Write_Asic_Direct_Register(hslb, carrier, asic, i, 0);
    }
    if (useExternalLogFile) {
      externalLogFile << "Registers 0 - 128 done" << endl;
    } else {
      callback.log(LogFile::DEBUG, "Registers 0 - 128 done");
    }

    for (unsigned channel = 0; channel < 8; ++channel) {
      int lookupValue = LookupValue(carrier, asic, channel);
      ThresholdData data = thresholdData[lookupValue];
      Write_Asic_Direct_Register(hslb, carrier, asic, 128 + channel * 4, (int)(data.mean - 3 * data.width));
      Write_Asic_Direct_Register(hslb, carrier, asic, 129 + channel * 4, registerValueMap[GetDBString("Vofs1", scrod, carrier,
                                 asic)]); //Vofs1
      Write_Asic_Direct_Register(hslb, carrier, asic, 130 + channel * 4, registerValueMap[GetDBString("Vofs2", scrod, carrier,
                                 asic)]); //Vofs2
      Write_Asic_Direct_Register(hslb, carrier, asic, 131 + channel * 4, registerValueMap[GetDBString("WBias", scrod, carrier,
                                 asic)]); //Wbias
    }
    if (useExternalLogFile) {
      externalLogFile << "Registers 128 - 159 done" << endl;
    } else {
      callback.log(LogFile::DEBUG, "Registers 128 - 159 done");
    }

    //set VBias values
    Write_Asic_Direct_Register(hslb, carrier, asic, 160, registerValueMap[GetDBString("TBias", scrod, carrier,
                               asic)]);//TBbias (influences TRGthresh, Vofs1, Vofs2, Wbias)

    //set Wbias for double triggers
    Write_Asic_Direct_Register(hslb, carrier, asic, 164, registerValueMap[GetDBString("WBiasDouble", scrod, carrier, asic)]);//Ch 1&2
    Write_Asic_Direct_Register(hslb, carrier, asic, 165, registerValueMap[GetDBString("WBiasDouble", scrod, carrier, asic)]);//Ch 3&4
    Write_Asic_Direct_Register(hslb, carrier, asic, 166, registerValueMap[GetDBString("WBiasDouble", scrod, carrier, asic)]);//Ch 5&6
    Write_Asic_Direct_Register(hslb, carrier, asic, 167, registerValueMap[GetDBString("WBiasDouble", scrod, carrier, asic)]);//Ch 7&8

    //LM set misc digital register  - force parallel read and positive edge for triggering
    for (unsigned i = 0; i < 4; ++i) {
      Write_Asic_Direct_Register(hslb, carrier, i, 168, 0x5);
    }

    //configure Wilkinson comparators
    Write_Asic_Direct_Register(hslb, carrier, asic, 169, registerValueMap[GetDBString("CMPBias2", scrod, carrier, asic)]); //CMPbias2
    Write_Asic_Direct_Register(hslb, carrier, asic, 170, registerValueMap[GetDBString("PUBias", scrod, carrier, asic)]); //PUbias
    Write_Asic_Direct_Register(hslb, carrier, asic, 171, registerValueMap[GetDBString("CMPBias", scrod, carrier, asic)]); //CMPbias

    //Vramp values
    Write_Asic_Direct_Register(hslb, carrier, asic, 172, registerValueMap[GetDBString("SBBias", scrod, carrier, asic)]); //SBbias
    Write_Asic_Direct_Register(hslb, carrier, asic, 173, registerValueMap[GetDBString("Vdischarge", scrod, carrier,
                               asic)]); //Vdischarge
    Write_Asic_Direct_Register(hslb, carrier, asic, 174, registerValueMap[GetDBString("Isel", scrod, carrier, asic)]); //ISEL
    Write_Asic_Direct_Register(hslb, carrier, asic, 175, registerValueMap[GetDBString("DBBias", scrod, carrier, asic)]); //DBbias

    // configure DLL registers
    Write_Asic_Direct_Register(hslb, carrier, asic, 176, registerValueMap[GetDBString("VTrimt", scrod, carrier, asic)]); //Vtrimt
    Write_Asic_Direct_Register(hslb, carrier, asic, 177, registerValueMap[GetDBString("QBias", scrod, carrier, asic)]); //Qbias
    Write_Asic_Direct_Register(hslb, carrier, asic, 178, registerValueMap[GetDBString("VQbuff", scrod, carrier, asic)]); //Vqbuff

    Write_Asic_Direct_Register(hslb, carrier, asic, 179, registerValueMap[GetDBString("A1", scrod, carrier,
                               asic)]); //corresponding to A1

    //configure analog timebase control
    Write_Asic_Direct_Register(hslb, carrier, asic, 180, registerValueMap[GetDBString("VadjP", scrod, carrier, asic)]);//VadjP
    Write_Asic_Direct_Register(hslb, carrier, asic, 181, registerValueMap[GetDBString("VAPBuff", scrod, carrier, asic)]);//VAPBuff
    Write_Asic_Direct_Register(hslb, carrier, asic, 182, registerValueMap[GetDBString("VadjN", scrod, carrier, asic)]);//VadjN
    Write_Asic_Direct_Register(hslb, carrier, asic, 183, registerValueMap[GetDBString("VANbuff", scrod, carrier,
                               asic)]);//VANbuff (1300 is recommended by Gary)

    //configure digital time base control
    Write_Asic_Direct_Register(hslb, carrier, asic, 184, registerValueMap[GetDBString("WR_SYNC_LE", scrod, carrier, asic)]);//WR_SYNC_LE
    Write_Asic_Direct_Register(hslb, carrier, asic, 185, registerValueMap[GetDBString("WR_SYNC_TE", scrod, carrier, asic)]);//WR_SYNC_TE
    Write_Asic_Direct_Register(hslb, carrier, asic, 186, registerValueMap[GetDBString("SSPin_LE", scrod, carrier, asic)]);//SSPin_LE
    Write_Asic_Direct_Register(hslb, carrier, asic, 187, registerValueMap[GetDBString("SSPin_TE", scrod, carrier, asic)]);//SSPin_TE
    Write_Asic_Direct_Register(hslb, carrier, asic, 188, registerValueMap[GetDBString("S1_LE", scrod, carrier, asic)]);//S1_LE
    Write_Asic_Direct_Register(hslb, carrier, asic, 189, registerValueMap[GetDBString("S1_TE", scrod, carrier, asic)]);//S1_TE
    Write_Asic_Direct_Register(hslb, carrier, asic, 190, registerValueMap[GetDBString("S2_LE", scrod, carrier, asic)]);//S2_LE
    Write_Asic_Direct_Register(hslb, carrier, asic, 191, registerValueMap[GetDBString("S2_TE", scrod, carrier, asic)]);//S2_TE
    Write_Asic_Direct_Register(hslb, carrier, asic, 192, registerValueMap[GetDBString("Phase_LE", scrod, carrier, asic)]);//Phase_LE
    Write_Asic_Direct_Register(hslb, carrier, asic, 193, registerValueMap[GetDBString("Phase_TE", scrod, carrier, asic)]);//Phase_TE
    Write_Asic_Direct_Register(hslb, carrier, asic, 194, registerValueMap[GetDBString("WR_STRB_LE", scrod, carrier, asic)]);//WR_STRB_LE
    Write_Asic_Direct_Register(hslb, carrier, asic, 195, registerValueMap[GetDBString("WR_STRB_TE", scrod, carrier, asic)]);//WR_STRB_TE

    //configure the other registers
    Write_Asic_Direct_Register(hslb, carrier, asic, 199, registerValueMap[GetDBString("TPGreg", scrod, carrier, asic)]);//TPGreg
    Write_Asic_Direct_Register(hslb, carrier, asic, 200, registerValueMap[GetDBString("LD_RD_ADDR", scrod, carrier, asic)]);//LD_RD_ADDR
    Write_Asic_Direct_Register(hslb, carrier, asic, 201, registerValueMap[GetDBString("LOAD_SS", scrod, carrier, asic)]);//LOAD_SS
    Write_Asic_Direct_Register(hslb, carrier, asic, 202, registerValueMap[GetDBString("JAM_SS", scrod, carrier, asic)]);//JAM_SS
    Write_Asic_Direct_Register(hslb, carrier, asic, 252, registerValueMap[GetDBString("CLR_SYNC", scrod, carrier, asic)]);//CLR_SYNC
    Write_Asic_Direct_Register(hslb, carrier, asic, 253, registerValueMap[GetDBString("CatchSpy", scrod, carrier, asic)]);//CatchSpy

    //ASIC specific configuration starting here
    int lookupValue = LookupValue(carrier, asic);
    int fb = feadbackData[lookupValue];
    if (fb > -1) Write_Asic_Direct_Register(hslb, carrier, asic, 196, fb);
    else Write_Asic_Direct_Register(hslb, carrier, asic, 196, 110);

    //start dll
    int retry_count = 1;

    string monName[10] = { "A1", "B1", "A2", "B2", "PHASE", "PHAB",
                           "SSPin", "WR_STRB", "SSTout", "SSToutFB"
                         };
    int goodMonVal[10] = { 106, 106, 106, 106, 212, 106, 212, 212, 212, 212 };
    //almost to identical to verify except for 2 write register commands -> refactor!
    while (retry_count != 0) {
      usleep(200000);
      StartDLLAsic(hslb, carrier, asic);

      int monVal[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

      for (unsigned mon = 0; mon < 8; ++mon) {
        Write_Asic_Direct_Register(hslb, carrier, asic, 179, mon);
        usleep(400000);
        monVal[mon] = Read_Register(hslb, CARRIER_IRSX_scalerMontiming, carrier, asic);
        printf("%-10s %10d\n", monName[mon].c_str(), monVal[mon]);
        if (monVal[mon] / 10000 != goodMonVal[mon]) {
          printf("Warning: Questionable mon timing value %d %d \n", monVal[mon] / 10000, goodMonVal[mon]);
        }
      }
      Write_Asic_Direct_Register(hslb, carrier, asic, 179, 0x40);//SSTout
      usleep(300000);
      monVal[8] = Read_Register(hslb, CARRIER_IRSX_scalerMontiming, carrier, asic);
      printf("%-10s %10d\n", monName[8].c_str(), monVal[8]);
      if (monVal[8] / 10000 != goodMonVal[8]) {
        printf("Warning: Questionable mon timing value %d \n", monVal[8] / 10000);
      }
      Write_Asic_Direct_Register(hslb, carrier, asic, 179, 0x48);//SSToutFB
      usleep(300000);
      monVal[9] = Read_Register(hslb, CARRIER_IRSX_scalerMontiming, carrier, asic);
      printf("%-10s %10d\n", monName[9].c_str(), monVal[9]);
      if (monVal[9] / 10000 != goodMonVal[9]) {
        printf("Warning: Questionable mon timing value\n");
      }

      bool dll_locked = true;
      for (unsigned i = 0; i < sizeof(monVal) / sizeof(int); ++i) {
        if (monVal[i] == 0) {
          dll_locked = false;
        }
      }
      if (dll_locked) break;
      else retry_count--;
    }

    if (retry_count == 0) {
      printf("Warning: DLL failed to lock on carrier %d asic %d\n on first try", carrier, asic);
    } else {
      printf("DLL locked\n");
    }

    //configure wr addr bitslip
    cout << "No need to configure WrAddr bitslip." << endl;
    int bitsliptarget = 6;
    Write_Register(hslb, CARRIER_IRSX_wrAddrBitSlip, bitsliptarget, carrier, asic);
    cout << "--> Set bitslip to " << bitsliptarget << endl;
    Write_Asic_Direct_Register(hslb, carrier, asic, 168, 0x5);

    //align serial data output eye diagram
    cout << "Aligning servial data output diagramm." << endl;
    int tpg = 0xAAA;
    Write_Asic_Direct_Register(hslb, carrier, asic, 199, tpg);
    int spg = 0;
    Write_Register(hslb, CARRIER_IRSX_spgIn, spg, carrier, asic);

    //setup readout for eye alignment
    Write_Register(hslb, CARRIER_IRSX_readoutMode, readoutMode_Calibration, carrier, asic);
    Raw_Write_Register_noReadBack(hslb, B2L_Address_Carrier_Asic(CARRIER_IRSX_hsDataDelay, carrier, asic), 16);
    int validDataAlignBitslip[4] = { 13, 14, 15, 16 };
    int validEyeAlignBitslip[4] = { 13, 14, 15, 16 };
    int bitslip = -1;
    int bitslipTryCount = 20;

    while (bitslip == -1) {
      for (unsigned i = 0; i < 4; ++i) {
        Raw_Write_Register_noReadBack(hslb, B2L_Address_Carrier_Asic(CARRIER_IRSX_readoutBitSlip, carrier, asic), validEyeAlignBitslip[i]);
        usleep(200000);
        int data = Read_Register(hslb, CARRIER_IRSX_tpgData, carrier, asic);
        if (data == tpg) {
          bitslip = validEyeAlignBitslip[i];
          break;//for loop
        }
      }
      if (bitslip < 0) {
        bitslipTryCount--;
        if (bitslipTryCount < 0) break; //while loop
        else cout << "No valid bitslip found, trying again!" << endl;
      } else {
        break;//while loop
      }
    }

    if (bitslip == -1) {
      if (useExternalLogFile) {
        externalLogFile << "WARNING: No valid bitslip found!" << endl;
      } else {
        callback.log(LogFile::WARNING, "No valid bitslip found!");
      }
    } else {
      cout << "--> God eye align bitslip found at " << bitslip << endl;
      Raw_Write_Register_noReadBack(hslb, B2L_Address_Carrier_Asic(CARRIER_IRSX_hsDataDelay, carrier, asic), bitslip);
    }

    //configure high speed data alignment
    cout << "Aligning high speed data." << endl;
    tpg = 0xCA1;
    Write_Asic_Direct_Register(hslb, carrier, asic, 199, tpg);
    spg = 0;
    Write_Register(hslb, CARRIER_IRSX_spgIn, spg, carrier, asic);
    Write_Register(hslb, CARRIER_IRSX_readoutMode, readoutMode_Calibration, carrier, asic);

    bitslip = -1;
    bitslipTryCount = 20;

    while (bitslip == -1) {
      for (unsigned i = 0; i < 4; ++i) {
        Write_Register(hslb, CARRIER_IRSX_readoutBitSlip, validDataAlignBitslip[i], carrier, asic);
        usleep(200000);
        int data = Read_Register(hslb, CARRIER_IRSX_tpgData, carrier, asic);
        if (data == tpg) {
          bitslip = validDataAlignBitslip[i];
          break;//for loop
        }
      }
      if (bitslip < 0) {
        bitslipTryCount--;
        if (bitslipTryCount < 0) break; //while loop
        else cout << "No valid data align bitslip found, trying again!" << endl;
      } else {
        break;//while loop
      }
    }

    if (bitslip == -1) {
      if (useExternalLogFile) {
        externalLogFile << "WARNING: No valid data align bitslip found!" << endl;
      } else {
        callback.log(LogFile::WARNING, "No valid data algin bitslip found!");
      }
    } else {
      cout << "--> Good bitslip found at " << bitslip << ", adjusting by 12 to align with data" << endl;
      Write_Register(hslb, CARRIER_IRSX_readoutBitSlip, bitslip - 12, carrier, asic);
    }

    //set biases to operating mode
    Write_Asic_Direct_Register(hslb, carrier, asic, 176, registerValueMap[GetDBString("VTrimtOp", scrod, carrier, asic)]);//VtrimT
    for (unsigned i = 0; i < 128; ++i) {
      Write_Asic_Direct_Register(hslb, carrier, asic, i, registerValueMap[GetDBString("dTrimOp", scrod, carrier, asic)]);//dTtrim
    }
    cout << "final dt tuning done." << endl;

    //set phase between asic and ftsw
    Write_Asic_Direct_Register(hslb, carrier, asic, 179, 5);
    Write_Register(hslb, CARRIER_IRSX_trigWidth1, 0x00FF0000, carrier, asic);
    int curphase = Read_Register(hslb, CARRIER_IRSX_phaseRead, carrier, asic);

    int phase = 0;
    int count = 0;
    int max_tries = 20;
    while (curphase != phase and count < max_tries) {
      //configure timebase digital reg
      //clear phase A/B toggle
      Write_Asic_Direct_Register(hslb, carrier, asic, 179, 128 + 5 + 64);
      Write_Asic_Direct_Register(hslb, carrier, asic, 179, 5 + 64);
      Write_Asic_Direct_Register(hslb, carrier, asic, 179, 5);

      //get current phase
      Write_Register(hslb, CARRIER_IRSX_trigWidth1, 0x00FF0000, carrier, asic);
      curphase = Read_Register(hslb, CARRIER_IRSX_phaseRead, carrier, asic);
      count++;
    }
    cout << "--> current phase set to " << curphase << endl;
    cout << endl;
  }

  /****************************** ASIC configuration **************************/
  int StartDLLAsic(Belle2::HSLB& hslb, const unsigned carrier, const unsigned asic)
  {

    return StartDLLAsic(hslb, carrier, asic, registerValueMap[GetDBString("VadjN", hslb.get_finid(), carrier, asic)]);
  }

  int StartDLLAsic(Belle2::HSLB& hslb, const unsigned carrier, const unsigned asic, const unsigned vadjn)
  {
    Write_Asic_Direct_Register(hslb, carrier, asic, 178, 0);//VQbuff
    Write_Asic_Direct_Register(hslb, carrier, asic, 183, registerValueMap[GetDBString("VANbuffStart", hslb.get_finid(), carrier,
                               asic)]);//VANBuff
    Write_Asic_Direct_Register(hslb, carrier, asic, 182, vadjn);//VadjN
    usleep(1000);

    Write_Asic_Direct_Register(hslb, carrier, asic, 178, registerValueMap[GetDBString("VQbuff", hslb.get_finid(), carrier, asic)]);
    Write_Asic_Direct_Register(hslb, carrier, asic, 183, 0);
    usleep(1000);

    return 0;
  }

  int VerifyDLLAsic(Belle2::HSLB& hslb, const unsigned carrier, const unsigned asic)
  {
    string monName[10] = { "A1", "B1", "A2", "B2", "PHASE", "PHAB",
                           "SSPin", "WR_STRB", "SSTout", "SSToutFB"
                         };
    int goodMonVal[10] = { 106, 106, 106, 106, 212, 106, 212, 212, 212, 212 };
    int retry_count = 1;
    int status = 0;
    while (retry_count > 0) {
      int monVal[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
      for (unsigned mon = 0; mon < 8; ++mon) {
        Write_Asic_Direct_Register(hslb, carrier, asic, 179, mon);
        usleep(200000);
        monVal[mon] = Read_Register(hslb, CARRIER_IRSX_scalerMontiming, carrier, asic);
        printf("%-10s %10d\n", monName[mon].c_str(), monVal[mon]);
        if (monVal[mon] / 10000 != goodMonVal[mon]) {
          printf("Warning: Questionable mon timing value\n");
          status = -1;
        }
      }
      Write_Asic_Direct_Register(hslb, carrier, asic, 179, 0x40);//SSTout
      usleep(200000);
      monVal[8] = Read_Register(hslb, CARRIER_IRSX_scalerMontiming, carrier, asic);
      printf("%-10s %10d\n", monName[8].c_str(), monVal[8]);
      if (monVal[8] / 10000 != goodMonVal[8]) {
        printf("Warning: Questionable mon timing value\n");
        status = -2;
      }
      Write_Asic_Direct_Register(hslb, carrier, asic, 179, 0x48);//SSToutFB
      usleep(200000);
      monVal[9] = Read_Register(hslb, CARRIER_IRSX_scalerMontiming, carrier, asic);
      printf("%-10s %10d\n", monName[9].c_str(), monVal[9]);
      if (monVal[9] / 10000 != goodMonVal[9]) {
        printf("Warning: Questionable mon timing value\n");
        status = -3;
      }

      bool dll_locked = true;
      for (unsigned i = 0; i < sizeof(monVal) / sizeof(int); ++i) {
        if (monVal[i] == 0) {
          dll_locked = false;
        }
      }
      if (dll_locked) break;
      else retry_count--;
    }

    if (retry_count == 0) {
      printf("WARNING: DLL failed to lock on carrier %d asic %d\n", carrier, asic);
    } else {
      printf("DLL locked\n");
    }
    Write_Asic_Direct_Register(hslb, carrier, asic, 179, 5);
    Write_Register(hslb, CARRIER_IRSX_trigWidth1, 0x00FF0000, carrier, asic);
    int curphase = Read_Register(hslb, CARRIER_IRSX_phaseRead, carrier, asic);
    printf("-->Current phase set to %d\n", curphase);

    return status;
  }


  void VerifyDLLAsic(Belle2::HSLB& hslb, Belle2::RCCallback& callback, const unsigned carrier,
                     const unsigned asic)
  {
    int dllstatus = VerifyDLLAsic(hslb, carrier, asic);
    int scrod = hslb.get_finid();
    string vname = StringUtil::form("top[%d].carrier[%d].asic[%d].", scrod, carrier, asic);
    if (dllstatus == 0) {
      callback.set(vname + "lockedDLL", 1);
    } else {
      callback.set(vname + "lockedDLL", 0);
      callback.log(LogFile::WARNING, StringUtil::form("DLL on scrod %d carrier %d asic %d did not lock", scrod, carrier, asic));
    }
  }

  /****************************** Power to ASIC *******************************/
  int JuiceCarrier(Belle2::HSLB& hslb, const unsigned carrier, const unsigned mask)
  {
    int status = JuiceCarrierNoASIC(hslb, carrier, mask);
    usleep(20000);
    for (int asic = 3; asic > -1; --asic) {
      if (carrier < 3 && asic == 3) cout << endl;
      if (mask & MapCA(carrier, asic)) {
        status |= JuiceASIC(hslb, carrier, asic, true);
        cout << "Juice Carrier 1 " << status << endl;
      } else {
        status |= JuiceASIC(hslb, carrier, asic, false);
        cout << "Juice Carrier 0 " << status << endl;
      }
    }
    return status;
  }

  int JuiceCarrierNoASIC(Belle2::HSLB& hslb, const unsigned carrier, const unsigned mask)
  {
    int other_mask = MapMC(mask, carrier);
    int pair_mask = 0;
    if (other_mask & 0xC) pair_mask |= 0x2;
    if (other_mask & 0x3) pair_mask |= 0x1;
    int status = Write_Register(hslb, CARRIER_PS_ampRegEn, pair_mask, carrier, 0);
    status |= Write_Register(hslb, CARRIER_PS_ampEn, other_mask, carrier, 0);
    int asicRegEn = Read_Register(hslb, CARRIER_PS_asicRegEn, carrier);
    status |= Write_Register(hslb, CARRIER_PS_asicRegEn, asicRegEn | other_mask, carrier, 0);
    return status;
  }

  int JuiceASIC(Belle2::HSLB& hslb, const unsigned carrier, const unsigned asic, bool enable)
  {
    int status = Write_Register(hslb, CARRIER_IRSX_regLoadPeriod, ideal_IRSX_regLoadPeriod, carrier, asic);
    status |= Write_Register(hslb, CARRIER_IRSX_regLatchPeriod, ideal_IRSX_regLatchPeriod, carrier, asic);
    status |= Write_Asic_Direct_Register(hslb, carrier, asic, 163, registerValueMap[GetDBString("ITBias", hslb.get_finid(), carrier,
                                         asic)]);//ITbias
    if (enable) {
      status |= Write_Asic_Direct_Register(hslb, carrier, asic, 161, registerValueMap[GetDBString("VBias", hslb.get_finid(), carrier,
                                           asic)]);//Vbias
      status |= Write_Asic_Direct_Register(hslb, carrier, asic, 162, registerValueMap[GetDBString("VBias2", hslb.get_finid(), carrier,
                                           asic)]);//Vbias2
    } else {
      status |= Write_Asic_Direct_Register(hslb, carrier, asic, 161, 0);
      status |= Write_Asic_Direct_Register(hslb, carrier, asic, 162, 0);
    }
    return status;
  }

  int MapCA(const unsigned carrier, const unsigned asic)
  {
    return 1 << (4 * carrier + asic);
  }

  int MapMC(const unsigned mask, const unsigned carrier)
  {
    return (mask >> (4 * carrier)) & 0xf;
  }

  int MapC(const unsigned carrier)
  {
    return 0xf << (4 * carrier);
  }
  /****************************** Reading config values from files ************/
  int LookupValue(const int carrier, const int asic, const int channel)
  {
    const int nAsic = 4;
    const int nChannel = 8;
    return carrier * (nAsic * nChannel) + asic * nChannel + channel;
  }

  int LookupValue(const int carrier, const int asic)
  {
    const int nAsic = 4;
    return carrier * nAsic + asic;
  }

  std::map<int, ThresholdData> ReadThreshold(const std::string& thresholdFilePath)
  {
    string line;
    map<int, ThresholdData> ret;
    fstream fin(thresholdFilePath.c_str());
    /*if (!fin.is_open()) {
      throw runtime_error("threshold file not found " + thresholdFilePath);
      }*/
    while (getline(fin, line)) {
      stringstream ss;
      ss << line;
      int asicID[3];
      float threshold[3];
      ss >> asicID[0] >> asicID[1] >> asicID[2] >> threshold[0] >> threshold[1] >> threshold[2];
      std::cout << " " <<  asicID[0] << " " <<  asicID[1] << " " <<  asicID[2] << " " <<  threshold[0] << " " <<  threshold[1] << " " <<
                threshold[2] << std::endl;
      int lookupValue = LookupValue(asicID[0], asicID[1], asicID[2]);
      ThresholdData data(threshold[0], threshold[1], threshold[2]);
      ret.insert(pair<int, ThresholdData>(lookupValue, data));
    }
    return ret;
  }

  std::map<int, int> ReadFeadbackValue(const std::string& fbFilePath)
  {
    fstream fin(fbFilePath.c_str());
    string line;
    unsigned values[3] = {0, 0, 0};
    std::map<int, int> ret;
    /*
    if (!fin.is_open()) {
      throw runtime_error("feadback file not found " + fbFilePath);
    }
    */
    while (getline(fin, line)) {
      stringstream ss(line);
      ss >> values[0] >> values[1] >> values[2];
      std::cout << " " <<  values[0] << " " <<  values[1] << " " <<  values[2] << std::endl;
      int lookupValue = LookupValue(values[0], values[1]);
      ret.insert(std::pair<int, int>(lookupValue, values[2]));
    }
    return ret;
  }

  /******************************* NSM Callbacks ******************************/
  void InitBoardStackCallbacks(Belle2::HSLB& hslb, Belle2::RCCallback& callback)
  {
    int scrod = hslb.get_finid();
    string vname = StringUtil::form("top[%d].", scrod);
    callback.add(new NSMVHandlerInt(vname + "configured", true, false, 0));
    unsigned connectedCarriers = GetNumberOfCarriers(scrod);
    LogFile::debug("%s:%d %u", __FILE__, __LINE__, connectedCarriers);
    for (unsigned carrier = 0; carrier < connectedCarriers; ++carrier) {
      InitCarrierCallbacks(hslb, callback, carrier);
    }
    LogFile::debug("%s:%d", __FILE__, __LINE__);
  }

  void InitCarrierCallbacks(Belle2::HSLB& hslb, Belle2::RCCallback& callback, const unsigned carrier)
  {
    int scrod = hslb.get_finid();
    string vname = StringUtil::form("top[%d].carrier[%d].", scrod, carrier);
    callback.add(new NSMVHandlerInt(vname + "configured", true, false, 0));
    //LogFile::debug("%s:%d", __FILE__, __LINE__);
    for (unsigned asic = 0; asic < 4; ++asic) {
      InitASICCallbacks(hslb, callback, carrier, asic);
    }
    //LogFile::debug("%s:%d", __FILE__, __LINE__);
  }

  void InitASICCallbacks(Belle2::HSLB& hslb, Belle2::RCCallback& callback, const unsigned carrier,
                         const unsigned asic)
  {
    int scrod = hslb.get_finid();
    string vname = StringUtil::form("top[%d].carrier[%d].asic[%d].", scrod, carrier, asic);
    //LogFile::debug("%s:%d", __FILE__, __LINE__);
    callback.add(new NSMVHandlerInt(vname + "configured", true, false, 0));
    callback.add(new NSMVHandlerInt(vname + "lockedDLL", true, false, 0));
    //LogFile::debug("%s:%d", __FILE__, __LINE__);
  }

}
