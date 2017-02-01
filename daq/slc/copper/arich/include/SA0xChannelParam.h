/********************\
  SA0xChannelParam.h

  update: 11/02/28

\********************/

#ifndef SA0xChannelParam_H
#define SA0xChannelParam_H

#include "daq/slc/copper/arich/SA0xParamBase.h"

class SA0xChannelParam : public SA0xParamBase {
public:
  enum { MASK_DECAYTIME      = 0x00007, SHIFT_DECAYTIME      = 0,
         MASK_OFFSET         = 0x00078, SHIFT_OFFSET         = 3,
         MASK_FINEADJ_UNIPOL = 0x00780, SHIFT_FINEADJ_UNIPOL = 7,
         MASK_FINEADJ_DIFF   = 0x07800, SHIFT_FINEADJ_DIFF   = 11,
         MASK_RESERVED       = 0x08000, SHIFT_RESERVED       = 15,
         MASK_TPENB          = 0x10000, SHIFT_TPENB          = 16,
         MASK_KILL           = 0x20000, SHIFT_KILL           = 17,
         MASK_VARIABLE       = 0x2ffff
       };
public:
  SA0xChannelParam() {}
  virtual ~SA0xChannelParam() {}
  unsigned int getMaskVariable() { return MASK_VARIABLE; }
  /* individual value */
  void set_decaytime(int x)
  {
    setvalue(x, MASK_DECAYTIME, SHIFT_DECAYTIME);
  }
  void set_offset(int x) { setvalue(x, MASK_OFFSET, SHIFT_OFFSET); }
  void set_fineadj_unipol(int x)
  {
    setvalue(x, MASK_FINEADJ_UNIPOL, SHIFT_FINEADJ_UNIPOL);
  }
  void set_fineadj_diff(int x)
  {
    setvalue(x, MASK_FINEADJ_DIFF, SHIFT_FINEADJ_DIFF);
  }
  void set_tpenb(int x) { setvalue(x, MASK_TPENB, SHIFT_TPENB); }
  void set_kill(int x) { setvalue(x, MASK_KILL, SHIFT_KILL); }

  int getrb_decaytime()
  {
    return getrbvalue(MASK_DECAYTIME, SHIFT_DECAYTIME);
  }
  int getrb_offset() { return getrbvalue(MASK_OFFSET, SHIFT_OFFSET); }
  int getrb_fineadj_unipol()
  {
    return getrbvalue(MASK_FINEADJ_UNIPOL, SHIFT_FINEADJ_UNIPOL);
  }
  int getrb_fineadj_diff()
  {
    return getrbvalue(MASK_FINEADJ_DIFF, SHIFT_FINEADJ_DIFF);
  }
  int getrb_tpenb() { return getrbvalue(MASK_TPENB, SHIFT_TPENB); }
  int getrb_kill() { return getrbvalue(MASK_KILL, SHIFT_KILL); }
};

#endif
