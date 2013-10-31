#ifndef _Belle2_Host_hh
#define _Belle2_Host_hh

#include "Module.h"

#include <string>

namespace Belle2 {

  class Host : public Module {

  public:
    Host() {};
    virtual ~Host() throw() {};

  public:
    const std::string& getName() const { return _name; }
    const std::string& getType() const { return _type; }
    //const std::string& getMacAddress() const { return _mac_address; }
    //const std::string& getHardware() const { return _hardware; }
    //const std::string& getBootServer() const { return _boot_server; }
    //const std::string& getBootImage() const { return _boot_image; }
    void setName(const std::string& name) { _name = name; }
    void setType(const std::string& type) { _type = type; }
    //void setMacAddress(const std::string& address) { _mac_address = address; }
    //void setHardware(const std::string& hardware) { _hardware = hardware; }
    //void setBootServer(const std::string& server) { _boot_server = server; }
    //void setBootImage(const std::string& image) { _boot_image = image; }
    virtual const std::string getSQLFields() const throw();
    virtual const std::string getSQLLabels() const throw();
    virtual const std::string getSQLValues() const throw();

  private:
    std::string _name;
    std::string _type;
    //std::string _mac_address;
    //std::string _hardware;
    //std::string _boot_server;
    //std::string _boot_image;

  };

}

#endif
