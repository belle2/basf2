/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "daq/rfarm/manager/RFNSM.h"

extern "C" {
#include <nsm2/nsm2.h>
#include <nsm2/nsmlib2.h>
#include <nsm2/belle2nsm.h>
}

#include <iostream>

#define RFNSMOUT stdout

using namespace std;
using namespace Belle2;

//extern "C" void nsmlib_debuglevel(int);

// Global variable to contain server instance
RFServerBase* g_nsmserver = 0;

RFNSM_Status* RFNSM_Status::s_instance = 0;

NSMcontext* RFNSM::g_context = 0;

// Constructor / Destructor
RFNSM::RFNSM(char* nodename, RFServerBase* server)
{

  // Set debug level
  //  nsmlib_debuglevel ( 1 );

  // Initialize NSM
  if (!(g_context = b2nsm_init2(nodename, 0, 0, 0, 0))) {
    //if (!(g_context = b2nsm_init(nodename))) {
    fprintf(RFNSMOUT, "RFNSM : %s initialization failure, %s\n",
            nodename, b2nsm_strerror());
    g_context = 0;
  }
  nsmlib_usesig(g_context, 0);
  m_nodename = nodename;

  // Redirect log message to standard output
  b2nsm_logging(RFNSMOUT);

  // Set debug
  nsmlib_debuglevel(0);

  // Register server
  g_nsmserver = server;

  // Hook server functions to NSM
  //  if (b2nsm_callback("RF_CONFIGURE", m_Configure) < 0) {
  if (b2nsm_callback("RC_LOAD", m_Configure) < 0) {
    fprintf(stderr, "RFNSM : %s hooking CONFIGURE failed, %s\n",
            m_nodename.c_str(), b2nsm_strerror());
  }
  //  if (b2nsm_callback("RF_UNCONFIGURE", m_UnConfigure) < 0) {
  if (b2nsm_callback("RC_ABORT", m_UnConfigure) < 0) {
    fprintf(stderr, "RFNSM : %s hooking UNCONFIGURE failed, %s\n",
            m_nodename.c_str(), b2nsm_strerror());
  }
  //  if (b2nsm_callback("RF_START", m_Start) < 0) {
  if (b2nsm_callback("RC_START", m_Start) < 0) {
    fprintf(stderr, "RFNSM : %s hooking START failed, %s\n",
            m_nodename.c_str(), b2nsm_strerror());
  }
  //  if (b2nsm_callback("RF_STOP", m_Stop) < 0) {
  if (b2nsm_callback("RC_STOP", m_Stop) < 0) {
    fprintf(stderr, "RFNSM : %s hooking STOP failed, %s\n",
            m_nodename.c_str(), b2nsm_strerror());
  }
  //  if (b2nsm_callback("RF_PAUSE", m_Pause) < 0) {
  if (b2nsm_callback("RC_PAUSE", m_Pause) < 0) {
    fprintf(stderr, "RFNSM : %s hooking PAUSE failed, %s\n",
            m_nodename.c_str(), b2nsm_strerror());
  }
  //  if (b2nsm_callback("RF_RESUME", m_Resume) < 0) {
  if (b2nsm_callback("RC_RESUME", m_Resume) < 0) {
    fprintf(stderr, "RFNSM : %s hooking RESUME failed, %s\n",
            m_nodename.c_str(), b2nsm_strerror());
  }
  //  if (b2nsm_callback("RF_RESTART", m_Restart) < 0) {
  if (b2nsm_callback("RC_RECOVER", m_Restart) < 0) {
    fprintf(stderr, "RFNSM : %s hooking RESTART failed, %s\n",
            m_nodename.c_str(), b2nsm_strerror());
  }

  // Status function
  if (b2nsm_callback("RC_STATUS", m_Status) < 0) {
    fprintf(stderr, "RFNSM : %s hooking STATUS failed, %s\n",
            m_nodename.c_str(), b2nsm_strerror());
  }

  // Hook communication functions
  if (b2nsm_callback("OK", m_OK) < 0) {
    fprintf(stderr, "RFNSM : %s hooking OK failed, %s\n",
            m_nodename.c_str(), b2nsm_strerror());
  }
  if (b2nsm_callback("ERROR", m_ERROR) < 0) {
    fprintf(stderr, "RFNSM : %s hooking ERROR failed, %s\n",
            m_nodename.c_str(), b2nsm_strerror());
  }

  // Node status = Unconfigured
  RFNSM_Status::Instance().set_state(RFSTATE_UNCONFIGURED);
}

RFNSM::~RFNSM()
{
}


void RFNSM::AllocMem(char* format)
{
  printf("AllocMem : format file = %s\n", format);
  //  Allocate shared memory
  m_info = (RfNodeInfo*)b2nsm_allocmem(m_nodename.c_str(), format,
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
  fflush(stdout);
  int nsmstate = RFNSM_Status::Instance().get_state();
  RFNSM_Status::Instance().set_state(RFSTATE_TRANSITION);
  int stat = g_nsmserver->Configure(msg, ctx);
  fflush(stdout);
  if (stat == 0) {
    RFNSM_Status::Instance().set_state(RFSTATE_CONFIGURED);
    b2nsm_ok(msg, "Configured", NULL);
  } else {
    RFNSM_Status::Instance().set_state(nsmstate);
    b2nsm_error(msg, NULL);
  }
}

void RFNSM::m_UnConfigure(NSMmsg* msg, NSMcontext* ctx)
{
  fflush(stdout);
  int nsmstate = RFNSM_Status::Instance().get_state();
  RFNSM_Status::Instance().set_state(RFSTATE_TRANSITION);
  int stat = g_nsmserver->UnConfigure(msg, ctx);
  fflush(stdout);
  if (stat == 0) {
    RFNSM_Status::Instance().set_state(RFSTATE_UNCONFIGURED);
    b2nsm_ok(msg, "Unconfigured", NULL);
  } else {
    RFNSM_Status::Instance().set_state(nsmstate);
    b2nsm_error(msg, NULL);
  }
}

void RFNSM::m_Start(NSMmsg* msg, NSMcontext* ctx)
{
  int nsmstate = RFNSM_Status::Instance().get_state();
  RFNSM_Status::Instance().set_state(RFSTATE_TRANSITION);
  int stat = g_nsmserver->Start(msg, ctx);
  if (stat == 0) {
    RFNSM_Status::Instance().set_state(RFSTATE_RUNNING);
    b2nsm_ok(msg, "Running", NULL);
  } else {
    RFNSM_Status::Instance().set_state(nsmstate);
    b2nsm_error(msg, NULL);
  }
}

void RFNSM::m_Stop(NSMmsg* msg, NSMcontext* ctx)
{
  int nsmstate = RFNSM_Status::Instance().get_state();
  RFNSM_Status::Instance().set_state(RFSTATE_TRANSITION);
  int stat = g_nsmserver->Stop(msg, ctx);
  if (stat == 0) {
    RFNSM_Status::Instance().set_state(RFSTATE_CONFIGURED);
    b2nsm_ok(msg, "Stopped", NULL);
  } else {
    RFNSM_Status::Instance().set_state(nsmstate);
    b2nsm_error(msg, NULL);
  }
}

void RFNSM::m_Pause(NSMmsg* msg, NSMcontext* ctx)
{
  int nsmstate = RFNSM_Status::Instance().get_state();
  RFNSM_Status::Instance().set_state(RFSTATE_TRANSITION);
  int stat = g_nsmserver->Pause(msg, ctx);
  if (stat == 0) {
    RFNSM_Status::Instance().set_state(RFSTATE_IDLE);
    b2nsm_ok(msg, "Idle", NULL);
  } else {
    RFNSM_Status::Instance().set_state(nsmstate);
    b2nsm_error(msg, NULL);
  }
}

void RFNSM::m_Resume(NSMmsg* msg, NSMcontext* ctx)
{
  int nsmstate = RFNSM_Status::Instance().get_state();
  RFNSM_Status::Instance().set_state(RFSTATE_TRANSITION);
  int stat = g_nsmserver->Resume(msg, ctx);
  if (stat == 0) {
    RFNSM_Status::Instance().set_state(RFSTATE_RUNNING);
    b2nsm_ok(msg, "Running", NULL);
  } else {
    RFNSM_Status::Instance().set_state(nsmstate);
    b2nsm_error(msg, NULL);
  }
}

void RFNSM::m_Restart(NSMmsg* msg, NSMcontext* ctx)
{
  int nsmstate = RFNSM_Status::Instance().get_state();
  RFNSM_Status::Instance().set_state(RFSTATE_TRANSITION);
  int stat = g_nsmserver->Restart(msg, ctx);
  if (stat == 0) {
    RFNSM_Status::Instance().set_state(RFSTATE_CONFIGURED);
    b2nsm_ok(msg, "Configured", NULL);
  } else {
    RFNSM_Status::Instance().set_state(nsmstate);
    b2nsm_error(msg, NULL);
  }
}

void RFNSM::m_Status(NSMmsg* msg, NSMcontext* ctx)
{
  /* Old imp
  int stat = g_nsmserver->Status(msg, ctx);
  if (stat == 0)
    b2nsm_ok(msg, "Status", NULL);
  else
    b2nsm_error(msg, NULL);
  */
  int curstate = RFNSM_Status::Instance().get_state();
  b2nsm_ok(msg, RFSTATE[curstate].c_str(), NULL);

}

// Function to handle execution status
void RFNSM::m_OK(NSMmsg* msg, NSMcontext* ctx)
{
  RFNSM_Status& rfs = RFNSM_Status::Instance();
  int flag = rfs.get_flag();
  flag++;
  rfs.set_flag(flag);
  //  printf ( "OK received. flag set to %d\n", flag );
}

void RFNSM::m_ERROR(NSMmsg* msg, NSMcontext* ctx)
{
  RFNSM_Status::Instance().set_flag(-1);
  //  printf ( "ERROR received. m_flag set to -1\n" );
}


// Interface to Node Info

RfNodeInfo* RFNSM::GetNodeInfo()
{
  return m_info;
}

// RFNSM_Status implementation

RFNSM_Status::RFNSM_Status()
{
}

RFNSM_Status::~RFNSM_Status()
{
}

RFNSM_Status& RFNSM_Status::Instance()
{
  if (!s_instance) {
    s_instance = new RFNSM_Status;
  }
  return *s_instance;
}

void RFNSM_Status::set_flag(int val)
{
  m_flag = val;
}

int RFNSM_Status::get_flag()
{
  return m_flag;
}

void RFNSM_Status::set_state(int val)
{
  m_state = val;
}

int RFNSM_Status::get_state()
{
  return m_state;
}
