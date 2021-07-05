/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_NSMCommand_hh
#define _Belle2_NSMCommand_hh

#include "daq/slc/base/Enum.h"

namespace Belle2 {

  class NSMCommand : public Enum {

  public:
    static const NSMCommand OK;
    static const NSMCommand STATE;
    static const NSMCommand ERROR;
    static const NSMCommand FATAL;
    static const NSMCommand LOG;
    static const NSMCommand LOGSET;
    static const NSMCommand LOGGET;
    static const NSMCommand VLISTGET;
    static const NSMCommand VLISTSET;
    static const NSMCommand VGET;
    static const NSMCommand VSET;
    static const NSMCommand VREPLY;

  public:
    static const NSMCommand DATAGET;
    static const NSMCommand DATASET;
    static const NSMCommand DBGET;
    static const NSMCommand DBSET;
    static const NSMCommand DBLISTGET;
    static const NSMCommand DBLISTSET;
    static const NSMCommand LOGLIST;

  public:
    static const int SUGGESTED = 2;
    static const int ENABLED = 1;
    static const int DISABLED = 0;

  public:
    NSMCommand() {}
    NSMCommand(const Enum& e) : Enum(e) {}
    NSMCommand(const NSMCommand& cmd) : Enum(cmd) {}
    NSMCommand(const char* label) { *this = label; }
    NSMCommand(const std::string& label) { *this = label; }
    NSMCommand(int id) { *this = id; }
    ~NSMCommand() {}

  public:
    NSMCommand(int id, const char* label)
      : Enum(id, label) {}

  public:
    const NSMCommand& operator=(const std::string& label);
    const NSMCommand& operator=(const char* label);
    const NSMCommand& operator=(int id);

  };

}

#endif
