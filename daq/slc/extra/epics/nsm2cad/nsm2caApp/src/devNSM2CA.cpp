#include "NSM2CA.h"

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/PThread.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <recGbl.h>
#include <dbAccess.h>

using namespace Belle2;

NSM2CA g_nsm;

extern "C" {

long init_nsm2(const char* node, const char* host, int port)
{
  NSM2CACallback::get().setNode(NSMNode(node));
  PThread(new NSMNodeDaemon(&(NSM2CACallback::get()), host, port));
  return 0;
}

long init_nsm2_ai(aiRecord *record, IOSCANPVT *pvt)
{
  return g_nsm.init_in(record, pvt);
}

long init_nsm2_longin(longinRecord *record, IOSCANPVT *pvt)
{
  return g_nsm.init_in(record, pvt);
}

long init_nsm2_stringin(stringinRecord *record, IOSCANPVT *pvt)
{
  return g_nsm.init_in(record, pvt);
}

long init_nsm2_ao(aoRecord* record)
{
  return g_nsm.init_out(record);
}

long init_nsm2_longout(longoutRecord* record)
{
  return g_nsm.init_out(record);
}

long init_nsm2_stringout(stringoutRecord* record)
{
  return g_nsm.init_out(record);
}

long get_ioint_info_nsm2_ai(int cmd, aiRecord* record, IOSCANPVT* pvt)
{
  return g_nsm.get_ioint_info_in(cmd, record, pvt);
}

long get_ioint_info_nsm2_longin(int cmd, longinRecord* record, IOSCANPVT* pvt)
{
  return g_nsm.get_ioint_info_in(cmd, record, pvt);
}

long get_ioint_info_nsm2_stringin(int cmd, stringinRecord* record, IOSCANPVT* pvt)
{
  return g_nsm.get_ioint_info_in(cmd, record, pvt);
}

long init_nsm2_data_ai(aiRecord* record)
{
  return g_nsm.init_data_in(record);
}

long init_nsm2_data_longin(longinRecord* record)
{
  return g_nsm.init_data_in(record);
}

long read_nsm2_data_ai(aiRecord* record)
{
  return g_nsm.read_data_in(record);
}

long read_nsm2_data_longin(longinRecord* record)
{
  return g_nsm.read_data_in(record);
}

long init_nsm2_vget_ai(aiRecord* record)
{
  return g_nsm.init_vget_in(record, "float");
}

long init_nsm2_vget_longin(longinRecord* record)
{
  return g_nsm.init_vget_in(record, "int");
}

long init_nsm2_vget_stringin(stringinRecord* record)
{
  return g_nsm.init_vget_in(record, "text");
}

long read_nsm2_vget_ai(aiRecord* record)
{
  return g_nsm.read_vget_in(record);
}

long read_nsm2_vget_longin(longinRecord* record)
{
  return g_nsm.read_vget_in(record);
}

long read_nsm2_vget_stringin(stringinRecord* record)
{
  return g_nsm.read_vget_in(record);
}

long init_nsm2_vset_ao(aoRecord *record)
{
  return g_nsm.init_vset_out(record);
}

long init_nsm2_vset_longout(longoutRecord *record)
{
  return g_nsm.init_vset_out(record);
}

long init_nsm2_vset_stringout(stringoutRecord *record)
{
  return g_nsm.init_vset_out(record);
}

long write_nsm2_vset_ao(aoRecord *record)
{
  return g_nsm.write_vset_out(record);
}

long write_nsm2_vset_longout(longoutRecord *record)
{
  return g_nsm.write_vset_out(record);
}

long write_nsm2_vset_stringout(stringoutRecord *record)
{
  return g_nsm.write_vset_out(record);
}

}
