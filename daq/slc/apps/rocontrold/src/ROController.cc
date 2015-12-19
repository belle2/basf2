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
  try {
    term();
    m_callback = callback;
    m_id = id;
    m_name = name;
    m_con.setCallback(m_callback);
    m_con.init(m_name, m_id);
    m_flow.open(&m_con.getInfo());
    initArguments(obj);
    return true;
  } catch (const std::exception& e) {
    throw (RCHandlerException(e.what()));
  }
}

bool ROController::term() throw()
{
  m_con.abort();
  m_con.getInfo().unlink();
  return true;
}

bool ROController::load(const DBObject& obj, int timeout)
throw(RCHandlerException)
{
  if (m_con.isAlive()) return true;
  try {
    m_con.clearArguments();
    if (loadArguments(obj)) {
      m_con.load(timeout);
    }
    return true;
  } catch (const std::exception& e) {
    throw (RCHandlerException(e.what()));
  }
}

bool ROController::start(int expno, int runno) throw(RCHandlerException)
{
  return m_con.start(expno, runno);
}

bool ROController::pause() throw()
{
  m_con.pause();
  return true;
}

bool ROController::resume(int /*subno*/) throw()
{
  m_con.resume();
  return true;
}

bool ROController::stop() throw(RCHandlerException)
{
  m_con.stop();
  return true;
}

bool ROController::abort() throw()
{
  m_con.abort();
  return true;
}

