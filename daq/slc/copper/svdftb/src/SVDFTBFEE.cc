#include "daq/slc/copper/svdftb/SVDFTBFEE.h"
#include "daq/slc/copper/svdftb/mt19937ar.h"
#include "daq/slc/copper/svdftb/mtb_param.h"
#include "daq/slc/copper/FEEHandler.h"

#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>

#include <mgt/hsreg.h>

namespace Belle2 {

  class FTBNWordHandler : public NSMVHandlerInt {
  public:
    FTBNWordHandler(const std::string& name, RCCallback& callback, HSLB& hslb, FEE& fee)
      : NSMVHandlerInt(name, true, true, 0), m_callback(callback), m_hslb(hslb), m_fee(fee) {}
    bool handleGetInt(int& val)
    {
      val = (m_hslb.readfee8(MT32_NWORD1) & 0xff) +
            ((m_hslb.readfee8(MT32_NWORD2) & 0xff) >> 8);
      return true;
    }
    bool handleSetText(int val)
    {
      m_hslb.writefee8(MT32_NWORD1, val & 0xFF);
      m_hslb.writefee8(MT32_NWORD2, (val >> 8) & 0xFF);
      return true;
    }
  private:
    RCCallback& m_callback;
    HSLB& m_hslb;
    FEE& m_fee;

  };

  class FTBStatusHandler : public NSMVHandlerInt {
  public:
    FTBStatusHandler(const std::string& name, RCCallback& callback, HSLB& hslb, FEE& fee, int bit)
      : NSMVHandlerInt(name, true, false, 0), m_callback(callback), m_hslb(hslb), m_fee(fee), m_bit(bit) {}
    bool handleGetInt(int& val)
    {
      val = (m_hslb.readfee8(STATUS) >> m_bit) & 0x1;
      return true;
    }
  private:
    RCCallback& m_callback;
    HSLB& m_hslb;
    FEE& m_fee;
    int m_bit;

  };


  class FTBRunModeHandler : public NSMVHandlerText {

  public:
    FTBRunModeHandler(const std::string& name, RCCallback& callback, HSLB& hslb, SVDFTBFEE& fee)
      : NSMVHandlerText(name, true, true, ""), m_callback(callback), m_hslb(hslb), m_fee(fee) {}
    bool handleGetText(std::string& val)
    {
      val = m_fee.getRunMode(m_hslb);
      return true;
    }
    bool handleSetText(const std::string& val)
    {
      m_fee.setRunMode(m_hslb, val);
      return NSMVHandlerText::handleSetText(val);
    }

  private:
    RCCallback& m_callback;
    HSLB& m_hslb;
    SVDFTBFEE& m_fee;

  };

}


using namespace Belle2;

SVDFTBFEE::SVDFTBFEE()
{
}

void SVDFTBFEE::init(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  int index;
  const int length = 4;
  unsigned long init[length];
  srand((unsigned int)time(NULL));
  for (index = 0; index < length; index++) {
    init[index] = (unsigned long)rand();
  }
  init_by_array(init, length);
  callback.add(new NSMVHandlerInitRand("initrand2"));
  std::string vname = StringUtil::form("svdftb[%d].", hslb.get_finid());
  callback.add(new FTBRunModeHandler(vname + "runmode_s", callback, hslb, *this));
  callback.add(new FEE8Handler(vname + "runmode", callback, hslb, *this, RUN_MODE));
  callback.add(new FEE8Handler(vname + "cfr", callback, hslb, *this, ADD_CFR));
  callback.add(new FEE8Handler(vname + "mbmr", callback, hslb, *this, ADD_MBMR));
  callback.add(new FEE8Handler(vname + "testreg", callback, hslb, *this, TEST_REG));
  callback.add(new FEE8Handler(vname + "fadc.id", callback, hslb, *this, FADC_ID));
  callback.add(new FEE8Handler(vname + "status", callback, hslb, *this, STATUS));
  callback.add(new FTBStatusHandler(vname + "status[0]", callback, hslb, *this, 0));
  callback.add(new FTBStatusHandler(vname + "status[1]", callback, hslb, *this, 1));
  callback.add(new FTBStatusHandler(vname + "status[2]", callback, hslb, *this, 2));
  callback.add(new FTBStatusHandler(vname + "status[3]", callback, hslb, *this, 3));
  callback.add(new FTBStatusHandler(vname + "status[4]", callback, hslb, *this, 4));
  callback.add(new FTBStatusHandler(vname + "status[5]", callback, hslb, *this, 5));
  callback.add(new FTBStatusHandler(vname + "status[6]", callback, hslb, *this, 6));
  callback.add(new FTBStatusHandler(vname + "status[7]", callback, hslb, *this, 7));
  callback.add(new FTBNWordHandler(vname + "mt32.nword", callback, hslb, *this));
  callback.add(new FEE8Handler(vname + "mt32.ctrl", callback, hslb, *this, MT32_CONTROL));
  callback.add(new FEE8Handler(vname + "mt32.nword[0]", callback, hslb, *this, MT32_NWORD1));
  callback.add(new FEE8Handler(vname + "mt32.nword[1]", callback, hslb, *this, MT32_NWORD2));
  callback.add(new FEE8Handler(vname + "mt32.data[0]", callback, hslb, *this, MT32_DATA1));
  callback.add(new FEE8Handler(vname + "mt32.data[1]", callback, hslb, *this, MT32_DATA2));
  callback.add(new FEE8Handler(vname + "mt32.data[2]", callback, hslb, *this, MT32_DATA3));
  callback.add(new FEE8Handler(vname + "mt32.data[3]", callback, hslb, *this, MT32_DATA4));
}

void SVDFTBFEE::boot(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{

}

void SVDFTBFEE::setRunMode(HSLB& hslb, const std::string& mode)
{
  LogFile::info(mode);
  if (mode == "fadc") {
    LogFile::info(mode);
    hslb.writefee8(RUN_MODE, 0x00);
    hslb.writefee8(ADD_CFR,  0xcf);
    hslb.writefee8(ADD_MBMR, 0x00);
  } else if (mode == "test1") {
    hslb.writefee8(RUN_MODE, 0x00);
    hslb.writefee8(ADD_CFR,  0x01);
    hslb.writefee8(ADD_MBMR, 0x00);
  } else if (mode == "test2") {
    hslb.writefee8(RUN_MODE, 0x00);
    hslb.writefee8(ADD_CFR,  0x01);
    hslb.writefee8(ADD_MBMR, 0x10);
  } else if (mode == "rand1") {
    hslb.writefee8(RUN_MODE, 0x02);
    hslb.writefee8(ADD_CFR,  0x81);
    hslb.writefee8(ADD_MBMR, 0x00);
  } else if (mode == "datcontest") {
    hslb.writefee8(RUN_MODE, 0x00);
    hslb.writefee8(ADD_CFR,  0xA5);
    hslb.writefee8(ADD_MBMR, 0x19);
  } else if (mode == "rand2") {
    hslb.writefee8(MT32_CONTROL, 0x03); // assert MT32_TBL_INIT
    for (unsigned int ptr = 0; ptr < N; ptr++) {
      /*
      LogFile::info("write: add=%03d, data=0x %02x_%02x_%02x_%02x", ptr,
        (((unsigned int)mt[ptr]) >> 24 & 0xff),
        (((unsigned int)mt[ptr]) >> 16 & 0xff),
        (((unsigned int)mt[ptr]) >> 8 & 0xff),
        (((unsigned int)mt[ptr]) >> 0 & 0xff));
      */
      hslb.writefee8(MT32_POINTER1, (ptr >> 0 & 0xff));
      hslb.writefee8(MT32_POINTER2, (ptr >> 8 & 0xff));
      hslb.writefee8(MT32_DATA1, (((unsigned int)mt[ptr]) >> 0 & 0xff));
      hslb.writefee8(MT32_DATA2, (((unsigned int)mt[ptr]) >> 8 & 0xff));
      hslb.writefee8(MT32_DATA3, (((unsigned int)mt[ptr]) >> 16 & 0xff));
      hslb.writefee8(MT32_DATA4, (((unsigned int)mt[ptr]) >> 24 & 0xff));
      hslb.writefee8(MT32_CONTROL, 0x05);
      hslb.writefee8(MT32_CONTROL, 0x04);
    }
    hslb.writefee8(MT32_CONTROL, 0x02);
    hslb.writefee8(MT32_CONTROL, 0x01);
    hslb.writefee8(MT32_CONTROL, 0x00);
    hslb.writefee8(RUN_MODE, 0x03);
    hslb.writefee8(ADD_CFR,  0x81);
    hslb.writefee8(ADD_MBMR, 0x00);
  }
}

std::string SVDFTBFEE::getRunMode(HSLB& hslb)
{
  int runmode = hslb.readfee8(RUN_MODE);
  int cfr = hslb.readfee8(ADD_CFR);
  int mbmr = hslb.readfee8(ADD_MBMR);
  if (runmode == 0x00 && cfr == 0xcf && mbmr == 0x00) {
    return "fadc";
  } else if (runmode == 0x00 && cfr == 0x01 && mbmr == 0x00) {
    return "test1";
  } else if (runmode == 0x00 && cfr == 0x01 && mbmr == 0x10) {
    return "test2";
  } else if (runmode == 0x02 && cfr == 0x81 && mbmr == 0x00) {
    return "rand1";
  } else if (runmode == 0x03 && cfr == 0x81 && mbmr == 0x00) {
    return "rand2";
  } else if (runmode == 0x00 && cfr == 0xa5 && mbmr == 0x19) {
    return "datcontest";
  }
  return "unknown";
}

void SVDFTBFEE::load(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  //  setRunMode(hslb, obj.getText("mode"));
  std::string vname = StringUtil::form("svdftb[%d].", hslb.get_finid());
  std::string mode;
  callback.get(vname + ".runmode_s", mode);
  setRunMode(hslb, mode);
}

void SVDFTBFEE::monitor(RCCallback& callback, HSLB& hslb)
{
  int status = hslb.readfee8(STATUS);
  std::string vname = StringUtil::form("svdftb[%d].", hslb.get_finid());
  callback.set(vname + "status", status);
  for (int i = 0; i < 8; i++) {
    callback.set(vname + StringUtil::form("status[%d]", i), (status >> i) & 0x1);
  }
}

bool SVDFTBFEE::NSMVHandlerInitRand::handleSetInt(int val)
{
  int index;
  const int length = 4;
  unsigned long init[length];
  srand((unsigned int)time(NULL));
  for (index = 0; index < length; index++) {
    init[index] = (unsigned long)rand();
  }
  init_by_array(init, length);
  return true;
}

extern "C" {
  void* getSVDFTBFEE()
  {
    return new Belle2::SVDFTBFEE();
  }
}

