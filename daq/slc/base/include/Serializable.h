/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_Serializable_hh
#define _Belle2_Serializable_hh

namespace Belle2 {

  class Reader;
  class Writer;

  class Serializable {

  public:
    virtual ~Serializable() {}

  public:
    virtual void readObject(Reader&) = 0;
    virtual void writeObject(Writer&) const = 0;

  };

}

#endif
