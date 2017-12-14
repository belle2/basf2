#include "devNSM2CA.h"

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
  DEVSUPFUN read_ai;
  DEVSUPFUN special_linconv;
} dev_nsm2_vget_ai = {
  6, 
  NULL, 
  NULL, 
  init_nsm2_vget_ai, 
  get_ioint_info_nsm2_ai, 
  read_nsm2_vget_ai,
  NULL
};

struct
{
  long number;
  DEVSUPFUN dev_report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN get_ioint_info;
  DEVSUPFUN read_longin;
} dev_nsm2_vget_longin = {
  5, NULL, NULL, init_nsm2_vget_longin,
  get_ioint_info_nsm2_longin, read_nsm2_vget_longin
};

struct
{
  long number;
  DEVSUPFUN dev_report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN get_ioint_info;
  DEVSUPFUN read_stringin;
} dev_nsm2_vget_stringin = {
  5, NULL, NULL, init_nsm2_vget_stringin,
  get_ioint_info_nsm2_stringin, read_nsm2_vget_stringin
};

struct
{
  long number;
  DEVSUPFUN dev_report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN get_ioint_info;
  DEVSUPFUN write_ao;
  DEVSUPFUN special_linconv;
} dev_nsm2_vset_ao = {
  6, NULL, NULL, init_nsm2_vset_ao,
  NULL, write_nsm2_vset_ao, NULL
};

struct
{
  long number;
  DEVSUPFUN dev_report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN get_ioint_info;
  DEVSUPFUN write_longout;
} dev_nsm2_vset_longout = {
  5, NULL, NULL, init_nsm2_vset_longout,
  NULL, write_nsm2_vset_longout
};

struct
{
  long number;
  DEVSUPFUN dev_report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN get_ioint_info;
  DEVSUPFUN write_stringout;
} dev_nsm2_vset_stringout = {
  5, NULL, NULL, init_nsm2_vset_stringout,
  NULL, write_nsm2_vset_stringout
};

struct
{
  long num;
  DEVSUPFUN report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN get_ioint_info;
  DEVSUPFUN read_ai;
  DEVSUPFUN special_linconv;
} dev_nsm2_data_ai = {
  6, NULL, NULL, init_nsm2_data_ai,
  NULL, read_nsm2_data_ai, NULL
};

struct
{
  long num;
  DEVSUPFUN report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN get_ioint_info;
  DEVSUPFUN read_ai;
  DEVSUPFUN special_linconv;
} dev_nsm2_data_longin = {
  6, NULL, NULL, init_nsm2_data_longin,
  NULL, read_nsm2_data_longin, NULL
};

epicsExportAddress(dset, dev_nsm2_vget_ai);
epicsExportAddress(dset, dev_nsm2_vget_stringin);
epicsExportAddress(dset, dev_nsm2_vget_longin);
epicsExportAddress(dset, dev_nsm2_vset_ao);
epicsExportAddress(dset, dev_nsm2_vset_stringout);
epicsExportAddress(dset, dev_nsm2_vset_longout);
epicsExportAddress(dset, dev_nsm2_data_ai);
epicsExportAddress(dset, dev_nsm2_data_longin);
