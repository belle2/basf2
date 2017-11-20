#include "daq/slc/apps/cprcontrold/COPPERCallback.h"

#include "daq/slc/apps/cprcontrold/COPPERHandler.h"

#include <daq/slc/readout/ronode_status.h>

#include <daq/slc/database/DBObjectLoader.h>

#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Time.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <mgt/libhslb.h>
#include <mgt/hsreg.h>

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <sstream>

#define CRCERR     0x68
#define B2LCSR     0x69
#define HSD32      0x6e
#define HSA32      0x6f
#define HSLBHW     0x70
#define HSLBFW     0x71
#define HSLSTAT    0x72
#define HSLCONT    0x73
#define CCLK       0x74
#define CONF       0x75
#define FEEHW      0x76
#define FEESERIAL  0x77
#define FEEVER     0x79
#define FEETYPE    0x79
#define SERIAL_L   0x7b
#define SERIAL_H   0x7c
#define TYPE_L     0x7d
#define TYPE_H     0x7e
#define HSLBID     0x80
#define HSLBVER    0x81
#define RESET      0x82
#define B2LSTAT    0x83
#define RXDATA     0x84
#define FWEVT      0x85
#define FWCLK      0x86
#define CNTSEC     0x87
#define NWORD      0x88
#define ERRDET     0x8c
#define ERRPAT1    0x8d
#define ERRPAT2    0x8e
#define B2LCTIM    0x91
#define B2LTAG     0x92
#define B2LUTIM    0x93
#define B2LERUN    0x94

bool initialized = false;

namespace Belle2 {
  class FEELoad {
  public:
    FEELoad(COPPERCallback& callback, FEE& fee, HSLB& hslb, DBObject& obj)
      : m_callback(callback), m_fee(fee), m_hslb(hslb), m_obj(obj) {}
  public:
    void run()
    {
      m_fee.load(m_callback, m_hslb, m_obj);
    }
  private:
    COPPERCallback& m_callback;
    FEE& m_fee;
    HSLB& m_hslb;
    DBObject& m_obj;
  };
}

using namespace Belle2;

std::string popen(const std::string& cmd)
{
  char buf[1000];
  FILE* fp;
  if ((fp = ::popen(cmd.c_str(), "r")) == NULL) {
    perror("can not exec commad");
    exit(EXIT_FAILURE);
  }
  std::stringstream ss;
  while (!feof(fp)) {
    memset(buf, 0, 1000);
    fgets(buf, sizeof(buf), fp);
    ss << buf << std::endl;
  }
  pclose(fp);
  return ss.str();
}

COPPERCallback::COPPERCallback(FEE* fee[4], bool dummymode, bool disablefeconf)
{
  m_dummymode = dummymode;
  m_disablefeconf = disablefeconf;
  setTimeout(5);
  m_con.setCallback(this);
  for (int i = 0; i < 4; i++) {
    m_fee[i] = fee[i];
  }
  system("killall -9 basf2");
  m_force_boothslb = true;
}

COPPERCallback::~COPPERCallback() throw()
{
}

void COPPERCallback::getfee(HSLB& hslb, int& hwtype, int& serial, int& fwtype, int& fwver)
throw(HSLBHandlerException)
{
  hslb.writefn(HSREG_CSR, 0x05); /* reset address fifo */
  hslb.writefn(HSREG_CSR, 0x06); /* reset status register */
  int ret = 0;
  if ((ret = hslb.readfn(HSREG_STAT))) {
    return ;
  }
  hslb.writefn(HSREG_FEEHWTYPE, 0x02); /* dummy value write */
  hslb.writefn(HSREG_FEESERIAL, 0x02); /* dummy value write */
  hslb.writefn(HSREG_FEEFWTYPE, 0x02); /* dummy value write */
  hslb.writefn(HSREG_FEEFWVER,  0x02); /* dummy value write */
  hslb.writefn(HSREG_CSR, 0x07);

  hslb.hswait_quiet();

  hwtype = hslb.readfn(HSREG_FEEHWTYPE);
  serial = hslb.readfn(HSREG_FEESERIAL);
  fwtype = hslb.readfn(HSREG_FEEFWTYPE);
  fwver  = hslb.readfn(HSREG_FEEFWVER);

  serial = (serial | (hwtype << 8)) & 0xfff;
  hwtype = (hwtype >> 4) & 0x0f;
  fwtype  = (fwtype >> 4) & 0x0f;

  //hsp->feecrce = readfee8(fd, HSREG_CRCERR);
}

void COPPERCallback::initialize(const DBObject& obj) throw(RCHandlerException)
{
  LogFile::debug("COPPERCallback::initialize");
  allocData(getNode().getName(), "ronode", ronode_status_revision);
  m_con.init("basf2", 1);
  if (!m_dummymode) {
    m_ttrx.open();
    m_copper.open();
  }
  m_flow.open(&m_con.getInfo());
  initialized = false;
  configure(obj);
  initialized = true;
  const std::string path_shm = "/cpr_pause_resume";
  if (!m_memory.open(path_shm, sizeof(int))) {
    perror("shm_open");
    LogFile::error("Failed to open %s", path_shm.c_str());
  }
  char* buf = (char*)m_memory.map(0, sizeof(int));
  memset(buf, 0, sizeof(int));
  LogFile::debug("COPPERCallback::initialize done");
}

void COPPERCallback::configure(const DBObject& obj) throw(RCHandlerException)
{
  try {
    add(new NSMVHandlerOutputPort(*this, "basf2.output.port"));
    add(new NSMVHandlerFifoEmpty(*this, "copper.err.fifoempty"));
    add(new NSMVHandlerFifoFull(*this, "copper.err.fifofull"));
    add(new NSMVHandlerLengthFifoFull(*this, "copper.err.lengthfifofull"));
    add(new NSMVHandlerInt("ttrx.err.b2link", true, false, 0));
    add(new NSMVHandlerInt("ttrx.err.linkup", true, false, 0));
    add(new NSMVHandlerText("ttrx.msg", true, false, ""));
    const DBObject& o_ttrx(obj("ttrx"));
    add(new NSMVHandlerTTRXFirmware(*this, "ttrx.firm", 0,
                                    o_ttrx.hasText("firm") ? o_ttrx.getText("firm") : ""));
    add(new NSMVHandlerFEELoadAll(*this, "loadfee"));
    for (int i = 0; i < 4; i++) {
      const DBObject& o_hslb(obj("hslb", i));
      std::string vname = StringUtil::form("hslb[%d]", i);
      add(new NSMVHandlerHSLBUsed(*this, vname + ".used", i, !(m_dummymode || !m_fee[i] || !o_hslb.getBool("used"))));
      if (m_dummymode || !m_fee[i] || !o_hslb.getBool("used")) continue;
      HSLB& hslb(m_hslb[i]);
      hslb.open(i);
      m_o_fee[i] = dbload(obj("fee", i).getText("path"));
      m_o_fee[i].print();
      m_fee[i]->init(*this, hslb, m_o_fee[i]);
      add(new NSMVHandlerHSLBFirmware(*this, "hslb.firm", -1,
                                      o_hslb.hasText("firm") ? o_hslb.getText("firm") : ""));
      add(new NSMVHandlerText("hslb.msg", true, false, ""));
      add(new NSMVHandlerText(vname + ".msg", true, false, ""));
      add(new NSMVHandlerInt(vname + ".err.fifoempty", true, false, 0));
      add(new NSMVHandlerInt(vname + ".err.b2linkdown", true, false, 0));
      add(new NSMVHandlerInt(vname + ".err.cprfifofull", true, false, 0));
      add(new NSMVHandlerInt(vname + ".err.cprlengthfifofull", true, false, 0));
      add(new NSMVHandlerInt(vname + ".err.fifofull", true, false, 0));
      add(new NSMVHandlerInt(vname + ".err.crc", true, false, 0));
      add(new NSMVHandlerHSLBFirmware(*this, vname + ".firm", i,
                                      o_hslb.hasText("firm") ? o_hslb.getText("firm") : ""));
      add(new NSMVHandlerHSLBBoot(*this, vname + ".boot", i, ""));
      vname = StringUtil::form("hslb[%d]", i);
      add(new NSMVHandlerHSLBRegFixed(*this, vname + ".fmver", i, 0x81, 4));
      add(new NSMVHandlerHSLBRegFixed(*this, vname + ".b2lstat", i, 0x83, 4));
      add(new NSMVHandlerHSLBRegFixed(*this, vname + ".rxdata", i, 0x84, 4));
      add(new NSMVHandlerHSLBRegFixed(*this, vname + ".evtsize", i, 0x85, 4));
      add(new NSMVHandlerHSLBRegFixed(*this, vname + ".nevent", i, 0x86, 4));
      add(new NSMVHandlerHSLBRegFixed(*this, vname + ".uptime", i, 0x87, 4));
      add(new NSMVHandlerHSLBRegFixed(*this, vname + ".nbyte", i, 0x88, 4));
      add(new NSMVHandlerHSLBRegFixed(*this, vname + ".nword", i, 0x89, 4));

      add(new NSMVHandlerHSLBRegFixed(*this, vname + ".hslbhw", i, HSLBHW, 4));
      add(new NSMVHandlerHSLBRegFixed(*this, vname + ".hslbfw", i, HSLBFW, 4));
      add(new NSMVHandlerHSLBRegFixed(*this, vname + ".hslbid", i, HSLBID, 4));
      add(new NSMVHandlerHSLBRegFixed(*this, vname + ".hslbver", i, HSLBVER, 4));
      add(new NSMVHandlerInt(vname + ".serial", true, true, 0));
      add(new NSMVHandlerInt(vname + ".type", true, true, 0));

      int feehw, feeserial, feetype, feever;
      try {
        getfee(hslb, feehw, feeserial, feetype, feever);
      } catch (const std::exception& e) {
        LogFile::error("COPPERCallback::comfigure getfee failed");
      }
      add(new NSMVHandlerText(vname + ".feename", true, false, ::feename(feehw, feetype)));
      add(new NSMVHandlerInt(vname + ".feehw", true, false, feehw));
      add(new NSMVHandlerInt(vname + ".feeserial", true, false, feeserial));
      add(new NSMVHandlerInt(vname + ".feetype", true, false, feetype));
      add(new NSMVHandlerInt(vname + ".feever", true, false, feever));

      add(new NSMVHandlerInt(vname + ".crcerr", true, false, 0));
      add(new NSMVHandlerInt(vname + ".b2lcsr", true, false, 0));
      add(new NSMVHandlerInt(vname + ".hsd32", true, false, 0));
      add(new NSMVHandlerInt(vname + ".hsa32", true, false, 0));
      add(new NSMVHandlerInt(vname + ".hslstat", true, false, 0));
      add(new NSMVHandlerInt(vname + ".hslcont", true, false, 0));
      add(new NSMVHandlerInt(vname + ".cclk", true, false, 0));
      add(new NSMVHandlerInt(vname + ".conf", true, false, 0));
      add(new NSMVHandlerInt(vname + ".b2lstat", true, false, 0));
      add(new NSMVHandlerInt(vname + ".rxdata", true, false, 0));
      add(new NSMVHandlerInt(vname + ".fwevt", true, false, 0));
      add(new NSMVHandlerInt(vname + ".fwclk", true, false, 0));
      add(new NSMVHandlerInt(vname + ".cntsec", true, false, 0));
      add(new NSMVHandlerInt(vname + ".nword", true, false, 0));
      add(new NSMVHandlerInt(vname + ".errdet", true, false, 0));
      add(new NSMVHandlerInt(vname + ".errpat1", true, false, 0));
      add(new NSMVHandlerInt(vname + ".errpat2", true, false, 0));
      add(new NSMVHandlerInt(vname + ".b2lctim", true, false, 0));
      add(new NSMVHandlerInt(vname + ".b2ltag", true, false, 0));
      add(new NSMVHandlerInt(vname + ".b2lutim", true, false, 0));
      add(new NSMVHandlerInt(vname + ".b2lerun", true, false, 0));

      if (m_fee[i] != NULL) {
        const DBObject& o_fee(m_o_fee[i]);
        m_fee[i]->readback(*this, hslb, o_fee);
        add(new NSMVHandlerText(vname + ".name", true, false, m_fee[i]->getName()));
        vname = StringUtil::form("fee[%d]", i);
        if (o_fee.hasText("stream")) {
          add(new NSMVHandlerFEEStream(*this, vname + ".stream", i,
                                       o_fee.hasText("stream") ? o_fee.getText("stream") : ""));
        }
        add(new NSMVHandlerFEEBoot(*this, vname + ".boot", i, ""));
        add(new NSMVHandlerFEELoad(*this, vname + ".load", i, ""));
        vname = StringUtil::form("hslb[%d]", i);
        add(new NSMVHandlerHSLBTest(*this, vname + ".test", i));
      }
    }
  } catch (const std::exception& e) {
    initialized = true;
    throw (RCHandlerException(e.what()));
  }
}

void COPPERCallback::term() throw()
{
  m_con.abort();
  m_con.getInfo().unlink();
  for (int i = 0; i < 4; i++) {
    m_hslb[i].close();
  }
  m_copper.close();
  m_ttrx.close();
}

bool COPPERCallback::feeload()
{
  if (m_disablefeconf) return true;
  try {
    DBObject& obj(getDBObject());
    //    get(obj);
    for (int i = 0; i < 4; i++) {
      const DBObject& o_hslb(obj("hslb", i));
      if (m_fee[i] != NULL && o_hslb.getBool("used")) {
        HSLB& hslb(m_hslb[i]);
        hslb.open(i);
        if (!obj.hasObject("fee")) continue;
        std::string vname = StringUtil::form("hslb[%d]", i);
        set(vname + ".hslbhw", hslb.readfn(HSLBHW));
        set(vname + ".hslbfw", hslb.readfn(HSLBFW));
        int feehw, feeserial, feetype, feever;
        getfee(hslb, feehw, feeserial, feetype, feever);
        set(vname + ".feename", ::feename(feehw, feetype));
        set(vname + ".feehw", feehw);
        set(vname + ".feeserial", feeserial);
        set(vname + ".feetype", feetype);
        set(vname + ".feever", feever);
        set(vname + ".serial", hslb.readfn(SERIAL_L) | (hslb.readfn(SERIAL_H) >> 8));
        set(vname + ".type", hslb.readfn(TYPE_L) | (hslb.readfn(TYPE_H) >> 8));
        set(vname + ".hslbid", hslb.readfn(HSLBID));
        set(vname + ".hslbver", hslb.readfn(HSLBVER));
        try {
          hslb.test();
        } catch (const HSLBHandlerException& e) {
          throw (RCHandlerException("tesths failed : %s", e.what()));
        }
        try {
          hslb.test();
          set(vname + ".hslbhw", hslb.readfn(HSLBHW));
          set(vname + ".hslbfw", hslb.readfn(HSLBFW));
          int feehw, feeserial, feetype, feever;
          getfee(hslb, feehw, feeserial, feetype, feever);
          set(vname + ".feename", ::feename(feehw, feetype));
          set(vname + ".feehw", feehw);
          set(vname + ".feeserial", feeserial);
          set(vname + ".feetype", feetype);
          set(vname + ".feever", feever);
          set(vname + ".serial", hslb.readfn(SERIAL_L) | (hslb.readfn(SERIAL_H) >> 8));
          set(vname + ".type", hslb.readfn(TYPE_L) | (hslb.readfn(TYPE_H) >> 8));
          set(vname + ".hslbid", hslb.readfn(HSLBID));
          set(vname + ".hslbver", hslb.readfn(HSLBVER));
        } catch (const HSLBHandlerException& e) {
          set(vname + ".hslbhw", -1);
          set(vname + ".hslbfw", -1);
          set(vname + ".feename", "LinkDown");
          set(vname + ".feehw", -1);
          set(vname + ".feeserial", -1);
          set(vname + ".feetype", -1);
          set(vname + ".feever", -1);
          set(vname + ".serial", -1);
          set(vname + ".type", -1);
          set(vname + ".hslbid", -1);
          set(vname + ".hslbver", -1);
          throw (RCHandlerException("tesths failed : %s", e.what()));
        }
      }
    }
    return true;
  } catch (const std::out_of_range& e) {
    throw (RCHandlerException(e.what()));
  }
  return false;
}

void COPPERCallback::boot(const DBObject& obj) throw(RCHandlerException)
{
  abort();

  if (!m_dummymode) {
    const DBObject& o_ttrx(obj("ttrx"));
    std::string firmware = o_ttrx.getText("firm");
    if (File::exist(firmware)) {
      set("ttrx.msg", "programing");
      log(LogFile::INFO, "bootrx %s", firmware.c_str());
      system(("bootrx " + firmware).c_str());
      set("ttrx.msg", "program done");
      log(LogFile::INFO, "bootrx done");
    }
    int val = 0;
    for (int i = 0; i < 4; i++) {
      const DBObject& o_hslb(obj("hslb", i));
      std::string vname = StringUtil::form("hslb[%d]", i);
      add(new NSMVHandlerHSLBUsed(*this, vname + ".used", i, !(m_dummymode || !m_fee[i] || !o_hslb.getBool("used"))));
      if (m_dummymode || !m_fee[i] || !o_hslb.getBool("used")) continue;
      val += (1 << i);
    }
    m_ttrx.write(0x130, val);
    for (int i = 0; i < 4; i++) {
      const DBObject& o_hslb(obj("hslb", i));
      if (m_dummymode || !m_fee[i] || !o_hslb.getBool("used")) continue;
      HSLB& hslb(m_hslb[i]);
      firmware = o_hslb.getText("firm");
      if (File::exist(firmware)) {
        log(LogFile::INFO, "booths -%c %s", ('a' + i), firmware.c_str());
        set("hslb.msg", "programing");
        std::string cmd = StringUtil::form("booths -%c ", ('a' + i)) + firmware;
        system(cmd.c_str());
        set("hslb.msg", "program done");
        log(LogFile::INFO, "booths -%c done", ('a' + i));
        std::string emsg = "";
        bool success = false;
        for (int j = 0; j < 3; j++) {
          try {
            log(LogFile::INFO, "test hslb-%c", i + 'a');
            hslb.test();
            log(LogFile::INFO, "test hslb-%c done", i + 'a');
            set("hslb.msg", "tesths done");
            success = true;
            break;
          } catch (const HSLBHandlerException& e) {
            emsg = e.what();
          }
          if (!success) {
            set("hslb.msg", "tesths failed");
            log(LogFile::ERROR, "test hslb-%c failed %s", i + 'a', emsg.c_str());
          }
        }
      }
    }
    try {
      for (int i = 0; i < 4; i++) {
        if (!m_fee[i]) continue;
        const DBObject& o_hslb(obj("hslb", i));
        if (o_hslb.getBool("used") && obj.hasObject("fee")) {
          const DBObject& o_fee(m_o_fee[i]);
          HSLB& hslb(m_hslb[i]);
          hslb.open(i);
          if (!m_disablefeconf) {
            FEE& fee(*m_fee[i]);
            try {
              fee.boot(*this, hslb, o_fee);
            } catch (const IOException& e) {
              throw (RCHandlerException(e.what()));
            }
          }
        }
      }
    } catch (const std::out_of_range& e) {
      throw (RCHandlerException(e.what()));
    }
  }
}

void COPPERCallback::load(const DBObject& obj) throw(RCHandlerException)
{
  bool done_tesths = true;
  if (!m_dummymode) {
    m_ttrx.open();
    m_ttrx.monitor();
    if (m_ttrx.isError()) {
      m_ttrx.close();
      throw (RCHandlerException("TTRX Link error"));
    }
    try {
      int flag = 0;
      int nhslb = 0;
      const DBObjectList& o_hslbs(obj.getObjects("hslb"));
      for (size_t i = 0; i < o_hslbs.size(); i++) {
        o_hslbs[i].print();
        if (o_hslbs[i].getBool("used")) {
          flag += 1 << i;
          nhslb++;
        }
      }
      for (int i = 0; i < 10; i++) {
        m_ttrx.write(0x130, flag);
        int flag_ret = (m_ttrx.read(0x130) & 0xF);
        if (flag == flag_ret) {
          break;
        }
        log(LogFile::WARNING, "ttrx-130 is not consistent : (%x>>%x)", flag, flag_ret);
        usleep(50000);
      }
      for (size_t i = 0; i < o_hslbs.size(); i++) {
        if (!m_fee[i]) continue;
        const DBObject& o_hslb(obj("hslb", i));
        if (o_hslb.getBool("used")) {
          HSLB& hslb(m_hslb[i]);
          hslb.open(i);
          if (!m_fee[i]) continue;
          if (!obj.hasObject("fee")) continue;
          std::string vname = StringUtil::form("hslb[%d]", i);
          try {
            hslb.test();
            set(vname + ".hslbhw", hslb.readfn(HSLBHW));
            set(vname + ".hslbfw", hslb.readfn(HSLBFW));
            int feehw, feeserial, feetype, feever;
            getfee(hslb, feehw, feeserial, feetype, feever);
            set(vname + ".feename", ::feename(feehw, feetype));
            set(vname + ".feehw", feehw);
            set(vname + ".feeserial", feeserial);
            set(vname + ".feetype", feetype);
            set(vname + ".feever", feever);
            set(vname + ".serial", hslb.readfn(SERIAL_L) | (hslb.readfn(SERIAL_H) >> 8));
            set(vname + ".type", hslb.readfn(TYPE_L) | (hslb.readfn(TYPE_H) >> 8));
            set(vname + ".hslbid", hslb.readfn(HSLBID));
            set(vname + ".hslbver", hslb.readfn(HSLBVER));
          } catch (const HSLBHandlerException& e) {
            set(vname + ".hslbhw", -1);
            set(vname + ".hslbfw", -1);
            set(vname + ".feename", "LinkDown");
            set(vname + ".feehw", -1);
            set(vname + ".feeserial", -1);
            set(vname + ".feetype", -1);
            set(vname + ".feever", -1);
            set(vname + ".serial", -1);
            set(vname + ".type", -1);
            set(vname + ".hslbid", -1);
            set(vname + ".hslbver", -1);
            log(LogFile::ERROR, "tesths -%c failed : %s", ('a' + i), e.what());
            done_tesths = false;
            continue;
          }
          if (!m_disablefeconf) {
            FEE& fee(*m_fee[i]);
            try {
              const DBObject& o_fee(m_o_fee[i]);
              fee.load(*this, hslb, o_fee);
            } catch (const IOException& e) {
              throw (RCHandlerException(e.what()));
            }
          }
        }
      }
    } catch (const std::out_of_range& e) {
      throw (RCHandlerException(e.what()));
    }
  }
  if (!done_tesths) {
    throw (RCHandlerException("Failed to load"));
  }
  bootBasf2(obj);
}

void COPPERCallback::start(int expno, int runno) throw(RCHandlerException)
{
  for (int i = 0; i < 4; i++) {
    if (!m_fee[i]) continue;
    HSLB& hslb(m_hslb[i]);
    FEE& fee(*m_fee[i]);
    try {
      fee.start(*this, hslb);
    } catch (const IOException& e) {
      throw (RCHandlerException(e.what()));
    }
  }
  if (!m_dummymode && !m_con.start(expno, runno)) {
    throw (RCHandlerException("Failed to start"));
  }
}

void COPPERCallback::stop() throw(RCHandlerException)
{
  m_con.stop();
  for (int i = 0; i < 4; i++) {
    if (!m_fee[i]) continue;
    HSLB& hslb(m_hslb[i]);
    FEE& fee(*m_fee[i]);
    try {
      fee.stop(*this, hslb);
    } catch (const IOException& e) {
      throw (RCHandlerException(e.what()));
    }
  }
}

bool COPPERCallback::pause() throw(RCHandlerException)
{
  LogFile::debug("Pausing");
  m_con.pause();
  while (m_con.getInfo().isRunning() || m_con.getInfo().isPausing()) {
    try {
      perform(wait(NSMNode(), RCCommand::ABORT, 1));
      LogFile::notice("Pause was canceled");
      return false;
    } catch (const TimeoutException& e) {
      LogFile::warning("Pasuing not finished yet");
    }
  }
  if (m_con.getInfo().isPaused()) {
    LogFile::info("Paused");
  } else {
    LogFile::warning("Pasuing was ignored");
  }
  return true;
}

bool COPPERCallback::resume(int /*subno*/) throw(RCHandlerException)
{
  LogFile::debug("Resuming");
  m_con.resume();
  while (m_con.getInfo().isPaused() || m_con.getInfo().isResuming()) {
    try {
      perform(wait(NSMNode(), RCCommand::ABORT, 1));
      LogFile::notice("Resume was canceled");
      return false;
    } catch (const TimeoutException& e) {
      LogFile::warning("Resuming not finished yet");
    }
  }
  if (m_con.getInfo().isRunning()) {
    LogFile::info("Resumed");
  } else {
    LogFile::warning("Resume was ignored");
  }
  return true;
}

void COPPERCallback::recover(const DBObject& /*obj*/) throw(RCHandlerException)
{
  //abort();
}

void COPPERCallback::abort() throw(RCHandlerException)
{
  stop();
  m_con.abort();
}

void COPPERCallback::logging(bool err, LogFile::Priority pri,
                             const char* str, ...) throw()
{
  if (err) {
    va_list ap;
    static char ss[1024];
    va_start(ap, str);
    vsprintf(ss, str, ap);
    va_end(ap);
    log(pri, ss);
    if (pri >= LogFile::ERROR)
      setState(RCState::NOTREADY_S);
  }
}

void COPPERCallback::monitor() throw(RCHandlerException)
{
  RCState state = getNode().getState();
  try {
    try {
      m_ttrx.monitor();
    } catch (const IOException& e) {
      LogFile::warning(e.what());
      return;
    }
    try {
      m_copper.monitor();
    } catch (const IOException& e) {
      LogFile::warning(e.what());
      return;
    }
    logging(m_copper.isFifoFull(), LogFile::WARNING, "COPPER FIFO full");
    logging(m_copper.isLengthFifoFull(), LogFile::WARNING, "COPPER length FIFO full");
    set("copper.err.fifoempty", m_copper.isFifoEmpty());
    for (int i = 0; i < 4; i++) {
      if (!m_fee[i]) continue;
      int used = 0;
      get(StringUtil::form("hslb[%d].used", i), used);
      if (used) {
        HSLB& hslb(m_hslb[i]);
        hslb.monitor();
        if (state == RCState::RUNNING_S) {
          logging(hslb.isBelle2LinkDown(), LogFile::ERROR,
                  "HSLB %c Belle2 link down", (char)(i + 'a'));
          logging(hslb.isCOPPERFifoFull(), LogFile::WARNING,
                  "HSLB %c COPPER fifo full", (char)(i + 'a'));
          logging(hslb.isCOPPERLengthFifoFull(), LogFile::WARNING,
                  "HSLB %c COPPER length fifo full", (char)(i + 'a'));
          logging(hslb.isFifoFull(), LogFile::WARNING, "HSLB %c fifo full", (char)(i + 'a'));
          logging(hslb.isCRCError(), LogFile::WARNING, "HSLB %c CRC error", (char)(i + 'a'));
        }
        if (!m_disablefeconf) {
          FEE& fee(*m_fee[i]);
          fee.monitor(*this, hslb);
        }
        std::string vname = StringUtil::form("hslb[%d]", i);
        set(vname + ".err.b2linkdown", hslb.isBelle2LinkDown());
        set(vname + ".err.cprfifofull", hslb.isCOPPERFifoFull());
        set(vname + ".err.cprlengthfifofull", hslb.isCOPPERLengthFifoFull());
        set(vname + ".err.fifofull", hslb.isFifoFull());
        set(vname + ".err.crc", hslb.isCRCError());
        try {
          set(vname + ".crcerr", hslb.readfn(CRCERR));
          set(vname + ".b2lcsr", hslb.readfn(B2LCSR));
          set(vname + ".hsd32", hslb.readfn(HSD32));
          set(vname + ".hsa32", hslb.readfn(HSA32));
          set(vname + ".hslstat", hslb.readfn(HSLSTAT));
          set(vname + ".hslcont", hslb.readfn(HSLCONT));
          set(vname + ".cclk", hslb.readfn(CCLK));
          set(vname + ".conf", hslb.readfn(CONF));
          set(vname + ".b2lstat", hslb.readfn(B2LSTAT));
          set(vname + ".rxdata", hslb.readfn(RXDATA));
          set(vname + ".fwevt", hslb.readfn(FWEVT));
          set(vname + ".fwclk", hslb.readfn(FWCLK));
          set(vname + ".cntsec", hslb.readfn(CNTSEC));
          set(vname + ".nword", hslb.readfn(NWORD));
          set(vname + ".errdet", hslb.readfn(ERRDET));
          set(vname + ".errpat1", hslb.readfn(ERRPAT1));
          set(vname + ".errpat2", hslb.readfn(ERRPAT2));
          set(vname + ".b2lctim", hslb.readfn(B2LCTIM));
          set(vname + ".b2ltag", hslb.readfn(B2LTAG));
          set(vname + ".b2lutim", hslb.readfn(B2LUTIM));
          set(vname + ".b2lerun", hslb.readfn(B2LERUN));
        } catch (const HSLBHandlerException& e) {
          set(vname + ".crcerr", -1);
          set(vname + ".b2lcsr", -1);
          set(vname + ".hsd32", -1);
          set(vname + ".hsa32", -1);
          set(vname + ".hslstat", -1);
          set(vname + ".hslcont", -1);
          set(vname + ".cclk", -1);
          set(vname + ".conf", -1);
          set(vname + ".b2lstat", -1);
          set(vname + ".rxdata", -1);
          set(vname + ".fwevt", -1);
          set(vname + ".fwclk", -1);
          set(vname + ".cntsec", -1);
          set(vname + ".nword", -1);
          set(vname + ".errdet", -1);
          set(vname + ".errpat1", -1);
          set(vname + ".errpat2", -1);
          set(vname + ".b2lctim", -1);
          set(vname + ".b2ltag", -1);
          set(vname + ".b2lutim", -1);
          set(vname + ".b2lerun", -1);
        }
      }
    }
    if (state == RCState::RUNNING_S && state.isStable()) {
      logging(m_ttrx.isBelle2LinkError(), LogFile::WARNING, "TTRX Belle2 link error");
      logging(m_ttrx.isLinkUpError(), LogFile::WARNING, "TTRX Link up error");
    }
    set("ttrx.err.b2link", m_ttrx.isBelle2LinkError());
    set("ttrx.err.linkup", m_ttrx.isLinkUpError());
    NSMData& data(getData());
    if (data.isAvailable()) {
      ronode_status* nsm = (ronode_status*)data.get();
      if (m_flow.isAvailable()) {
        ronode_status& status(m_flow.monitor());
        memcpy(nsm, &status, sizeof(ronode_status));
      } else {
        memset(nsm, 0, sizeof(ronode_status));
      }
      double loads[3];
      if (getloadavg(loads, 3) > 0) {
        nsm->loadavg = (float)loads[0];
      } else {
        nsm->loadavg = -1;
      }
      data.flush();
    }
  } catch (const std::exception& e) {
    LogFile::warning(e.what());
  }
  if (getNode().getState() == RCState::RUNNING_S) {
    if (!m_con.isAlive()) {
      setState(RCState::ERROR_ES);
      log(LogFile::ERROR, "basf2 was down");
    }
  }
}

void COPPERCallback::bootBasf2(const DBObject& obj) throw(RCHandlerException)
{
  if (m_con.isAlive()) return;
  std::string script;
  try {
    int flag = 0;
    int nhslb = 4;
    for (size_t i = 0; i < 4; i++) {
      if (!m_fee[i]) continue;
      const DBObject& o_hslb(obj.getObject("hslb", i));
      if (m_fee[i] != NULL && o_hslb.getBool("used")) {
        flag += 1 << i;
      }
    }
    m_con.clearArguments();
    if (!m_dummymode) {
      //m_con.setExecutable(StringUtil::form("%s/daq/ropc/des_ser_COPPER_main", getenv("BELLE2_LOCAL_DIR")));
      m_con.setExecutable("/home/usr/b2daq/cprdaq/bin/des_ser_COPPER_main");
      m_con.addArgument(obj.getText("hostname"));
      std::string copperid_s = obj.getText("copperid");
      int id = atoi(copperid_s.substr(3).c_str());
      int copperid = id % 1000;
      int detectorid = id / 1000;
      char str[64];
      sprintf(str, "0x%d000000", detectorid);
      copperid += strtol(str, NULL, 0);
      m_con.addArgument(StringUtil::form("%d", copperid));
      m_con.addArgument("%d", flag);
      m_con.addArgument("1");
      const std::string nodename = StringUtil::tolower(getNode().getName());
      m_con.addArgument(nodename + "_" + "basf2");
      try {
        m_con.load(30);
      } catch (const std::exception& e) {
        LogFile::warning("load timeout");
      }
    } else {
      m_con.setExecutable(StringUtil::form("%s/daq/rawdata/src/dummy_data_src", getenv("BELLE2_LOCAL_DIR")));
      std::string copperid_s = obj.getText("copperid");
      int id = atoi(copperid_s.substr(3).c_str());
      int copperid = id % 1000;
      int detectorid = id / 1000;
      char str[64];
      sprintf(str, "0x%d000000", detectorid);
      copperid += strtol(str, NULL, 0);
      m_con.addArgument(StringUtil::form("%d", copperid));
      m_con.addArgument("0");
      m_con.addArgument("%d", (obj.hasValue("nwords") ? obj.getInt("nwords") : 10));
      m_con.addArgument("1");
      m_con.addArgument("%d", nhslb);
      m_con.load(0);
    }
  } catch (const std::out_of_range& e) {
    throw (RCHandlerException(e.what()));
  }
  LogFile::debug("load succeded");
}

bool COPPERCallback::isError() throw()
{
  if (m_copper.isError()) return true;
  for (int i = 0; i < 4; i++) {
    int used = 0;
    get(StringUtil::form("hslb[%d].used", i), used);
    if (m_fee[i] != NULL && used && m_hslb[i].isError()) return true;
  }
  if (m_ttrx.isBelle2LinkError())
    return true;
  return false;
}

/*
      add(new NSMVHandlerInt(vname + ".crcerr", true, false, hslb.readfn(CRCERR)));
      add(new NSMVHandlerInt(vname + ".b2lcsr", true, false, hslb.readfn(B2LCSR)));
      add(new NSMVHandlerInt(vname + ".hsd32", true, false, hslb.readfn(HSD32)));
      add(new NSMVHandlerInt(vname + ".hsa32", true, false, hslb.readfn(HSA32)));
      add(new NSMVHandlerInt(vname + ".hslstat", true, false, hslb.readfn(HSLSTAT)));
      add(new NSMVHandlerInt(vname + ".hslcont", true, false, hslb.readfn(HSLCONT)));
      add(new NSMVHandlerInt(vname + ".cclk", true, false, hslb.readfn(CCLK)));
      add(new NSMVHandlerInt(vname + ".conf", true, false, hslb.readfn(CONF)));
      add(new NSMVHandlerInt(vname + ".b2lstat", true, false, hslb.readfn(B2LSTAT)));
      add(new NSMVHandlerInt(vname + ".rxdata", true, false, hslb.readfn(RXDATA)));
      add(new NSMVHandlerInt(vname + ".fwevt", true, false, hslb.readfn(FWEVT)));
      add(new NSMVHandlerInt(vname + ".fwclk", true, false, hslb.readfn(FWCLK)));
      add(new NSMVHandlerInt(vname + ".cntsec", true, false, hslb.readfn(CNTSEC)));
      add(new NSMVHandlerInt(vname + ".nword", true, false, hslb.readfn(NWORD)));
      add(new NSMVHandlerInt(vname + ".errdet", true, false, hslb.readfn(ERRDET)));
      add(new NSMVHandlerInt(vname + ".errpat1", true, false, hslb.readfn(ERRPAT1)));
      add(new NSMVHandlerInt(vname + ".errpat2", true, false, hslb.readfn(ERRPAT2)));
      add(new NSMVHandlerInt(vname + ".b2lctim", true, false, hslb.readfn(B2LCTIM)));
      add(new NSMVHandlerInt(vname + ".b2ltag", true, false, hslb.readfn(B2LTAG)));
      add(new NSMVHandlerInt(vname + ".b2lutim", true, false, hslb.readfn(B2LUTIM)));
      add(new NSMVHandlerInt(vname + ".b2lerun", true, false, hslb.readfn(B2LERUN)));

 */
