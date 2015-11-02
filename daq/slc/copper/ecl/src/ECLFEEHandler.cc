#include "daq/slc/copper/ecl/ECLFEEHandler.h"

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/copper/FEE.h>
#include <daq/slc/copper/HSLB.h>

#include "mgt/hsreg.h"

#include <unistd.h>

using namespace Belle2;

bool ECLFEEHandler::feeget(int& val)
{
  try {
    val = m_hslb.readfee8(m_adr);
    /*
    m_hslb.writefn(HSREG_CSR, 0x05); //reset address fifo
    m_hslb.writefn(HSREG_CSR, 0x06); //resert status register
    m_hslb.writefn(m_adr, 0x02);
    m_hslb.writefn(HSREG_CSR, 0x07);
    usleep(10000);
    val = m_hslb.readfn(HSREG_STAT);
    */
    LogFile::info("ECL FEE read 0x%x >> %d", m_adr, val);
  } catch (const IOException& e) {
    LogFile::error(e.what());
  }
  return true;
}

bool ECLFEEHandler::feeset(int val)
{
  try {
    m_hslb.writefee8(m_adr, val);
    /*
    m_hslb.writefn(HSREG_CSR, 0x05); //reset address fifo
    m_hslb.writefn(HSREG_CSR, 0x06); //resert status register
    m_hslb.writefn(m_adr, val);
    m_hslb.writefn(HSREG_CSR, 0x0A);
    */
    LogFile::info("ECL FEE write 0x%x << %d", m_adr, val);
  } catch (const IOException& e) {
    LogFile::error(e.what());
  }
  return true;

}
