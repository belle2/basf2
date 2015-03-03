#include "daq/slc/apps/rocontrold/ROController.h"

#include "daq/slc/apps/rocontrold/ROCallback.h"

#include <daq/slc/system/LogFile.h>

using namespace Belle2;

ROController::~ROController() throw()
{
  term();
}

bool ROController::init(ROCallback* callback, int id,
                        const std::string& name,
                        const DBObject& obj) throw()
{
  term();
  m_callback = callback;
  m_id = id;
  m_name = name;
  m_con.setCallback(m_callback);
  m_con.init(m_name, m_id);
  m_flow.open(&m_con.getInfo());
  return initArguments(obj);
}

bool ROController::term() throw()
{
  m_con.abort();
  m_con.getInfo().unlink();
  return true;
}

bool ROController::load(int timeout) throw()
{
  if (m_con.isAlive()) return true;
  m_con.clearArguments();
  loadArguments();
  return m_con.load(timeout);
}

bool ROController::start(int expno, int runno) throw()
{
  return m_con.start(expno, runno);
}

bool ROController::recover(int timeout) throw()
{
  return abort() && load(timeout);
}

bool ROController::abort() throw()
{
  m_con.abort();
  return true;
}

