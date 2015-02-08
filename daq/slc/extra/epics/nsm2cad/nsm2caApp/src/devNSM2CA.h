#ifndef _EPICS_NSM_MSG_C_WRAPPER_H_
#define _EPICS_NSM_MSG_C_WRAPPER_H_

#include <aiRecord.h>
#include <longinRecord.h>
#include <stringinRecord.h>
#include <stringoutRecord.h>
#include <dbScan.h>

#ifdef __cplusplus
extern "C" {
#endif

long init_nsm2(const char* node, const char* host, int port);
long init_nsm2_longin(longinRecord* record, IOSCANPVT* pvt);
long init_nsm2_stringin(stringinRecord* record, IOSCANPVT* pvt);
long init_nsm2_stringout(stringoutRecord* record);
long get_ioint_info_nsm2_longin(int cmd, longinRecord* record, IOSCANPVT* pvt);
long get_ioint_info_nsm2_stringin(int cmd, stringinRecord* record, IOSCANPVT* pvt);
long get_ioint_info_nsm2_data_ai(int cmd, aiRecord* record, IOSCANPVT* pvt);
long get_ioint_info_nsm2_data_longin(int cmd, longinRecord* record, IOSCANPVT* pvt);

long read_nsm2_ai(aiRecord* record, double val);
long read_nsm2_longin(longinRecord* record, int val);
long read_nsm2_stringin(stringinRecord* record, const char* val);

long init_nsm2_data_ai(aiRecord* record);
long read_nsm2_data_ai(aiRecord* record);

long init_nsm2_data_longin(longinRecord* record);
long read_nsm2_data_longin(longinRecord* record);

long init_nsm2_request_stringin(stringinRecord* record);
long read_nsm2_request_stringin(stringinRecord* record);

long init_nsm2_state_stringin(stringinRecord* record);
long read_nsm2_state_stringin(stringinRecord* record);

long init_nsm2_request_stringout(stringoutRecord* record);
long write_nsm2_request_stringout(stringoutRecord* record);

#ifdef __cplusplus
}
#endif

#endif
