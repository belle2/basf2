#include "daq/slc/copper/klm/KLMFEE.h"
#include "daq/slc/copper/FEEHandler.h"

#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>

#include <mgt/hsreg.h>

#include <unistd.h>

using namespace Belle2;

KLMFEE::KLMFEE() : FEE("klm")
{
}

void KLMFEE::init(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  std::string vname = StringUtil::form("klm[%d].", hslb.get_finid());
  //callback.add(new FEE32Handler(vname + "test", callback, hslb, *this, 0x32));
  //callback.add(new KLMWindowHandler(vname + "coarse_start", callback, hslb, *this, 0x30));
  //callback.add(new KLMWindowHandler(vname + "coarse_stop", callback, hslb, *this, 0x31));
  //callback.add(new KLMWindowHandler(vname + "coarse_stop", callback, hslb, *this, 0x32));
  //callback.add(new KLMWindowHandler(vname + "coarse_stop", callback, hslb, *this, 0x33));

  callback.add(new NSMVHandlerInt(vname + ".coarse_start", true, false, 0));
  callback.add(new NSMVHandlerInt(vname + ".coarse_stop", true, false, 0));
  callback.add(new NSMVHandlerInt(vname + ".fine_start", true, false, 0));
  callback.add(new NSMVHandlerInt(vname + ".fine_stop", true, false, 0));
}

void KLMFEE::boot(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
}

void KLMFEE::load(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  // write access to register "reg0"
  /*LogFile::info("0x38>>0x%x", hslb.readfee32(0x38));
  hslb.writefee32(0x38, 10);
  int ret = hslb.readfee32(0x38);
  if (ret != 10) {
    LogFile::info("0x38>>%d", ret);
  } else {
    LogFile::warning("0x38>>%d!=10", ret);
  }
  int rcl = obj.getInt("rcl");
  hslb.writefee32(0x38, rcl);
  ret = hslb.readfee32(0x38);
  if (ret != rcl) {
    LogFile::info("0x38>>%d", ret);
  } else {
    LogFile::warning("0x38>>%d!=10", ret);
  }

  // writing stream file to FEE
  // no need to send slow control every run. It will overide the calibraiton for scin., Let's comment out for a while
  const std::string bitfile = obj.getText("bitfile");
  if (File::exist(bitfile)) {
    hslb.writestream(bitfile.c_str());
  } else {
    LogFile::error("file %s not exsits", bitfile.c_str());
  }
  sleep(2);


  // Write look back window values
  if (obj.hasObject("window")) {
    DBObject o_window(obj("window"));
    LogFile::debug("Write window values...");
    hslb.writefee32(0x30, o_window.getInt("val0")); //rpc window start
    hslb.writefee32(0x34, o_window.getInt("val0")); //write the window start to daq stream
    hslb.writefee32(0x31, o_window.getInt("val1"));
    hslb.writefee32(0x32, o_window.getInt("val2"));
    hslb.writefee32(0x33, o_window.getInt("val3"));
  }
  sleep(30);
  */
}

void KLMFEE::monitor(RCCallback& callback, HSLB& hslb)
{

  std::string vname = StringUtil::form("klm[%d].", hslb.get_finid());

  //rpc look back window coarse start
  callback.wait(5);
  int wins = hslb.readfee32(0X30);
  int win_start = wins & 0xffff;
  callback.set(vname + ".coarse_start", win_start);

  //rpc look back window coarse stop
  callback.wait(5);
  int winstop = hslb.readfee32(0X31);
  int win_stop = winstop & 0xffff;
  callback.set(vname + ".coarse_stop", win_stop);

  //rpc look back window fine start
  callback.wait(5);
  int fwins = hslb.readfee32(0X32);
  int fwin_start = fwins & 0xffff;
  callback.set(vname + ".fine_start", fwin_start);

  //rpc look back window fine stop
  callback.wait(5);
  int fwinstop = hslb.readfee32(0X33);
  int fwin_stop = fwinstop & 0xffff;
  callback.set(vname + ".fine_stop", fwin_stop);

}

extern "C" {
  void* getKLMFEE()
  {
    return new Belle2::KLMFEE();
  }
}
