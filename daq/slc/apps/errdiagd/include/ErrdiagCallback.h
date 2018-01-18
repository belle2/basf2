#ifndef _Belle2_ErrdiagCallback_h
#define _Belle2_ErrdiagCallback_h

#include "daq/slc/nsm/NSMCallback.h"

#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <daq/slc/psql/PostgreSQLInterface.h>

using namespace boost::property_tree;

namespace Belle2 {

  class ErrdiagCallback : public NSMCallback {

  public:
    ErrdiagCallback(const std::string& nodename, int timout = 5);
    virtual ~ErrdiagCallback() throw();

  public:
    virtual void init(NSMCommunicator& com) throw();
    virtual void timeout(NSMCommunicator& com) throw();
    virtual void show(ptree apt, std::string str) throw();
    virtual int test1() throw();
    virtual int test2() throw();

  private:


    ptree m_pt_fee;
    ptree m_pt_dont_touch; // I don't know the reason but touching the second ptree causes glibc error...
    ptree m_pt_ttd;
    ptree m_pt_cprropc;
    ptree m_pt_hlt;
    ptree m_pt_eb;
    ptree m_pt_storage;
    ptree m_pt_slc;

    PostgreSQLInterface* m_db;

  };

}

#endif
