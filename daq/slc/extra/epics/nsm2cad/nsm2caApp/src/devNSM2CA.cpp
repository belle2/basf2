#include "NSM2CA.h"

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/PThread.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <recGbl.h>
#include <dbAccess.h>
#include <dbScan.h>
#include <dbEvent.h>
#include <dbDefs.h>
#include <devSup.h>

using namespace Belle2;

NSM2CA g_nsm;

extern "C" {

bool checkinRecord(unsigned int type, unsigned int scan, void* record)
{
  if (type != INST_IO) {
    recGblRecordError(S_db_badField, record, "init_record Illegal INP field. INT_IO only."); 
    return false;
  }
  if (scan != SCAN_IO_EVENT) {
    recGblRecordError(S_db_badField, record, "init_record Illegal SCAN field. IO/INTR scans only.");
    return false;
  }
  return true;
}

bool checkoutRecord(unsigned int scan, void* record)
{
  if (scan != SCAN_PASSIVE) {
    recGblRecordError(S_db_badField, (void*)record,
		      "init_record Illegal SCAN field. PASSIVE scans only.");
    return false;
  }
  return true;
}

long init_nsm2(const char* node, const char* host, int port)
{
  NSM2CACallback::get().setCA(&g_nsm);
  NSM2CACallback::get().setNode(NSMNode(node));
  PThread(new NSMNodeDaemon(&(NSM2CACallback::get()), host, port));
  return 0;
}

long get_ioint_info_nsm2_ai(int cmd, aiRecord* record, IOSCANPVT* pvt)
{
  if (cmd != 0) return 0;
  if (!record->dpvt) return 1;
  *pvt = *((IOSCANPVT*)record->dpvt);
  return 0;
}

long get_ioint_info_nsm2_longin(int cmd, longinRecord* record, IOSCANPVT* pvt)
{
  if (cmd != 0) return 0;
  if (!record->dpvt) return 1;
  *pvt = *((IOSCANPVT*)record->dpvt);
  return 0;
}

long get_ioint_info_nsm2_stringin(int cmd, stringinRecord* record, IOSCANPVT* pvt)
{
  if (cmd != 0) return 0;
  if (!record->dpvt) return 1;
  *pvt = *((IOSCANPVT*)record->dpvt);
  return 0;
}

long init_nsm2_data_ai(aiRecord* record)
{
  if (record->inp.type != INST_IO) {
    recGblRecordError(S_db_badField, (void*)record,
		      "init_record Illegal INP field. INT_IO only.");
    return S_db_badField;
  }
  if ((record->scan == SCAN_IO_EVENT) ||
      (record->scan == SCAN_PASSIVE)) {
    recGblRecordError(S_db_badField, (void*)record,
		      "Ai (init_record) Illegal SCAN field. Periodic scans only.");
    return S_db_badField;
  }
  IOSCANPVT* pvt = new IOSCANPVT;
  scanIoInit(pvt);
  record->dpvt = pvt;
  return g_nsm.init_data_in(record->name)? 0: S_db_badField;
}

long init_nsm2_data_longin(longinRecord* record)
{
  if (record->inp.type != INST_IO) {
    recGblRecordError(S_db_badField, (void*)record,
		      "init_record Illegal INP field. INT_IO only.");
    return S_db_badField;
  }
  if ((record->scan == SCAN_IO_EVENT) ||
      (record->scan == SCAN_PASSIVE)) {
    recGblRecordError(S_db_badField, (void*)record,
		      "Ai (init_record) Illegal SCAN field. Periodic scans only.");
    return S_db_badField;
  }
  IOSCANPVT* pvt = new IOSCANPVT;
  scanIoInit(pvt);
  record->dpvt = pvt;
  return g_nsm.init_data_in(record->name)? 0: S_db_badField;
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
  IOSCANPVT* pvt;
  if(checkinRecord(record->inp.type, record->scan, record)
     && (pvt = g_nsm.init_vget_in(record->name)) != NULL) {
    scanIoInit(pvt);
    record->dpvt = pvt;
    return 0;
  }
  return S_db_badField;
}

long init_nsm2_vget_longin(longinRecord* record)
{
  IOSCANPVT* pvt;
  if(checkinRecord(record->inp.type, record->scan, record)
     && (pvt = g_nsm.init_vget_in(record->name)) != NULL) {
    scanIoInit(pvt);
    record->dpvt = pvt;
    return 0;
  }
  return S_db_badField;
}

long init_nsm2_vget_stringin(stringinRecord* record)
{
  IOSCANPVT* pvt;
  if(checkinRecord(record->inp.type, record->scan, record)
     && (pvt = g_nsm.init_vget_in(record->name)) != NULL) {
    scanIoInit(pvt);
    record->dpvt = pvt;
    return 0;
  }
  return S_db_badField;
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
  record->udf = TRUE;
  if(checkoutRecord(record->scan, record) 
     && g_nsm.init_vset_out(record->name, record)) {
    return 0;
  }
  return S_db_badField;
}

long init_nsm2_vset_longout(longoutRecord *record)
{
  record->udf = TRUE;
  if(checkoutRecord(record->scan, record) 
     && g_nsm.init_vset_out(record->name, record)) {
    return 0;
  }
  return S_db_badField;
}

long init_nsm2_vset_stringout(stringoutRecord *record)
{
  record->udf = TRUE;
  if(checkoutRecord(record->scan, record) 
     && g_nsm.init_vset_out(record->name, record)) {
    return 0;
  }
  return S_db_badField;
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
