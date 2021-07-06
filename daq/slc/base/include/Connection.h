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
    Connection(const Enum& st) ;
    Connection(const Connection& st) ;
    Connection(const std::string& st) { *this = st; }
    Connection(int id) { *this = id; }
    ~Connection() ;

  private:
    Connection(int id, const char* label);

  public:
    bool operator==(const Connection& st) const;
    bool operator!=(const Connection& st) const;
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
