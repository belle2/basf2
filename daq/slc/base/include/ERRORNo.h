/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_ERRORNo_hh
#define _Belle2_ERRORNo_hh

#include "daq/slc/base/Enum.h"

namespace Belle2 {

  class ERRORNo : public Enum {

  public:
    // cppcheck-suppress duplInheritedMember ; the derived class deliberately provides its own
    static const ERRORNo UNKNOWN;
    static const ERRORNo NSMONLINE;
    static const ERRORNo NSMSENDREQ;
    static const ERRORNo NSMMEMOPEN;
    static const ERRORNo NSMINIT;
    static const ERRORNo DATABASE;

  public:
    ERRORNo() {}
    // cppcheck-suppress noExplicitConstructor ; implicit conversion is part of the Enum-style API
    ERRORNo(const Enum& e) : Enum(e) {}
    // cppcheck-suppress noExplicitConstructor ; implicit conversion is part of the Enum-style API
    ERRORNo(const char* label) { *this = label; }
    explicit ERRORNo(int id) { *this = id; }
    ~ERRORNo() {}

  protected:
    ERRORNo(int id, const char* label)
      : Enum(id, label) {}

  public:
    const ERRORNo& operator=(const std::string& label);
    const ERRORNo& operator=(const char* label);
    const ERRORNo& operator=(int id);

  };

}

#endif
