#ifndef _B2DAQ_NSMData_hh
#define _B2DAQ_NSMData_hh

#include "NSMHandlerException.hh"

namespace B2DAQ {

  class NSMNode;

  class NSMData {

  public:
    NSMData(const std::string& data_name, const std::string& format,
	    int revision) throw() : _pdata(NULL), _data_name(data_name),
				    _format(format), _revision(revision) {}
    NSMData() throw() : _pdata(NULL) {}
    virtual ~NSMData() throw() {}

  public:
    bool isAvailable() throw() { return (_pdata != NULL); }
    void* open() throw(NSMHandlerException);
    void* allocate(int interval = 3) throw(NSMHandlerException);
    void* get() throw(NSMHandlerException);
    const void* get() const throw(NSMHandlerException);
    virtual void read(NSMNode* node) throw(NSMHandlerException) = 0;
    virtual void write(NSMNode* node) throw(NSMHandlerException) = 0;
    virtual void serialize(NSMNode* node, int& npar, int* pars, std::string& data) throw(IOException) {}
    virtual void deserialize(NSMNode* node, int npar, int* pars, const std::string& data) throw(IOException) {}

  private:
    void* _pdata;
    std::string _data_name;
    std::string _format;
    int _revision;

  };

}

#endif
