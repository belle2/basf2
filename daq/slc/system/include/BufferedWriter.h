/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_BufferedWriter_hh
#define _Belle2_BufferedWriter_hh

#include "daq/slc/base/Writer.h"

namespace Belle2 {

  class BufferedWriter : public Writer {

    // constructors & destructors
  public:
    BufferedWriter();
    BufferedWriter(size_t size, unsigned char* memory = NULL);
    BufferedWriter(const BufferedWriter& w);
    virtual ~BufferedWriter();

    // member methods implemented
  public:
    void seekTo(size_t pos) { m_pos = pos; }
    unsigned char* ptr() { return m_memory; }
    size_t count() const { return m_pos; }
    size_t size() const { return m_size; }
    virtual size_t write(const void*, size_t);
    virtual bool available() { return true; }

    // operators
  public:
    const BufferedWriter& operator = (const BufferedWriter&);

    // member data
  protected:
    unsigned char* m_memory;
    size_t m_size;
    size_t m_pos;
    bool m_allocated;

  };

}

#endif
