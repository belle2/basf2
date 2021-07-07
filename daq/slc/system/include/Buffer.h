/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_Buffer_hh
#define _Belle2_Buffer_hh

#include <cstddef>

namespace Belle2 {

  class Buffer {

    // constructors & destructors
  public:
    Buffer();
    Buffer(unsigned int size, unsigned char* memory = NULL);
    Buffer(const Buffer& w);
    virtual ~Buffer();

    // member methods implemented
  public:
    unsigned char* ptr() { return m_memory; }
    const unsigned char* ptr() const { return m_memory; }
    unsigned int size() const { return m_size; }
    virtual bool available() { return m_memory != NULL; }

    // operators
  public:
    const Buffer& operator = (const Buffer&);

    // member data
  protected:
    unsigned char* m_memory;
    unsigned int m_size;
    bool m_allocated;

  };

}

#endif
