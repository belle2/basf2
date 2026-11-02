/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_Connection_hh
#define _Belle2_Connection_hh

#include "daq/slc/base/Enum.h"

namespace Belle2 {

  class Connection : public Enum {

  public:
    static const Connection ONLINE;
    static const Connection OFFLINE;

  public:
    Connection();
    // cppcheck-suppress noExplicitConstructor ; implicit conversion is part of the Enum-style API
    Connection(const Enum& st) ;
    Connection(const Connection& st) ;
    explicit Connection(const std::string& st) { *this = st; }
    explicit Connection(int id) { *this = id; }
    ~Connection() ;

  private:
    Connection(int id, const char* label);

  public:
    bool operator==(const Connection& st) const;
    bool operator!=(const Connection& st) const;

    /**
     * Operator =.
     */
    const Connection& operator=(const Connection& connection);
    const Connection& operator=(const std::string& msg);
    const Connection& operator=(const char* msg);
    const Connection& operator=(int i);

  };

  inline bool Connection::operator==(const Connection& st) const
  {
    return (getId() == st.getId());
  }

  inline bool Connection::operator!=(const Connection& st) const
  {
    return (getId() != st.getId());
  }

}

#endif
