/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_NSMNode_hh
#define _Belle2_NSMNode_hh

#include <daq/slc/base/Date.h>

#include "daq/slc/nsm/NSMState.h"

namespace Belle2 {

  class NSMNode {

  public:
    NSMNode(const std::string& name = "");
    virtual ~NSMNode() {}

  public:
    const std::string& getName() const { return m_name; }
    int getId() const { return m_id; }
    bool isUsed() const { return m_used; }
    NSMState& getState() { return m_state; }
    const NSMState& getState() const { return m_state; }
    void setName(const std::string& name);
    void setUsed(bool used) { m_used = used; }
    void setState(const NSMState& state) { m_state = state; }
    void setId(int id) { m_id = id; }
    int getDate() const { return m_date; }
    void setDate(int date) { m_date = date; }
    void setDate(const Date& date) { m_date = date.get(); }

  protected:
    std::string m_name;
    int m_id;
    bool m_used;
    NSMState m_state;
    int m_date;

  };

};

#endif
