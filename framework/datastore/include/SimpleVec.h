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

    /*! Constructor with STL vector for saving.*/
    /*! \par Vector, that will be saved internally.
    */
    SimpleVec(std::vector<T> AVector) {
      m_vector = AVector;
    }

    /*  Empty constructor.*/
    /*! This constructor is needed for I/O purposes.
    */
    SimpleVec()
        : m_vector(0) {}

    /*! Destructor.*/
    /*! As no pointers are used, there is nothing special done here.
    */
    ~SimpleVec() {}

    /*! Assign a vector to the interally saved vector.*/
    /*! \par Vector to be saved. Keep in mind the initial template instantiation.
    */
    void setVector(std::vector<T> AVector) {
      m_vector = AVector;
    }

    /*! Getter for the vector.*/
    /* \return Vector, that was internally saved. */
    std::vector<T> getVector() {
      return m_vector;
    }

  private:
    /*! Saved vector.*/
    /*! This variable actually holds the saved vector.*/
    std::vector<T> m_vector;

    /*! Needed for ROOT purposes.*/
    /*! This Macro makes a ROOT object from SimpleVec.
    */
    ClassDef(SimpleVec, 1);
  }; //class
} // namespace Belle2
#endif // SIMPLEVEC
