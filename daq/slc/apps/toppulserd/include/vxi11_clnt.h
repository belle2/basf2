#include "daq/slc/apps/toppulserd/vxi11.h"

enum clnt_stat
device_abort_1(Device_Link* argp, Device_Error* clnt_res, CLIENT* clnt);

enum clnt_stat
create_link_1(Create_LinkParms* argp, Create_LinkResp* clnt_res, CLIENT* clnt);

enum clnt_stat
device_write_1(Device_WriteParms* argp, Device_WriteResp* clnt_res, CLIENT* clnt);

enum clnt_stat
device_read_1(Device_ReadParms* argp, Device_ReadResp* clnt_res, CLIENT* clnt);

enum clnt_stat
device_readstb_1(Device_GenericParms* argp, Device_ReadStbResp* clnt_res, CLIENT* clnt);

enum clnt_stat
device_trigger_1(Device_GenericParms* argp, Device_Error* clnt_res, CLIENT* clnt);

enum clnt_stat
device_clear_1(Device_GenericParms* argp, Device_Error* clnt_res, CLIENT* clnt);

enum clnt_stat
device_remote_1(Device_GenericParms* argp, Device_Error* clnt_res, CLIENT* clnt);

enum clnt_stat
device_local_1(Device_GenericParms* argp, Device_Error* clnt_res, CLIENT* clnt);

enum clnt_stat
device_lock_1(Device_LockParms* argp, Device_Error* clnt_res, CLIENT* clnt);

enum clnt_stat
device_unlock_1(Device_Link* argp, Device_Error* clnt_res, CLIENT* clnt);

enum clnt_stat
device_enable_srq_1(Device_EnableSrqParms* argp, Device_Error* clnt_res, CLIENT* clnt);

enum clnt_stat
device_docmd_1(Device_DocmdParms* argp, Device_DocmdResp* clnt_res, CLIENT* clnt);

enum clnt_stat
destroy_link_1(Device_Link* argp, Device_Error* clnt_res, CLIENT* clnt);

enum clnt_stat
create_intr_chan_1(Device_RemoteFunc* argp, Device_Error* clnt_res, CLIENT* clnt);

enum clnt_stat
destroy_intr_chan_1(void* argp, Device_Error* clnt_res, CLIENT* clnt);

enum clnt_stat
device_intr_srq_1(Device_SrqParms* argp, void* clnt_res, CLIENT* clnt);

