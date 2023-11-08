/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <daq/dqm/DqmMasterCallback.h>
#include <daq/slc/system/LogFile.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>

#include <TText.h>
#include <TH1.h>
#include <TKey.h>

#include <unistd.h>

using namespace Belle2;
using namespace std;

int DqmMasterCallback::m_running = 0; // TODO

//-----------------------------------------------------------------
// Rbuf-Read Thread Interface
//-----------------------------------------------------------------
void* RunDqmMasterLogger(void*)
{
  return nullptr;
}

DqmMasterCallback::DqmMasterCallback(ConfigFile& config)
{
  m_histodir = config.get("dqmmaster.histodir");
  m_instance = config.get("dqmmaster.instance");
  auto host = config.get("dqmmaster.host");
  auto port = config.getInt("dqmmaster.port");
  m_running = 0;
  LogFile::info("DqmMasterCallback : instance = %s, histodir = %s", m_instance.c_str(), m_histodir.c_str());

  // Open sockets to hserver
  m_sock = new EvtSocketSend(host.c_str(), port);
}

DqmMasterCallback::~DqmMasterCallback()
{

}

void DqmMasterCallback::load(const DBObject& /* obj */, const std::string& runtype)
{
  m_runtype = runtype;
  LogFile::info("LOAD: runtype %s", m_runtype.c_str());
}

void DqmMasterCallback::start(int expno, int runno)
{
  m_expno = expno;
  m_runno = runno;

  MsgHandler hdl(0);
  int numobjs = 0;

  TText rc_clear(0, 0, "DQMRC:CLEAR");
  hdl.add(&rc_clear, "DQMRC:CLEAR");
  numobjs++;
  TText subdir(0, 0, "DQMInfo");
  hdl.add(&subdir, "SUBDIR:DQMInfo") ;
  numobjs++;
  TH1F h_expno("expno", to_string(m_expno).c_str(), 1, 0, 1);
  hdl.add(&h_expno, "expno");
  numobjs++;
  TH1F h_runno("runno", to_string(m_runno).c_str(), 1, 0, 1);
  hdl.add(&h_runno, "runno");
  numobjs++;
  TH1F h_rtype("rtype", m_runtype.c_str(), 1, 0, 1);
  hdl.add(&h_rtype, "rtype");
  numobjs++;
  TText command(0, 0, "COMMAND:EXIT");
  hdl.add(&command, "SUBDIR:EXIT");
  numobjs++;
  TText rc_merge(0, 0, "DQMRC:MERGE");
  hdl.add(&rc_merge, "DQMRC:MERGE");
  numobjs++;

  EvtMessage* msg = hdl.encode_msg(MSG_EVENT);
  (msg->header())->reserved[0] = 0;
  (msg->header())->reserved[1] = numobjs;

  m_sock->send(msg);
  delete (msg);

  LogFile::info("START: expno = %d, runno = %d, runtype %s", m_expno, m_runno, m_runtype.c_str());
  m_running = 1;
}

void DqmMasterCallback::stop(void)
{
  LogFile::info("STOP: expno = %d, runno = %d, runtype %s", m_expno, m_runno, m_runtype.c_str());

  if (m_running == 0) return;

  m_running = 0;

  char outfile[1024];

  MsgHandler hdl(0);
  int numobjs = 0;

  snprintf(outfile, sizeof(outfile), "DQMRC:SAVE:%s/%sdqm_e%4.4dr%6.6d.root", m_histodir.c_str(), m_instance.c_str(), m_expno,
           m_runno);

  TText rc_save(0, 0, outfile);
  hdl.add(&rc_save, outfile);
  numobjs++;

  EvtMessage* msg = hdl.encode_msg(MSG_EVENT);
  (msg->header())->reserved[0] = 0;
  (msg->header())->reserved[1] = numobjs;

  m_sock->send(msg);
  delete (msg);
}

void DqmMasterCallback::abort(void)
{
  stop();
}





