#include "daq/slc/copper/svdftb/SVDFTBFEE.h"
#include "daq/slc/copper/svdftb/mt19937ar.h"
#include "daq/slc/copper/svdftb/mtb_param.h"
#include "daq/slc/copper/FEEHandler.h"

#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>

#include <mgt/hsreg.h>

using namespace Belle2;

SVDFTBFEE::SVDFTBFEE()
{
}

void SVDFTBFEE::init(RCCallback& callback, HSLB& hslb)
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
  callback.add(new FEE8Handler(vname + "run.mode", callback, hslb, *this, RUN_MODE));
  callback.add(new FEE8Handler(vname + "add.cfr", callback, hslb, *this, ADD_CFR));
  callback.add(new FEE8Handler(vname + "add.mbmr", callback, hslb, *this, ADD_MBMR));
}

void SVDFTBFEE::boot(HSLB& hslb,  const DBObject& obj)
{

}

void SVDFTBFEE::load(HSLB& hslb, const DBObject& obj)
{
  LogFile::info(obj.getText("mode"));
  if (obj.getText("mode") == "fadc") {
    hslb.writefee8(RUN_MODE, 0x00);
    hslb.writefee8(ADD_CFR,  0x80);
  } else if (obj.getText("mode") == "mt_test1") {
    hslb.writefee8(RUN_MODE, 0x00);
    hslb.writefee8(ADD_CFR,  0x01);
    hslb.writefee8(ADD_MBMR, 0x00);
  } else if (obj.getText("mode") == "mt_test2") {
    hslb.writefee8(RUN_MODE, 0x00);
    hslb.writefee8(ADD_CFR,  0x01);
    hslb.writefee8(ADD_MBMR, 0x10);
  } else if (obj.getText("mode") == "mt_rand1") {
    hslb.writefee8(RUN_MODE, 0x02);
    hslb.writefee8(ADD_CFR,  0x81);
  } else if (obj.getText("mode") == "mt_rand2") {
    hslb.writefee8(MT32_CONTROL, 0x03); // assert MT32_TBL_INIT
    for (unsigned int ptr = 0; ptr < N; ptr++) {
      hslb.writefee32(MT32_POINTER, mt[ptr]); // set data[7:0]
      printf("write: add=%03d, data=0x %02x_%02x_%02x_%02x\n", ptr,
             (((unsigned int)mt[ptr]) >> 24 & 0xff),
             (((unsigned int)mt[ptr]) >> 16 & 0xff),
             (((unsigned int)mt[ptr]) >> 8 & 0xff),
             (((unsigned int)mt[ptr]) >> 0 & 0xff));
      hslb.writefee8(MT32_CONTROL, 0x05);
      hslb.writefee8(MT32_CONTROL, 0x04);

    }
    hslb.writefee8(MT32_CONTROL, 0x02);
    hslb.writefee8(MT32_CONTROL, 0x01);
    hslb.writefee8(MT32_CONTROL, 0x00);
    hslb.writefee8(RUN_MODE, 0x03);
    hslb.writefee8(ADD_CFR,  0x81);
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
