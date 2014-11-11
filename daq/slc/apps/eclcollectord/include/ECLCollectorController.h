#ifndef Belle2_ECLCollectorController_h
#define Belle2_ECLCollectorController_h

#include <daq/slc/system/TCPSocket.h>

#include <string>
#include <vector>

namespace Belle2 {

  class ECLCollectorController {
  public:
    class Shaper {
    public:
      int id;
      std::string host;
    };
    class ShaperReg {
    public:
      ShaperReg() {}
      ShaperReg(int adr, int val, const char* name) {
        this->adr = adr;
        this->val = val;
        this->name = name;
      }
    public:
      int adr;
      int val;
      std::string name;
    };

  public:
    ECLCollectorController(const std::string& hostname,
                           unsigned int port = 6001)
      : m_hostname(hostname), m_socket(hostname, port) {}
    ~ECLCollectorController() {}

  public:
    bool boot(const std::string& config);
    bool initialize(int mode);
    bool connect();
    void close();
    bool sendRequest(const char* param,
                     unsigned int param_size,
                     char* reply, int reply_max);

  public:
    bool bootShapers(int sh_num, int adrr = 0xA7000000);
    bool initDSP(int sh_num, int adrr = 0xA8000000);
    bool status(int status_type = 0);

  private:
    std::string m_hostname;
    TCPSocket m_socket;
    std::vector<Shaper> m_shaper_v;

  };

}

#endif
