#ifndef Belle2_ECLShaper_h
#define Belle2_ECLShaper_h

#include <daq/slc/base/IOException.h>

namespace Belle2 {

  class ECLShaper {

  public:
    static const int ECL_TCP_STATUS_PORT = 6001;

  public:
    ECLShaper() :
      m_id(0) {}
    ECLShaper(int id, const std::string& host):
      m_id(id), m_host(host) {}
    ~ECLShaper() throw() {}

  public:
    bool boot(int shm_num, int mem_addr) throw(IOException);
    bool init(int shm_num, int mem_addr) throw(IOException);
    int read(int sh_num, int mem_addr) throw(IOException);
    int write(int sh_num, int mem_addr, int reg_wdata) throw(IOException);

  public:
    int getId() const throw() { return m_id; }
    const std::string& getHost() const throw() { return m_host; }
    int* getData() throw() { return m_reg_data; }

  private:
    int m_id;
    std::string m_host;
    int m_reg_data[12];

  };

};

#endif
