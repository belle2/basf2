//+
// File : RFNSM.cc
// Description : NSM interface for RFARM
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 25 - June - 2013
//-

#include "daq/rfarm/manager/RFNSM.h"
#define RFNSMOUT stdout

using namespace std;
using namespace Belle2;

extern "C" void nsmlib_debuglevel(int);

// Global variable to contain server instance
RFServerBase* g_nsmserver = 0;

// Constructor / Destructor
RFNSM::RFNSM(char* nodename, RFServerBase* server)
{

  // Set debug level
  //  nsmlib_debuglevel ( 1 );

  // Initialize NSM
  if (!b2nsm_init(nodename)) {
    fprintf(RFNSMOUT, "RFNSM : %s initialization failure, %s\n",
            nodename, b2nsm_strerror());
  }
  m_nodename = nodename;

  // Redirect log message to standard output
  b2nsm_logging(RFNSMOUT);

  // Set debug
  nsmlib_debuglevel(0);

  // Register server
  g_nsmserver = server;

  // Hook server functions to NSM
  if (b2nsm_callback("RF_CONFIGURE", m_Configure) < 0) {
    fprintf(stderr, "RFNSM : %s hooking CONFIGURE failed, %s\n",
            m_nodename.c_str(), b2nsm_strerror());
  }
  if (b2nsm_callback("RF_START", m_Start) < 0) {
    fprintf(stderr, "RFNSM : %s hooking START failed, %s\n",
            m_nodename.c_str(), b2nsm_strerror());
  }
  if (b2nsm_callback("RF_STOP", m_Stop) < 0) {
    fprintf(stderr, "RFNSM : %s hooking STOP failed, %s\n",
            m_nodename.c_str(), b2nsm_strerror());
  }
  if (b2nsm_callback("RF_PAUSE", m_Pause) < 0) {
    fprintf(stderr, "RFNSM : %s hooking PAUSE failed, %s\n",
            m_nodename.c_str(), b2nsm_strerror());
  }
  if (b2nsm_callback("RF_RESUME", m_Resume) < 0) {
    fprintf(stderr, "RFNSM : %s hooking RESUME failed, %s\n",
            m_nodename.c_str(), b2nsm_strerror());
  }
  if (b2nsm_callback("RF_RESTART", m_Restart) < 0) {
    fprintf(stderr, "RFNSM : %s hooking RESTART failed, %s\n",
            m_nodename.c_str(), b2nsm_strerror());
  }
  if (b2nsm_callback("RF_STATUS", m_Status) < 0) {
    fprintf(stderr, "RFNSM : %s hooking STATUS failed, %s\n",
            m_nodename.c_str(), b2nsm_strerror());
  }
}

RFNSM::~RFNSM()
{
}


void RFNSM::AllocMem(char* format)
{
  printf("AllocMem : format file = %s\n", format);
  // Allocate shared memory
  RfNodeInfo* m_info = (RfNodeInfo*)b2nsm_allocmem(m_nodename.c_str(), format,
                                                   1, 3);
  if (!m_info) {
    fprintf(RFNSMOUT, "RFNSM : %s allocmem failure, %s\n",
            m_nodename.c_str(), b2nsm_strerror());
  }

  m_formatfile = string(format);

}

// Wrapper functions to be hooked to NSM
void RFNSM::m_Configure(NSMmsg* msg, NSMcontext* ctx)
{
  g_nsmserver->Configure(msg, ctx);
}

void RFNSM::m_Start(NSMmsg* msg, NSMcontext* ctx)
{
  g_nsmserver->Start(msg, ctx);
}

void RFNSM::m_Stop(NSMmsg* msg, NSMcontext* ctx)
{
  g_nsmserver->Stop(msg, ctx);
}

void RFNSM::m_Pause(NSMmsg* msg, NSMcontext* ctx)
{
  g_nsmserver->Pause(msg, ctx);
}

void RFNSM::m_Resume(NSMmsg* msg, NSMcontext* ctx)
{
  g_nsmserver->Resume(msg, ctx);
}

void RFNSM::m_Restart(NSMmsg* msg, NSMcontext* ctx)
{
  g_nsmserver->Restart(msg, ctx);
}

void RFNSM::m_Status(NSMmsg* msg, NSMcontext* ctx)
{
  g_nsmserver->Status(msg, ctx);
}

// Interface to Node Info

RfNodeInfo* RFNSM::GetNodeInfo()
{
  return m_info;
}


