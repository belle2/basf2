/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HLTBUFFER_H
#define HLTBUFFER_H

#include <string>
#include <vector>

namespace Belle2 {

  //! HLTBuffer class
  /*! This class is a buffer for data communication in HLT
   *  This buffer is supposed to work as FIFO
   */
  class HLTBuffer {
  public:
    //! Constructor
    HLTBuffer(void);
    //! Destructor
    ~HLTBuffer(void);

    //! Put data inside the buffer
    void put(const std::string data);
    //! Get data from the buffer
    std::string get(void);

    //! Get size of elements in the buffer
    int size(void);

  private:
    std::vector<std::string> m_buffer;    /*!< Actual buffer to contain things */
  };
}

#endif
