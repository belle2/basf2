/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SINGLETONDESTROYERT_H_
#define SINGLETONDESTROYERT_H_

namespace Belle2 {

  /** Class for managing the destruction of Singletons.
   *
   *  Make SingletonDestroyerT<YourSingleton> a friend class of your Singleton.
   *  Once you create a new object of your Singleton (typically inside your Instance function),
   *  hand over the management of the object to a static of the SingletonDestroyerT<YourSingleton> type.
   *  @sa Unit
   */
  template <class T>
  class SingletonDestroyerT {
  public:

    /** Constructor takes pointer to SingletonObject as argument.
     */
    SingletonDestroyerT(T* doomed = 0) {
      m_doomed = doomed;
    }

    /** Destructor destroys Singleton, if one is available.
     */
    ~SingletonDestroyerT() {
      delete m_doomed;
    }

    /** Reset the pointer to the Singleton, that shall be destroyed.
     *
     *  Usually not necessary, when the correct pointer is already given to the constructor.
     */
    void setDoomed(T* doomed) {
      m_doomed = doomed;
    }

  private:
    /** Prevent users from making copies.
     */
    SingletonDestroyerT(const SingletonDestroyerT&);

    /** Prevent users from making copies.
     */
    void operator = (const SingletonDestroyerT&);

    /** Pointer for managing the Singleton.
     */
    T* m_doomed;

  };
}

#endif /* SINGLETONDESTROYERT_H_ */
