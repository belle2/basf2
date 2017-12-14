#include "daq/slc/copper/arich/HAPDFEE.h"
#include "daq/slc/copper/arich/N6SA0xHSLB.h"

#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>

#include <mgt/hsreg.h>

#include <sstream>

using namespace Belle2;

HAPDFEE::HAPDFEE()
{
}

void HAPDFEE::init(RCCallback& callback, HSLB& hslb)
{

}

void HAPDFEE::boot(HSLB& hslb,  const DBObject& obj)
{

}

void HAPDFEE::load(HSLB& hslb, const DBObject& obj)
{
  N6SA0xHSLB sa03(hslb);
  // initialization
  const DBField::NameList& names(obj.getFieldNames());
  for (DBField::NameList::const_iterator it = names.begin();
       it != names.end(); it++) {
    const std::string cmd = *it;
    if (obj.hasValue(cmd)) {
      int val = obj.getInt(cmd);
      printf("%s : %x\n", cmd.c_str(), val);
      execute(sa03, cmd, val);
    }
  }

  // global parameter
  const DBObjectList& o_chips(obj.getObjects("chip"));
  for (size_t ichip = 0; ichip < o_chips.size(); ichip++) {
    const DBObject& o_chip(o_chips[ichip]);
    const DBField::NameList& cnames(o_chip.getFieldNames());
    for (DBField::NameList::const_iterator it = cnames.begin();
         it != cnames.end(); it++) {
      const std::string cmd = *it;
      if (o_chip.hasValue(cmd)) {
        int val = o_chip.getInt(cmd);
        printf("%s : %x\n", cmd.c_str(), val);
        execute(sa03, cmd, val, ichip);
      }
    }
  }

  // channel parameter
  for (size_t ichip = 0; ichip < o_chips.size(); ichip++) {
    const DBObject& o_chip(o_chips[ichip]);
    const DBObjectList& o_chs(o_chip.getObjects("ch"));
    for (size_t ich = 0; ich < o_chs.size(); ich++) {
      const DBObject& o_ch(o_chs[ich]);
      const DBField::NameList& cnames(o_ch.getFieldNames());
      for (DBField::NameList::const_iterator it = cnames.begin();
           it != cnames.end(); it++) {
        const std::string cmd = *it;
        if (o_ch.hasValue(cmd)) {
          int val = o_ch.getInt(cmd);
          printf("%s : %x\n", cmd.c_str(), val);
          execute(sa03, cmd, val, ichip, ich);
        }
      }
    }
  }

  // select chip-channel
  const DBObjectList& o_selects(obj.getObjects("select"));
  for (size_t i = 0; i < o_selects.size(); i++) {
    const DBObject& o_select(o_selects[i]);
    if (o_select.hasValue("chip") && o_select.getValue("ch")) {
      sa03.select(o_select.getInt("chip"), o_select.getInt("ch"));
    }
  }
}

int HAPDFEE::execute(N6SA0xHSLB& sa03, const std::string& cmd,
                     unsigned int val, int chip, int ch)
{
  if (val < 0) return 0;
  if (cmd == "init") {
    sa03.init();
  } else if (cmd == "clear") {
    sa03.clear();
  } else if (cmd == "runclear") {
    sa03.runclear();
  } else if (cmd == "csr1") {
    sa03.csr1(val);
  } else if (cmd == "hdcycle") {
    sa03.hdcycle(val);
  } else if (cmd == "trgdelay") {
    sa03.trgdelay(val);
  } else if (cmd == "csr2") {
    sa03.csr2(val);
  } else if (cmd == "trigen") {
    sa03.trigen(val);
  } else if (cmd == "param_global") {
    m_globalparam[chip].param(val);
  } else if (cmd == "phasecmps") {
    m_globalparam[chip].set_phasecmps(val);
  } else if (cmd == "gain") {
    m_globalparam[chip].set_gain(val);
  } else if (cmd == "shapingtime") {
    m_globalparam[chip].set_shapingtime(val);
  } else if (cmd == "comparator") {
    m_globalparam[chip].set_comparator(val);
  } else if (cmd == "vrdrive") {
    m_globalparam[chip].set_vrdrive(val);
  } else if (cmd == "monitor") {
    m_globalparam[chip].set_monitor(val);
  } else if (cmd == "load_global") {
    load_global(sa03, chip);
  } else if (cmd == "param_ch") {
    m_channelparam[chip][ch].param(val);
  } else if (cmd == "decaytime") {
    m_channelparam[chip][ch].set_decaytime(val);
  } else if (cmd == "offset") {
    m_channelparam[chip][ch].set_offset(val);
  } else if (cmd == "fineadj_unipol") {
    m_channelparam[chip][ch].set_fineadj_unipol(val);
  } else if (cmd == "fineadj_diff") {
    m_channelparam[chip][ch].set_fineadj_diff(val);
  } else if (cmd == "tpenb") {
    m_channelparam[chip][ch].set_tpenb(val);
  } else if (cmd == "kill") {
    m_channelparam[chip][ch].set_kill(val);
  } else if (cmd == "load_ch") {
    load_ch(sa03, chip, ch);
  } else if (cmd == "select") {
    sa03.select(chip, ch);
  } else {
    fprintf(stderr, "parse error: no command %s\n", cmd.c_str());
    return -1;
  }
  return 1;
}

void HAPDFEE::load_global(N6SA0xHSLB& sa03, unsigned int chip)
{
  SA0xGlobalParam& prm(m_globalparam[chip]);
  sa03.select(chip, -1);
  //   printf( "0x%x\n", sa03.sel() );
  sa03.wparam(prm.param());
  sa03.prmset(); // load parameter
  sa03.ndro();   // ndro
  prm.rbparam(sa03.rparam());
  //   printf( "%x\n", prm.rbparam() );
  /* compare */
  if (prm.compare(false) == 0) {
    printf("global param 0x%07x correctly loaded for chip %d (id=0x%x).\n",
           prm.param(), chip, prm.getrb_id());
  } else {
    printf("Error in global param for chip %d (id=0x%x):"
           "(set 0x%07x readback 0x%07x).\n", chip,
           prm.getrb_id(), prm.param(), prm.getrb_masked());
  }
}

void HAPDFEE::load_ch(N6SA0xHSLB& sa03, unsigned int chip, unsigned int ch)
{
  SA0xChannelParam& prm(m_channelparam[chip][ch]);
  sa03.select(chip, ch);
  sa03.wparam(prm.param());
  sa03.prmset(); // load parameter
  sa03.ndro(); // load parameter once more
  prm.rbparam(sa03.rparam());
  /* compare */
  if (prm.compare(false) == 0) {
    printf("ch. param 0x%07x correctly loaded for chip %d ch %2d.\n",
           prm.param(), chip, ch);
  } else {
    printf("Error in ch. param for chip %d ch %2d:"
           "(set 0x%07x readback 0x%07x).\n", chip, ch,
           prm.param(), prm.rbparam());
  }
}

