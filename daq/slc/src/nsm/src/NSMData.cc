#include "NSMData.hh"

#include <belle2nsm.h>

using namespace B2DAQ;

void* NSMData::open() throw(NSMHandlerException)
{
  if ((_pdata = b2nsm_openmem(_data_name.c_str(), _format.c_str(), _revision))
      == NULL) {
    throw (NSMHandlerException(__FILE__, __LINE__, "Failed to open data memory"));
  }
  return _pdata;
}

void* NSMData::allocate(int interval) throw(NSMHandlerException)
{
  if ((_pdata = b2nsm_allocmem(_data_name.c_str(), _format.c_str(), _revision, interval))
      == NULL) {
    throw (NSMHandlerException(__FILE__, __LINE__, "Failed to allocate data memory"));
  }
  return _pdata;
}

void* NSMData::get() throw(NSMHandlerException)
{
  if (_pdata == NULL) {
    throw (NSMHandlerException(__FILE__, __LINE__, "NSM data is not ready."));
  }
  return _pdata;
}

const void* NSMData::get() const throw(NSMHandlerException)
{
  if (_pdata == NULL) {
    throw (NSMHandlerException(__FILE__, __LINE__, "NSM data is not ready."));
  }
  return _pdata;
}
