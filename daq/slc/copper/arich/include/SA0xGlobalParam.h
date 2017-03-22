/********************\
  SA0xGlobalParam.h

  update: 11/02/28

\********************/

#ifndef SA0xGlobalParam_H
#define SA0xGlobalParam_H

#include "daq/slc/copper/arich/SA0xParamBase.h"

class SA0xGlobalParam : public SA0xParamBase {
public:
  enum { MASK_PHASECMPS   = 0x0000003, SHIFT_PHASECMPS   =  0,
         MASK_GAIN        = 0x000000c, SHIFT_GAIN        =  2,
         MASK_SHAPINGTIME = 0x0000030, SHIFT_SHAPINGTIME =  4,
         MASK_COMPARATOR  = 0x0000040, SHIFT_COMPARATOR  =  6,
         MASK_VRDRIVE     = 0x0007f80, SHIFT_VRDRIVE     =  7,
         MASK_MONITOR     = 0x0018000, SHIFT_MONITOR     = 15,
         MASK_ID          = 0x3fe0000, SHIFT_ID          = 17,
         MASK_VARIABLE    = 0x001ffff
       };
public:
  SA0xGlobalParam() {}
  virtual ~SA0xGlobalParam() {}
  unsigned int getMaskVariable() { return MASK_VARIABLE; }
  /* individual value */
  void set_phasecmps(int x)
  {
    setvalue(x, MASK_PHASECMPS, SHIFT_PHASECMPS);
  }
  void set_gain(int x) { setvalue(x, MASK_GAIN, SHIFT_GAIN); }
  void set_shapingtime(int x)
  {
    setvalue(x, MASK_SHAPINGTIME, SHIFT_SHAPINGTIME);
  }
  void set_comparator(int x)
  {
    setvalue(x, MASK_COMPARATOR, SHIFT_COMPARATOR);
  }
  void set_vrdrive(int x) { setvalue(x, MASK_VRDRIVE, SHIFT_VRDRIVE); }
  void set_monitor(int x) { setvalue(x, MASK_MONITOR, SHIFT_MONITOR); }
  int getrb_phasecmps()
  {
    return getrbvalue(MASK_PHASECMPS, SHIFT_PHASECMPS);
  }
  int getrb_gain() { return getrbvalue(MASK_GAIN, SHIFT_GAIN); }
  int getrb_shapingtime()
  {
    return getrbvalue(MASK_SHAPINGTIME, SHIFT_SHAPINGTIME);
  }
  int getrb_comparator()
  {
    return getrbvalue(MASK_COMPARATOR, SHIFT_COMPARATOR);
  }
  int getrb_vrdrive() { return getrbvalue(MASK_VRDRIVE, SHIFT_VRDRIVE); }
  int getrb_monitor() { return getrbvalue(MASK_MONITOR, SHIFT_MONITOR); }
  int getrb_id() { return getrbvalue(MASK_ID, SHIFT_ID); }
  int getrb_masked() { return getrbvalue(MASK_VARIABLE, 0); }
};

#endif
