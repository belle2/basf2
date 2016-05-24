#include <daq/slc/copper/top/b2l_top_asic.h>
#include <daq/slc/copper/top/b2l_top.h>
#include <daq/slc/copper/top/topreg.h>
#include <daq/slc/copper/top/TOPFEE.h>

#include <daq/slc/copper/HSLB.h>

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/File.h>
#include <daq/slc/system/Time.h>

#include <daq/slc/base/StringUtil.h>

#include <unistd.h>
#include <fstream>
#include <map>

#define configDebug 0

namespace Belle2 {

  namespace TOPDAQ {

    int validWrAddrBitslip_pass1[] = {7};
    int validWrAddrBitslip_pass2[] = {6};
    int validEyeAlignBitslip[] = {13, 14, 15, 16};
    int validDataAlignBitslip[] = {13, 14, 15, 16};

    void write_irs_reg(HSLB& hslb, RCCallback& callback, int carNum, int asicNum,
                       int address, int value, int irsxwrite_retry)
    {
      unsigned int hexValue = address * 0x1000 + value;
      //int addr = b2l_fulladdr(CARRIER_IRSX_irsxDirect, carNum, asicNum);
      b2l_write(hslb, CARRIER_IRSX_irsxDirect, hexValue, carNum, asicNum);
      b2l_write(hslb, CARRIER_IRSX_irsxDirect, hexValue, carNum, asicNum);
      unsigned int ret = b2l_read(hslb, CARRIER_IRSX_irsxRegWordOut, carNum, asicNum);
      int count = irsxwrite_retry;
      if (count > 0) {
        while (ret != hexValue && count > 0) {
          b2l_write(hslb, CARRIER_IRSX_irsxDirect, hexValue, carNum, asicNum);
          b2l_write(hslb, CARRIER_IRSX_irsxDirect, 0xF0000, carNum, asicNum);
          ret = b2l_read(hslb, CARRIER_IRSX_irsxRegWordOut, carNum, asicNum);
          count = count - 1;
          if (ret != hexValue) {
            callback.log(LogFile::ERROR, "IRSX write fail on %d successive tries: expected 0x%x, got 0x%x",
                         irsxwrite_retry, hexValue, ret);
          }
        }
      }
    }

    std::map<std::string, float> ReadThreshold(int car, int asicnum, int ch, const std::string& datafile)
    {
      std::fstream fin(datafile.c_str());
      std::string line;
      std::map<std::string, float> m;
      while (getline(fin, line)) {
        StringList items = StringUtil::split(StringUtil::replace(StringUtil::replace(line, "\t", " "), "\n", " "), ' ');
        if (items.size() > 5) {
          if (atoi(items[0].c_str()) == car && atoi(items[1].c_str()) == asicnum && atoi(items[2].c_str()) == ch) {
            m.insert(std::map<std::string, float>::value_type("mean", atoi(items[3].c_str())));
            m.insert(std::map<std::string, float>::value_type("width", atoi(items[4].c_str())));
            m.insert(std::map<std::string, float>::value_type("3sigma", atoi(items[5].c_str())));
            return m;
          }
        }
      }
      return m;
    }

    int ReadFB(int car, int asicnum, const std::string& datafile)
    {
      std::fstream fin(datafile.c_str());
      std::string line;
      while (getline(fin, line)) {
        StringList items = StringUtil::split(StringUtil::replace(StringUtil::replace(line, "\t", " "), "\n", " "), ' ');
        if (items.size() > 2) {
          if (atoi(items[0].c_str()) == car && atoi(items[1].c_str()) == asicnum) {
            int value = atoi(items[2].c_str());
            LogFile::debug("Found sstoutFB value of: %d",  value);
            return value;
          }
        }
      }
      LogFile::error("not found...");
      return -1;
    }

    double dt0 = 0;
    void verify_dll_asic(HSLB& hslb, RCCallback& callback, int carrier, int asic)
    {
      dt0 = Time().get();
      //bool asic_pass = true;
      std::string asic_fail_msg;
      int retry_count = 1;

      // check monTiming values to see if DLL locked
      std::string monName[] = { "A1", "B1", "A2", "B2", "PHASE", "PHAB",
                                "SSPin", "WR_STRB", "SSTout", "SSToutFB"
                              };

      // good monTiming values (/10000 for 0.1 sec scaler)
      int goodMonVal[] = { 106, 106, 106, 106, 212, 106, 212, 212, 212, 212 };
      while (retry_count > 0) {
        int monVal[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        for (int mon = 0; mon < 8; mon++) {
          write_irs_reg(hslb, callback, carrier, asic, 179, mon);
          usleep(200000);
          monVal[mon] = b2l_read(hslb, CARRIER_IRSX_scalerMontiming, carrier, asic);
          LogFile::debug("%-10s %10d", monName[mon].c_str(), monVal[mon]);
          if (monVal[mon] / 10000 != goodMonVal[mon]) {
            LogFile::warning("Questionable montiming value");
          }
        }
        write_irs_reg(hslb, callback, carrier, asic, 179, 0x40);  // SSTout
        usleep(200000);
        monVal[8] = b2l_read(hslb, CARRIER_IRSX_scalerMontiming, carrier, asic);
        LogFile::debug("%-10s %10d", monName[8].c_str(), monVal[8]);
        if (monVal[8] / 10000 != goodMonVal[8]) {
          LogFile::warning("Questionable montiming value");
        }
        write_irs_reg(hslb, callback, carrier, asic, 179, 0x48);  // SSToutFB
        usleep(200000);
        monVal[9] = b2l_read(hslb, CARRIER_IRSX_scalerMontiming, carrier, asic);
        LogFile::debug("%-10s %10d", monName[9].c_str(), monVal[9]);
        if (monVal[9] / 10000 != goodMonVal[9]) {
          LogFile::warning("Questionable montiming value");
        }
        for (unsigned int i = 0; i < sizeof(monVal) / sizeof(int); i++) {
          if (monVal[i] == 0) {
            retry_count--;
            break;
          }
        }
        if (retry_count <= 0) {
          callback.log(LogFile::WARNING, "DLL failed to lock");
          //asic_pass = 0;
          //asic_fail_msg = asic_fail_msg + "DLL ";
        } else {
          LogFile::debug("DLL started successfully");
        }
        // get phase (sync between ASIC and FTSW)
        write_irs_reg(hslb, callback, carrier, asic, 179, 5);  // req'd to set phase
        // get current phase
        b2l_write(hslb, CARRIER_IRSX_trigWidth1, 0x00FF0000, carrier, asic);
        int curphase = b2l_read(hslb, CARRIER_IRSX_phaseRead, carrier, asic);
        LogFile::debug("  --> Current phase set to %d", curphase);

        LogFile::debug("Done!");
      }
    }

    void config1asic_FB(HSLB& hslb, RCCallback& callback, int carrier, int asic)
    {
      //bool asic_pass = true;   // assume ASIC passes (until it doesn't)
      //std::string asic_fail_msg = ""; // reason for failure

      dt0 = Time().get();

      LogFile::debug("Starting configuration of BS %d / carrier %d / ASIC %d",
                     hslb.get_finid(), carrier, asic);
      LogFile::debug("Configuring common IRSX registers");

      // set regLatch/regLoad values
      b2l_write(hslb, CARRIER_IRSX_regLoadPeriod, 0, carrier, asic);
      b2l_write(hslb, CARRIER_IRSX_regLatchPeriod, 0, carrier, asic);
      // modify here to try stuff
      // b2l_write(hslb, CARRIER_IRSX_regLoadPeriod, 2, carrier, asic)
      // b2l_write(hslb, CARRIER_IRSX_regLatchPeriod, 1, carrier, asic)

      // set dT values
      for (int i = 0; i < 128; i++) {
        // GSV mod -- write_irs_reg(hslb, callback, carrier, asic, i, 1800)  // dTtrim
        write_irs_reg(hslb, callback, carrier, asic, i, 0);  // dTtrim
      }
      LogFile::debug("Registers   0~128 done");

      int SCROD_ID = b2l_read(hslb, SCROD_AxiVersion_UserID) / 256;
      std::string thresholdfile = StringUtil::form("thresholds/SCROD%d.dat", SCROD_ID);
      LogFile::debug("threshold file:" + thresholdfile);
      if (!File::exist(thresholdfile)) {
        throw (HSLBHandlerException("Threshold file %s does not exist, exiting...", thresholdfile));
      }
      std::string FBfile = StringUtil::form("sstoutFB/SCROD%d.fb", SCROD_ID);
      if (!File::exist(FBfile)) {
        throw (HSLBHandlerException("SSTOUT_FB file %s does not exist, exiting...", FBfile));
      }

      // set channel trigger values
      for (int i = 0; i < 8; i++) {
        std::map<std::string, float> thrvalue = ReadThreshold(carrier, asic, i, thresholdfile);
        write_irs_reg(hslb, callback, carrier, asic, 128 + i * 4, int(thrvalue["mean"] - 3 * thrvalue["width"]));
        // TRGthresh  2016-02-17 BF: modified to lower threshold to avoid some bad fit values
        write_irs_reg(hslb, callback, carrier, asic, 129 + i * 4, 1500); // Vofs1
        write_irs_reg(hslb, callback, carrier, asic, 130 + i * 4, 2000); // Vofs2
        write_irs_reg(hslb, callback, carrier, asic, 131 + i * 4, 1300); // Wbias
      }
      LogFile::debug("Registers 128~159 done");

      // set VBias values
      write_irs_reg(hslb, callback, carrier, asic, 160, 1300);  // TBbias
      write_irs_reg(hslb, callback, carrier, asic, 161,  950);  // Vbias
      write_irs_reg(hslb, callback, carrier, asic, 162, 1100);  // Vbias2
      write_irs_reg(hslb, callback, carrier, asic, 163, 1300);  // ITbias
      LogFile::debug("Registers 160~163 done");

      // set Wbias for double triggers
      write_irs_reg(hslb, callback, carrier, asic, 164,  900);  // Ch 1&2
      write_irs_reg(hslb, callback, carrier, asic, 165,  900);  // Ch 3&4
      write_irs_reg(hslb, callback, carrier, asic, 166,  900);  // Ch 5&6
      write_irs_reg(hslb, callback, carrier, asic, 167,  900);  // Ch 7&8

      // set misc digial register - disable spy port for all ASICs in case one is enabled
      for (int x = 0; x < 4; x++) {
        write_irs_reg(hslb, callback, carrier, x, 168, irsx168_normal);
      }

      // configure Wilkinson comparators
      write_irs_reg(hslb, callback, carrier, asic, 169,  737);  // CMPbias2
      write_irs_reg(hslb, callback, carrier, asic, 170, 3112);  // PUbias
      write_irs_reg(hslb, callback, carrier, asic, 171, 1000);  // CMPbias

      // configure Vramp values -- correct
      write_irs_reg(hslb, callback, carrier, asic, 172, 1300);  // SBbias
      write_irs_reg(hslb, callback, carrier, asic, 173,    0);  // Vdischarge
      write_irs_reg(hslb, callback, carrier, asic, 174, 2200);  // ISEL
      write_irs_reg(hslb, callback, carrier, asic, 175, 1300);  // DBbias

      // configure DLL registers
      write_irs_reg(hslb, callback, carrier, asic, 176, 4090);  // VtrimT, fine feedback timing trim
      write_irs_reg(hslb, callback, carrier, asic, 177, 1300);  // Qbias -- 0 until DLL set
      write_irs_reg(hslb, callback, carrier, asic, 178, 1300);  // Vqbuff is Qbias buffer bias

      //Misc. Timing register
      //  (0)    1 = Sel0
      //  (1)    2 = Sel1
      //  (2)    4 = Sel2
      //  (3)    8 = Unused
      //  (4)   16 = Unused
      //  (5)   32 = SSTSEL
      //  (6)   64 = Time1Time2
      //  (7)  128 = CLR_PHASE
      //  (8)  256 = Unused
      //  (9)  512 = Unused
      // (10) 1024 = Unused
      // (11) 2048 = Unused
      write_irs_reg(hslb, callback, carrier, asic, 179,    0); // corresponding to A1

      // configure analog timebase control
      write_irs_reg(hslb, callback, carrier, asic, 180, 2700);  // VadjP
      write_irs_reg(hslb, callback, carrier, asic, 181, 3500);  // VAPbuff is VadjP buffer bias
      write_irs_reg(hslb, callback, carrier, asic, 182, 1680);  // VadjN
      write_irs_reg(hslb, callback, carrier, asic, 183, 3500);  // VANbuff // 1300 rec'd by Gary

      // configure digital timebase control
      write_irs_reg(hslb, callback, carrier, asic, 184,    0);  // WR_SYNC_LE
      write_irs_reg(hslb, callback, carrier, asic, 185,   30);  // WR_SYNC_TE
      write_irs_reg(hslb, callback, carrier, asic, 186,   92);  // SSPin_LE
      write_irs_reg(hslb, callback, carrier, asic, 187,   10);  // SSPin_TE
      write_irs_reg(hslb, callback, carrier, asic, 188,   38);  // S1_LE
      write_irs_reg(hslb, callback, carrier, asic, 189,   86);  // S1_TE
      write_irs_reg(hslb, callback, carrier, asic, 190,  120);  // S2_LE
      write_irs_reg(hslb, callback, carrier, asic, 191,   20);  // S2_TE
      write_irs_reg(hslb, callback, carrier, asic, 192,   45);  // PHASE_LE
      write_irs_reg(hslb, callback, carrier, asic, 193,   85);  // PHASE_TE
      write_irs_reg(hslb, callback, carrier, asic, 194,   95);  // WR_STRB_LE
      write_irs_reg(hslb, callback, carrier, asic, 195,   17);  // WR_STRB_TE

      // configure other registers
      write_irs_reg(hslb, callback, carrier, asic, 199, 1026);  // TPGreg
      write_irs_reg(hslb, callback, carrier, asic, 200, 2048); // LD_RD_ADDR
      write_irs_reg(hslb, callback, carrier, asic, 201,    0); // LOAD_SS
      write_irs_reg(hslb, callback, carrier, asic, 202,    1); // Jam_SS
      write_irs_reg(hslb, callback, carrier, asic, 252,    1); // CLR_Sync
      write_irs_reg(hslb, callback, carrier, asic, 253,    1); // CatchSpy

      //////////////////////////////////////////////////////////
      // from here, asic specific
      LogFile::debug("Configuring ASIC-specific IRSX registers");
      int FBvalue = ReadFB(carrier, asic, FBfile);
      //writeIrsReg(BS, carrier, asic, 196,  110)  # SSTout_FB
      write_irs_reg(hslb, callback, carrier, asic, 196,  FBvalue);  // SSTout_FB
      // 110 default rec'd by Gary,this will be ASIC-dependent

      // set thresholds
      // will be ASIC-dependent

      //////////////////////////////////////////////////////////
      // switch to DDL

      LogFile::debug("Setting up DLL");

      // number of attempts
      int retry_count = 1;

      while (retry_count > 0) {
        // wait for voltage to stabilize
        usleep(100000);

        // adjust Qbias and disable VANbuff for DLL operation
        //Uncomment for nominal!!!!
        write_irs_reg(hslb, callback, carrier, asic, 177, 1300);  // Qbias, turn on Qbias
        write_irs_reg(hslb, callback, carrier, asic, 183,    0);  // VANbuff, turn off VadjN buffer bias

        // check monTiming values to see if DLL locked
        // check monTiming values to see if DLL locked
        std::string monName[] = { "A1", "B1", "A2", "B2", "PHASE", "PHAB",
                                  "SSPin", "WR_STRB", "SSTout", "SSToutFB"
                                };
        int monVal[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

        // good monTiming values (/10000 for 0.1 sec scaler)
        int goodMonVal[] = {106, 106, 106, 106, 212, 106, 212, 212, 212, 212 };

        for (int mon = 0; mon < 8; mon++) {
          write_irs_reg(hslb, callback, carrier, asic, 179, mon);
          usleep(200000);
          monVal[mon] = b2l_read(hslb, CARRIER_IRSX_scalerMontiming, carrier, asic);
          LogFile::debug("%-10s %10d", monName[mon], monVal[mon]);
          if (monVal[mon] / 10000 != goodMonVal[mon]) {
            //monVal[mon] = 0 // mark as "bad"
            callback.log(LogFile::WARNING, "Questionable montiming value");
          }
        }

        write_irs_reg(hslb, callback, carrier, asic, 179, 0x40);  // SSTout
        // time.sleep(0.4); -- GSV mod
        usleep(200000);
        monVal[8] = b2l_read(hslb, CARRIER_IRSX_scalerMontiming, carrier, asic);
        LogFile::debug("%-10s %10d", monName[8], monVal[8]);
        if (monVal[8] / 10000 != goodMonVal[8]) {
          //monVal[8] = 0 // mark as "bad"
          callback.log(LogFile::WARNING, "Questionable montiming value");
        }
        write_irs_reg(hslb, callback, carrier, asic, 179, 0x48);  // SSToutFB
        // time.sleep(0.4); -- GSV mod
        usleep(200000);
        monVal[9] = b2l_read(hslb, CARRIER_IRSX_scalerMontiming, carrier, asic);
        LogFile::debug("%-10s %10d", monName[9], monVal[9]);
        if (monVal[9] / 10000 != goodMonVal[9]) {
          //monVal[9] = 0 // mark as "bad"
          callback.log(LogFile::WARNING, "Questionable montiming value");
        }
        bool found = false;
        for (unsigned int i = 0; i < sizeof(monVal) / sizeof(int); i++) {
          if (monVal[i] == 0) {
            LogFile::debug("DLL failed to lock, retrying...");
            retry_count--;
            found = true;
            break;
          }
        }
        if (!found) {
          break;
        }
      }
      //            // reset parameters
      //            write_irs_reg(hslb, callback, carrier, asic, 183, 1300); // VANbuff
      //            write_irs_reg(hslb, callback, carrier, asic, 177,    0);  // Qbias
      //            time.sleep(1)

      //        else:
      //            break

      if (retry_count == 0) {
        //print_error("DLL failed to lock, exiting", 1)
        callback.log(LogFile::WARNING, "DLL failed to lock");
        //asic_pass = 0;
        //asic_fail_msg = asic_fail_msg + "DLL ";
      } else {
        LogFile::debug("DLL started successfully");
      }

      //////////////////////////////////////////////////////////
      // configure WrAddr bitslip

      LogFile::debug("Configuring WrAddr bitslip");

      // set up readout for bitslip alignment
      const int target = 0x000000A5;
      b2l_write(hslb, CARRIER_IRSX_wrAddrMode, wrAddrMode_Calibration, carrier, asic);
      b2l_write(hslb, CARRIER_IRSX_wrAddrFixed, target, carrier, asic);

      // enable Spy port
      write_irs_reg(hslb, callback, carrier, asic, 168, irsx168_spy);    // done here

      // timeout for attempts that found NO valid bitslip
      int notfound_count = 50;

      int bitslip = -1;
      while (bitslip < 0) {
        // clear serial interface sync
        write_irs_reg(hslb, callback, carrier, asic, 252, 1); // CLR_Sync
        usleep(100000);
        // scan through bitslip values
        bool success = false;
        for (unsigned int i = 0; i < sizeof(validWrAddrBitslip_pass1) / sizeof(int); i++) {
          bitslip = validWrAddrBitslip_pass1[i];
          b2l_write(hslb, CARRIER_IRSX_wrAddrBitSlip, bitslip, carrier, asic);
          // usleep(200000);;  // allow new serial interface to settle
          usleep(100000);  // allow new serial interface to settle
          int spy = b2l_read(hslb, CARRIER_IRSX_wrAddrSpy, carrier, asic);
          if (configDebug) {
            LogFile::debug("    %2d:  %s  0x%08x", bitslip, spy, target);
          }
          if (spy == target) {
            // found match, break out of loop
            success = true;
            break;
          }
        }
        if (!success) { // (for loop); reached end of loop = didn't find match
          bitslip = -1;
          notfound_count = notfound_count - 1;
          if (notfound_count == 0) {
            callback.log(LogFile::WARNING, "No wrAddr bitslip not found in pass1, trying pass2 values");
            break;
          }
        }
      }

      if (bitslip == -1) {
        notfound_count = 50;
        while (bitslip < 0) {
          // clear serial interface sync
          write_irs_reg(hslb, callback, carrier, asic, 252, 1);
          // usleep(200000); -- GSV mod
          usleep(100000);
          // scan through bitslip values
          bool success = false;
          for (unsigned int i = 0; i < sizeof(validWrAddrBitslip_pass2) / sizeof(int); i++) {
            bitslip = validWrAddrBitslip_pass2[i];
            b2l_write(hslb, CARRIER_IRSX_wrAddrBitSlip, bitslip, carrier, asic);
            // usleep(200000);  // allow new serial interface to settle
            usleep(100000);  // allow new serial interface to settle
            int spy = b2l_read(hslb, CARRIER_IRSX_wrAddrSpy, carrier, asic);
            if (configDebug) {
              LogFile::debug("    %2d:  %s  0x%08x", bitslip, spy, target);
            }
            if (spy == target) {
              // found match, break out of loop
              success = true;
              break;
            }
          }
          if (!success) { // (for loop); reached end of loop = didn't find match
            bitslip = -1;
            notfound_count = notfound_count - 1;
            if (notfound_count == 0) {
              callback.log(LogFile::ERROR, "No wrAddr bitslip not found in pass2, exiting", 1);
              //asic_pass = 0;
              //asic_fail_msg = asic_fail_msg + "WrAddr ";
            }
          }
        }
      }

      LogFile::debug("  --> Good wrAddr bitslip found at %d", bitslip);

      write_irs_reg(hslb, callback, carrier, asic, 168, irsx168_normal);

      //////////////////////////////////////////////////////////
      // align serial data output eye diagram

      LogFile::debug("Aligning serial data output eye diagram");

      // set up test pattern
      int tpg = 0xAAA;
      write_irs_reg(hslb, callback, carrier, asic, 199, tpg);  // TPGreg
      int spg = 0;
      b2l_write(hslb, CARRIER_IRSX_spgIn, spg, carrier, asic);

      // set up readout for eye alignment
      b2l_write(hslb, CARRIER_IRSX_readoutMode, readoutMode_Calibration, carrier, asic);
      //    print getframeinfo(currentframe())
      b2l_write(hslb, CARRIER_IRSX_hsDataDelay, 16, carrier, asic, 0);

      // timeout for attempts that found NO valid slip
      notfound_count = 20;

      bitslip = -1;
      while (bitslip < 0) {
        bool success = false;
        for (unsigned int i = 0; i < sizeof(validEyeAlignBitslip) / sizeof(int); i++) {
          bitslip = validEyeAlignBitslip[i];
          b2l_write(hslb, CARRIER_IRSX_readoutBitSlip, bitslip, carrier, asic, 0);
          //usleep(200000);
          usleep(100000);
          int data = b2l_read(hslb, CARRIER_IRSX_tpgData, carrier, asic);
          if (configDebug) {
            LogFile::debug("  %2d:  %s  0x%08x", bitslip, data, tpg);
          }
          if (data == tpg) {
            // found match, break out of loop
            success = true;
            break;
          }
        }
        if (!success) { // (for loop); reached end of loop = didn't find match
          callback.log(LogFile::WARNING, "Did not find valid eyeAlign bitslip, trying again");
          bitslip = -1;
          notfound_count = notfound_count - 1;
          if (notfound_count == 0) {
            callback.log(LogFile::ERROR, "No valid eyeAlign bitslip not found, try reconfiguring?", 1);
            //asic_pass = 0;
            //asic_fail_msg = asic_fail_msg + " eyeAlign";
          }
        }
      }
      LogFile::debug("  --> Good eyeAlign bitslip found at %d", bitslip);

      // not doing scan left/right for further eye alignment

      // set alignment value
      //    print getframeinfo(currentframe())
      b2l_write(hslb, CARRIER_IRSX_hsDataDelay, bitslip, carrier, asic, 0);

      //////////////////////////////////////////////////////////
      // configure high-speed data alignment

      LogFile::debug("Aligning high-speed data");

      // set up test pattern
      tpg = 0xCA1;
      write_irs_reg(hslb, callback, carrier, asic, 199, tpg);  // TPGreg
      spg = 0;
      b2l_write(hslb, CARRIER_IRSX_spgIn, spg, carrier, asic);
      b2l_write(hslb, CARRIER_IRSX_readoutMode, readoutMode_Calibration, carrier, asic);

      // timeout for attempts that found NO valid slip
      notfound_count = 20;

      bitslip = -1;
      if (bitslip < 0) {
        bool success = false;
        for (unsigned int i = 0; i < sizeof(validDataAlignBitslip) / sizeof(int); i++) {
          bitslip = validDataAlignBitslip[i];
          b2l_write(hslb, CARRIER_IRSX_readoutBitSlip, bitslip, carrier, asic);
          // usleep(200000);
          usleep(100000);
          int data = b2l_read(hslb, CARRIER_IRSX_tpgData, carrier, asic);
          if (configDebug) {
            LogFile::debug("  %2d:  %s  0x%08x", bitslip, data, tpg);
          }
          if (data == tpg) {
            // found match, break out of loop
            success = true;
            break;
          }
        }
        // (for loop); reached end of loop = didn't find match
        if (!success) {
          callback.log(LogFile::WARNING, "Did not find valid dataAlign bitslip, trying again");
          bitslip = -1;
          notfound_count = notfound_count - 1;
          if (notfound_count == 0) {
            callback.log(LogFile::ERROR, "No valid dataAlign bitslip not found, try reconfiguring?", 1);
            //asic_pass = 0;
            //asic_fail_msg = asic_fail_msg + " dataAlign";
          }
        }
      }

      LogFile::debug("  --> Good bitslip found at %d, adjusting by 12 to align with data", bitslip);
      b2l_write(hslb, CARRIER_IRSX_readoutBitSlip, bitslip - 12, carrier, asic);

      //////////////////////////////////////////////////////////
      // set biases to operating mode

      //    print("Verifying before Final dTtrim set")
      //    verifyDLL1asic(carrier, asic)
      //    write_irs_reg(hslb, callback, carrier, asic, 183, 3500);  // VANbuff try before changing the dTtrim back to non-0

      //LM: big difference with UH laser test - VtrimT adapted to 1500 at the end!!!
      write_irs_reg(hslb, callback, carrier, asic, 176, 1500);  // VtrimT, fine feedback timing trim

      for (int i = 0; i < 128; i++) {
        // GSV mod -- write_irs_reg(hslb, callback, carrier, asic, i, 1800);  // dTtrim
        write_irs_reg(hslb, callback, carrier, asic, i, 2000);  // dTtrim
        //write_irs_reg(hslb, callback, carrier, asic, i, 1800);  // dTtrim
        //write_irs_reg(hslb, callback, carrier, asic, i, 1500);  // dTtrim
        //write_irs_reg(hslb, callback, carrier, asic, i, 0);  // dTtrim
      }
      LogFile::debug("Final dT tuning  done");

      // write_irs_reg(hslb, callback, carrier, asic, 183, 0);  // VANbuff try before changing the dTtrim back to non-0

      // write_irs_reg(hslb, callback, carrier, asic, 161,  950);  // Vbias
      // write_irs_reg(hslb, callback, carrier, asic, 161,  900);  // Vbias
      // GSV mod -- write_irs_reg(hslb, callback, carrier, asic, 162, 1100);  // Vbias2
      // write_irs_reg(hslb, callback, carrier, asic, 162, 1000);  // Vbias2

      LogFile::debug("Setting final biases for operating mode");
      // set thresholds, etc. here?

      //////////////////////////////////////////////////////////
      // set phase (sync between ASIC and FTSW)

      write_irs_reg(hslb, callback, carrier, asic, 179, 5);  // req'd to set phase

      // print("Verifying before phase set")
      // verifyDLL1asic(carrier, asic)

      // get current phase
      b2l_write(hslb, CARRIER_IRSX_trigWidth1, 0x00FF0000, carrier, asic);
      int curphase = b2l_read(hslb, CARRIER_IRSX_phaseRead, carrier, asic);
      while (curphase != phase) {
        // configure timebase digital reg
        // clear phase A/B toggle
        write_irs_reg(hslb, callback, carrier, asic, 179, 128 + 5 + 64);
        write_irs_reg(hslb, callback, carrier, asic, 179,     5 + 64);

        sleep(1);
        write_irs_reg(hslb, callback, carrier, asic, 179, 5);

        // get current phase
        b2l_write(hslb, CARRIER_IRSX_trigWidth1, 0x00FF0000, carrier, asic);
        curphase = b2l_read(hslb, CARRIER_IRSX_phaseRead, carrier, asic);

        LogFile::debug("  --> Current phase set to %d", curphase);

        //////////////////////////////////////////////////////////
        // done!

        LogFile::debug("Done!");
        LogFile::debug("Verifying at the End");
        verify_dll_asic(hslb, callback, carrier, asic);
      }
    }

    void repairDLL1asic(HSLB& hslb, RCCallback& callback,
                        int carrier, int asic, int dTtrimVal)
    {
      // configure DLL registers
      write_irs_reg(hslb, callback, carrier, asic, 176, 4095); // VtrimT, fine feedback timing trim
      write_irs_reg(hslb, callback, carrier, asic, 178, 1300); // Vqbuff is Qbias buffer bias
      write_irs_reg(hslb, callback, carrier, asic, 183, 3500); // VANbuff # 1300 rec'd by Gary

      LogFile::debug("setting dTtrim value = %d", dTtrimVal);
      for (int i = 0; i < 128; i++) {
        // GSV mod -- write_irs_reg(hslb, callback, carrier, asic, i, 1800)  // dTtrim
        write_irs_reg(hslb, callback, carrier, asic, i, dTtrimVal);  // dTtrim
      }
      usleep(200000);

      write_irs_reg(hslb, callback, carrier, asic, 183, 0);  // VANbuff # 1300 rec'd by Gary
      check1asicConfig(hslb, callback, carrier, asic);
    }

    void check1asicConfig(HSLB& hslb, RCCallback& callback, int carrier, int asic)
    {
      //bool asic_pass = true;   // assume ASIC passes (until it doesn't)
      //std::string asic_fail_msg = "";// reason for failure

      dt0 = Time().get();

      LogFile::debug("Checking configuration of BS %d / carrier %d / ASIC %d",
                     hslb.get_finid(), carrier, asic);


      //////////////////////////////////////////////////////////
      // switch to DDL

      LogFile::debug("Verifying DLL");

      // number of attempts
      // int retry_count = 3  -- GSV mod
      int retry_count = 1;

      std::string monName[] = { "A1", "B1", "A2", "B2", "PHASE", "PHAB",
                                "SSPin", "WR_STRB", "SSTout", "SSToutFB"
                              };
      int monVal[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

      // good monTiming values (/10000 for 0.1 sec scaler)
      int goodMonVal[] = { 106, 106, 106, 106, 212, 106, 212, 212, 212, 212 };

      while (retry_count > 0) {
        // wait for voltage to stabilize
        // time.sleep(0.1)

        // adjust Qbias and disable VANbuff for DLL operation
        // Uncomment for nominal!!!!
        // writeIrsReg(BS, carrier, asic, 177, 1300)  // Qbias, turn on Qbias
        // writeIrsReg(BS, carrier, asic, 183,    0)  // VANbuff, turn off VadjN buffer bias

        // check monTiming values to see if DLL locked
        // check monTiming values to see if DLL locked
        for (int mon = 0; mon < 8; mon++) {
          write_irs_reg(hslb, callback, carrier, asic, 179, mon);
          // time.sleep(0.4) -- GSV mod
          usleep(200000);
          monVal[mon] = b2l_read(hslb, CARRIER_IRSX_scalerMontiming, carrier, asic);
          LogFile::debug("%-10s %10d", monName[mon], monVal[mon]);
          if (monVal[mon] / 10000 != goodMonVal[mon]) {
            monVal[mon] = -1; // mark as "bad"
            LogFile::debug("Questionable montiming value");
          }
          write_irs_reg(hslb, callback, carrier, asic, 179, 0x40);  // SSTout
        }
        // time.sleep(0.4) -- GSV mod
        usleep(200000);
        monVal[8] = b2l_read(hslb, CARRIER_IRSX_scalerMontiming, carrier, asic);
        LogFile::debug("%-10s %10d", monName[8], monVal[8]);
        if (monVal[8] / 10000 != goodMonVal[8]) {
          monVal[8] = -1; // mark as "bad"
          LogFile::debug("Questionable montiming value");
        }
        write_irs_reg(hslb, callback, carrier, asic, 179, 0x48);  // SSToutFB
        // time.sleep(0.4) -- GSV mod
        usleep(200000);
        monVal[9] = b2l_read(hslb, CARRIER_IRSX_scalerMontiming, carrier, asic);
        LogFile::debug("%-10s %10d", monName[9], monVal[9]);
        if (monVal[9] / 10000 != goodMonVal[9]) {
          monVal[9] = -1; // mark as "bad"
          LogFile::debug("Questionable montiming value");
        }
        bool found = false;
        for (unsigned int i = 0; i < sizeof(monVal) / sizeof(int); i++) {
          if (monVal[i] == 0) {
            LogFile::debug("DLL not locked");
            retry_count = retry_count - 1;
            found = true;
            break;
          }
        }
        if (!found) {
          break;
        }
      }

      if (retry_count == 0) {
        callback.log(LogFile::WARNING, "DLL failed to lock");
        //asic_pass = 0;
        //asic_fail_msg = asic_fail_msg + "DLL ";
      } else {
        bool found = false;
        for (unsigned int i = 0; i < sizeof(monVal) / sizeof(int); i++) {
          if (monVal[i] == -1) {
            callback.log(LogFile::WARNING, "Montime error");
            //asic_pass = -1;
            //asic_fail_msg = asic_fail_msg + "MONTIME ";
            break;
          }
        }
        if (!found) {
          LogFile::debug("DLL running successfully");
        }
      }
      //////////////////////////////////////////////////////////
      // done!
      LogFile::debug("Done!");
    }
  }
}
