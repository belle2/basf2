#ifndef Belle2_ECLShaperController_h
#define Belle2_ECLShaperController_h

#include <daq/slc/base/IOException.h>

namespace Belle2 {

  class ECLShaperController {

  public:
    static const int ECL_TCP_STATUS_PORT = 6001;

  public:
    ECLShaperController() :
      m_id(0) {}
    ECLShaperController(int id, const std::string& host):
      m_id(id), m_host(host) {}
    ~ECLShaperController() throw() {}

  public:
    bool boot(int shm_num, int mem_addr) throw(IOException);
    bool init(int shm_num, int mem_addr) throw(IOException);

  public:
    int getId() const throw() { return m_id; }
    const std::string& getHost() const throw() { return m_host; }

  private:
    int m_id;
    std::string m_host;

  };

};

#endif
