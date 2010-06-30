/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef SIMPLEVEC_H
#define SIMPLEVEC_H

#include <TObject.h>

namespace Belle2 {

  /*! Use this class to store primitive types like ints in the DataStore
      \author <a href="mailto:martin.heck@kit.edu?subject=SimpleVec">Martin Heck</a>
  */
  template <class T>
  class SimpleVec : public TObject {
  public:

    SimpleVec(std::vector<T> AVector) {
      m_vector = AVector;
    }

    SimpleVec()
        : m_vector(0) {}

    ~SimpleVec() {}

    void setVector(std::vector<T> AVector) {
      m_vector = AVector;
    }

    std::vector<T> getVector() {
      return m_vector;
    }

  private:
    std::vector<T> m_vector;

    ClassDef(SimpleVec, 1);
  }; //class
} // namespace Belle2
#endif // SIMPLEVEC
