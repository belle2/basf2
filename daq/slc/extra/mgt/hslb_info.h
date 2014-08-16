#ifndef _hslb_info_h
#define _hslb_info_h

struct hslb_info {
  int fd;
  int fin;
  int fee8[0x67];
  int crcerr;
  int b2lcsr;
  int fee32[4];
  int hsd32;
  int hsa32;
  int hslbhw;
  int hslbfw;
  int hsstat;
  int hscont;
  int cclk;
  int conf;
  int cpldver;
  int feehw;
  int feeserial;
  int feetype;
  int feever;
  int serial[2];
  int type[2];
  int hslbid;
  int hslbver;
  int reset;
  int b2lstat;
  int rxdata;
  int fwevt;
  int fwclk;
  int cntsec;
};

#endif
