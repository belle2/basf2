#ifndef _EPICS_NSM_MSG_C_WRAPPER_H_
#define _EPICS_NSM_MSG_C_WRAPPER_H_

#include <longinRecord.h>
#include <stringinRecord.h>
#include <stringoutRecord.h>
#include <dbScan.h>

#ifdef __cplusplus
extern "C" {
#endif

long init_rc(const char* node, const char* host, int port);
long init_rc_longin(longinRecord* record, IOSCANPVT* pvt);
long init_rc_stringin(stringinRecord* record, IOSCANPVT* pvt);
long init_rc_stringout(stringoutRecord* record);
long get_ioint_info_rc_longin(int cmd, longinRecord* record, IOSCANPVT* pvt);
long get_ioint_info_rc_stringin(int cmd, stringinRecord* record, IOSCANPVT* pvt);
long read_rc_longin(longinRecord* record, int val);
long read_rc_stringin(stringinRecord* record, const char* val);

long init_rc_expno_longin(longinRecord* record);
long read_rc_expno_longin(longinRecord* record);
long init_rc_runno_longin(longinRecord* record);
long read_rc_runno_longin(longinRecord* record);
long init_rc_subno_longin(longinRecord* record);
long read_rc_subno_longin(longinRecord* record);

long init_rc_config_stringin(stringinRecord* record);
long read_rc_config_stringin(stringinRecord* record);

long init_rc_request_stringin(stringinRecord* record);
long read_rc_request_stringin(stringinRecord* record);

long init_rc_state_stringout(stringoutRecord* record);
long write_rc_state_stringout(stringoutRecord* record);

#ifdef __cplusplus
}
#endif

#endif
