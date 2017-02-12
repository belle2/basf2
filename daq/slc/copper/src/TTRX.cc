#include "daq/slc/copper/TTRX.h"

#include <daq/slc/system/File.h>

#include <cstring>

#define D(a,b,c) (((a)>>(c))&((1<<((b)+1-(c)))-1))
#define B(a,b)   D(a,b,b)
#define W(r,n,a,b) ((((r)>>(n))&1)?(a):(b))

using namespace Belle2;

bool TTRX::open() throw()
{
  if (m_ttrx == NULL) {
    m_ttrx = open_ttrx(0, TTRX_RDWR);
  }
  return m_ttrx != NULL;
}

bool TTRX::close() throw()
{
  if (m_ttrx != NULL) {
    close_ttrx(m_ttrx);
    m_ttrx = NULL;
  }
  return true;
}

bool TTRX::boot(const std::string& file) throw()
{
  if (m_ttrx != NULL && file.size() > 0 && File::exist(file)) {
    memset(&m_info, 0, sizeof(ttrx_info));
    if (boot_ttrx_fpga(m_ttrx, TTRX_PRGM, file.c_str(), 0, 0) < 0) {
      return false;
    }
    readregs_4r19(m_ttrx, &m_info);
    write_ttrx(m_ttrx, 0x130, 0x3);
    return true;
  }
  return false;
}

ttrx_info& TTRX::monitor() throw()
{
  m_info.csr     = read_ttrx(m_ttrx, TT4R_CSR);
  m_info.csr2    = read_ttrx(m_ttrx, TT4R_CSR2);
  m_info.fin     = read_ttrx(m_ttrx, TT4R_FIN);
  m_info.fsta    = read_ttrx(m_ttrx, TT4R_FSTA);
  m_info.uraw    = read_ttrx(m_ttrx, TT4R_URAW);
  m_info.tlast   = read_ttrx(m_ttrx, TT4R_TLAST);
  m_info.rate    = read_ttrx(m_ttrx, TT4R_RATE);
  return m_info;
}

bool TTRX::isBelle2LinkError() throw()
{
  if (m_ttrx == NULL) return false;
  return B(m_info.uraw, 31) == 0;
}

bool TTRX::isLinkUpError() throw()
{
  if (m_ttrx == NULL) return false;
  return B(m_info.csr2, 31) != 0;
}

unsigned int TTRX::read(unsigned int addr) throw()
{
  return read_ttrx(m_ttrx, addr >> 2);
}

int TTRX::write(unsigned int addr, unsigned int val) throw()
{
  return write_ttrx(m_ttrx, addr >> 2, val);
}
