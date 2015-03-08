#include "devRC.h"

#include <epicsExport.h>
#include <dbAccess.h>
#include <dbScan.h>
#include <devSup.h>
#include <recSup.h>
#include <recGbl.h>

struct
{
  long number;
  DEVSUPFUN dev_report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN get_ioint_info;
  DEVSUPFUN read_longin;
} dev_rc_expno = {
  5, NULL, NULL, init_rc_expno_longin,
  get_ioint_info_rc_longin, read_rc_expno_longin
};

struct
{
  long number;
  DEVSUPFUN dev_report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN get_ioint_info;
  DEVSUPFUN read_longin;
} dev_rc_runno = {
  5, NULL, NULL, init_rc_runno_longin,
  get_ioint_info_rc_longin, read_rc_runno_longin
};

struct
{
  long number;
  DEVSUPFUN dev_report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN get_ioint_info;
  DEVSUPFUN read_longin;
} dev_rc_subno = {
  5, NULL, NULL, init_rc_subno_longin,
  get_ioint_info_rc_longin, read_rc_subno_longin
};

struct
{
  long number;
  DEVSUPFUN dev_report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN get_ioint_info;
  DEVSUPFUN read_stringin;
} dev_rc_config = {
  5, NULL, NULL, init_rc_config_stringin,
  get_ioint_info_rc_stringin, read_rc_config_stringin
};

struct
{
  long number;
  DEVSUPFUN dev_report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN get_ioint_info;
  DEVSUPFUN read_stringin;
} dev_rc_request = {
  5, NULL, NULL, init_rc_request_stringin,
  get_ioint_info_rc_stringin, read_rc_request_stringin
};

struct
{
  long number;
  DEVSUPFUN dev_report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN get_ioint_info;
  DEVSUPFUN write_stringout;
} dev_rc_state = {
  5, NULL, NULL, init_rc_state_stringout,
  NULL, write_rc_state_stringout
};

epicsExportAddress(dset, dev_rc_expno);
epicsExportAddress(dset, dev_rc_runno);
epicsExportAddress(dset, dev_rc_subno);
epicsExportAddress(dset, dev_rc_config);
epicsExportAddress(dset, dev_rc_request);
epicsExportAddress(dset, dev_rc_state);
