#ifndef _Belle2_Buffer_hh
#define _Belle2_Buffer_hh

#include <string>

namespace Belle2 {

  class Buffer {

    // constructors & destructors
  public:
    Buffer() throw();
    Buffer(unsigned int size, unsigned char* memory = NULL) throw();
    Buffer(const Buffer& w) throw();
    virtual ~Buffer() throw();

    // member methods implemented
  public:
    unsigned char* ptr() throw() { return m_memory; }
    const unsigned char* ptr() const throw() { return m_memory; }
    unsigned int size() const throw() { return m_size; }
    virtual bool available() throw() { return m_memory != NULL; }

    // operators
  public:
    const Buffer& operator = (const Buffer&) throw();

    // member data
  protected:
    unsigned char* m_memory;
    unsigned int m_size;
    bool m_allocated;

  };

}

#endif
