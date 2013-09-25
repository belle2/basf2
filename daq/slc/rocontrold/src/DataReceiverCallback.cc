#include "DataReceiverCallback.hh"

#include "RecieverManager.hh"
#include "EventBuilderManager.hh"

#include <nsm/RunStatus.hh>
#include <nsm/DataReceiverData.hh>

#include <node/DataReceiverNode.hh>

#include <system/Fork.hh>

#include <util/Debugger.hh>
#include <util/StringUtil.hh>

using namespace B2DAQ;

DataReceiverCallback::DataReceiverCallback(DataReceiverNode* node, DataReceiverData* data)
  : RCCallback(node), _node(node), _data(data)
{
  _status = new RunStatus("RUN_STATUS");
  _buf_config = NULL;
  _buf_status = NULL;
}

DataReceiverCallback::~DataReceiverCallback() throw()
{
  delete _status;
}

bool DataReceiverCallback::boot() throw()
{
  while (!_status->isAvailable()) {
    try {
      _status->open();
    } catch (const NSMHandlerException& e) {
      B2DAQ::debug("TTD daemon : Failed to open run status. Waiting for 5 seconds..");
      sleep(5);
    }
  }
  try {
    if (_data != NULL) {
      _data->read(_node);
    }
  } catch (const NSMHandlerException& e) {
    B2DAQ::debug("Failed to access NSM data.");
    return false;
  }
  return true;
}

bool DataReceiverCallback::reboot() throw()
{
  return boot();
}

bool DataReceiverCallback::load() throw()
{
  try {
    if (_data != NULL) {
      _data->read(_node);
    }
  } catch (const NSMHandlerException& e) {
    B2DAQ::debug("Failed to access NSM data.");
    return false;
  }
  system("killall basf2");
  system("killall eb0");
  _fork_v[0].cancel();
  _fork_v[1].cancel();
  _fork_v[0] = Fork(new EventBuilderManager(_node));
  _fork_v[1] = Fork(new RecieverManager(_node));
  return true;
}

bool DataReceiverCallback::reload() throw()
{
  return load();
}

bool DataReceiverCallback::start() throw()
{
  return true;
}

bool DataReceiverCallback::stop() throw()
{
  return true;
}

bool DataReceiverCallback::resume() throw()
{
  return true;
}

bool DataReceiverCallback::pause() throw()
{
  return true;
}

bool DataReceiverCallback::recover() throw()
{
  return true;
}

bool DataReceiverCallback::abort() throw()
{
  return true;
}
