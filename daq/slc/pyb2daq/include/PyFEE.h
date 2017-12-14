#ifndef _Belle2_PyFEE_h
#define _Belle2_PyFEE_h

#include <daq/slc/copper/FEE.h>

#include <boost/python.hpp>
#include <boost/ref.hpp>

namespace Belle2 {

  class PyFEE : public FEE {

  private:
    static bool g_pyinitialized;
    static boost::python::object g_ns;
    static boost::python::object g_init;
    static boost::python::object g_boot;
    static boost::python::object g_load;
    static boost::python::object g_monitor;

  public:
    PyFEE(const std::string& script);
    virtual ~PyFEE() throw();

  public:
    virtual void init(RCCallback& callback, HSLB& hslb, const DBObject& obj);
    virtual void boot(RCCallback& callback, HSLB& hslb, const DBObject& obj);
    virtual void load(RCCallback& callback, HSLB& hslb, const DBObject& obj);
    virtual void monitor(RCCallback& callback, HSLB& hslb);

  private:
    boost::python::object m_ns;
    boost::python::object m_init;
    boost::python::object m_boot;
    boost::python::object m_load;
    boost::python::object m_monitor;

  };

}

#endif
