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

#include <vector>

namespace Belle2 {
  /** Possibility to store primitive types like ints into the DataStore; not recommended for use in official modules.
   *
   *  Use this class to store primitive types like integers in the DataStore in a simple way.
   *  However, official modules should properly encapsulate data in their own objects, so this
   *  object is meant for private modules/analysis and so on. <br>
   *  This object is still quite preliminary. Please improve by making it more convenient.
   *
   *  @author <a href="mailto:martin.heck@kit.edu?subject=SimpleVec">Martin Heck</a>
   */
  template <class T>
  class SimpleVec : public TObject {
  public:

    /** Constructor with STL vector for saving.
     *
     *  @par AVector, that will be saved internally.
     */
    SimpleVec(const std::vector<T>& AVector) : m_vector(AVector) { }

    /**  Empty constructor.
     *
     *  This constructor is needed for I/O purposes.
     */
    SimpleVec()
      : m_vector(0) {}

    /** Destructor.
     *
     *  As no pointers are used, there is nothing special done here.
     */
    ~SimpleVec() {}

    /** Assign a vector to the interally saved vector.
     *
     *  @par AVector Vector to be saved. Keep in mind the initial template instantiation.
     */
    void setVector(const std::vector<T>& AVector) {
      m_vector = AVector;
    }

    /** Getter for the vector.
     *
     *  @return Vector, that was internally saved.
     */
    const std::vector<T>& getVector() const {
      return m_vector;
    }

  private:
    /** Saved vector.  */
    std::vector<T> m_vector;

    /** Needed for ROOT purposes. This Macro makes a ROOT object from SimpleVec. */
    ClassDef(SimpleVec, 1);
  }; //class
} // namespace Belle2
#endif // SIMPLEVEC
