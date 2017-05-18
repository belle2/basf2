#ifndef __SCROD_PS_DEFS__
#define __SCROD_PS_DEFS__

const int PC_PASS = 0;
const int PC_FAIL_GENERIC = 1;
const int PC_FAIL_DISABLE_PAT_GEN = 2;
const int PC_FAIL_REG_LD = 3;
const int PC_FAIL_REG_LATCH = 4;
const int PC_FAIL_SET_WIN = 5;
const int PC_FAIL_TRIG_MASK = 6;
const int PC_FAIL_SET_RD_MODE = 7;
const int PC_FAIL_SET_LOOKBACK = 8;
const int PC_FAIL_NO_MEM = 9;
const int PC_FAIL_DATA_TIMEOUT = 10;
const int PC_FAIL_SET_IRSX_NUM = 11;
const int PC_FAIL_NO_CARRIERS = 12;
const int PC_FAIL_INVALID_CARRIER = 13;
const int PC_FAIL_CLR_CAL = 14;
const int PC_WARN_DATA_ISSUES = 32;
//Defines location of info for PC_ERR_ADDR when Error is PC_WARN_DATA_ISSUES
const int MISSING_ASIC_BITMASK_OFFSET = 0;
const int BAD_FOOTER_BITMASK_OFFSET = 16;
const int PC_FAIL_PS_HALT = 255;

const int ONLINE_PED_CALC_CARRIER_MASK = 0x0F;//Defines which carriers to run
const int ONLINE_PED_CALC_RMS_MASK = 0xF0;//Defines whether user wants RMS calc - takes longer
const int ONLINE_PED_CALC_RMS_OFFSET = 4;
const int ONLINE_PED_CALC_PROGRESS_MASK = 0xFFFF;
const int ONLINE_PED_CALC_PROGRESS_OFFSET = 0;
const int ONLINE_PED_CALC_ERROR_CODE_MASK = 0xFF000000;//Defines erorr status of ped calc
const int ONLINE_PED_CALC_ERROR_CODE_OFFSET = 24;

#endif
