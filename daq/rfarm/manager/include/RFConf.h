/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef RFCONF_H
#define RFCONF_H

#include <stdio.h>

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
