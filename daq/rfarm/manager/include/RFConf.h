#ifndef RFCONF_H
#define RFCONF_H
//+
// File : RFConf.h
// Description : Configuration Manager for RFARM
//
// Author : Ryosuke Itoh, KEK
// Date : 12 - Jun - 2013
//-

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

namespace Belle2 {

  struct RFConf_t {
    char* key; char* val; struct RFConf_t* next;
  };


  class RFConf {
  public:
    RFConf(const char* file);
    ~RFConf();

    char* getconf(const char* key1,
                  const char* key2 = NULL,
                  const char* key3 = NULL);

    int getconfi(const char* key1,
                 const char* key2 = NULL,
                 const char* key3 = NULL);

  private:
    FILE* m_fd;

  };
}

#endif
