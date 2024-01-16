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
  m_tmpdir = config.get("dqmmaster.tmpdir");
  m_instance = config.get("dqmmaster.instance");
  auto host = config.get("dqmmaster.host");
  auto port = config.getInt("dqmmaster.port");
  m_running = 0;
  LogFile::info("DqmMasterCallback : instance = %s, histodir = %s, tmpdir = %s", m_instance.c_str(), m_histodir.c_str(),
                m_tmpdir.c_str());

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

  {
    std::string filename = m_tmpdir + "/dqm_" + m_instance + "_runtype";
    // workaround until we have a better solution
    auto fh = fopen(filename.c_str(), "wt+");
    if (fh) {
      fprintf(fh, "%s", m_runtype.c_str());
      fclose(fh);
    }
  }
}

void DqmMasterCallback::start(int expno, int runno)
{
  m_expno = expno;
  m_runno = runno;

  // currently, we do not (yet) use exp and run nr, just add it in case it may be needed later
  {
    std::string filename = m_tmpdir + "/dqm_" + m_instance + "_expnr";
    // workaround until we have a better solution
    auto fh = fopen(filename.c_str(), "wt+");
    if (fh) {
      fprintf(fh, "%d", m_expno);
      fclose(fh);
    }
  }
  {
    std::string filename = m_tmpdir + "/dqm_" + m_instance + "_runnr";
    // workaround until we have a better solution
    auto fh = fopen(filename.c_str(), "wt+");
    if (fh) {
      fprintf(fh, "%d", m_runno);
      fclose(fh);
    }
  }

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

  while (m_sock->send(msg) < 0) {
    LogFile::error("Connection to histogramm server is missing in START: expno = %d, runno = %d, runtype %s", m_expno, m_runno,
                   m_runtype.c_str());
    m_sock->sock()->reconnect(10); // each one waits 5s
  }
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

  while (m_sock->send(msg) < 0) {
    LogFile::error("Connection closed during STOP, file not saved: expno = %d, runno = %d, runtype %s", m_expno, m_runno,
                   m_runtype.c_str());
    m_sock->sock()->reconnect(10); // each one waits 5s
    // we assume that the connection was terminated by a restart of the server
    // depending on when this happened, we may have new histograms to dump
    // EVEN if the DQM analysis could not handle it (because after restart there is no
    // run information.
  }
  delete (msg);
}

void DqmMasterCallback::abort(void)
{
  stop();
}





