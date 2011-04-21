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

  template <class T>
  class SingletonDestroyerT {
  public:

    SingletonDestroyerT(T* doomed = 0) {
      m_doomed = doomed;
    }


    ~SingletonDestroyerT() {
      if (m_doomed != NULL) delete m_doomed;
    }

    void setDoomed(T* doomed) {
      m_doomed = doomed;
    }

  private:
    /** Prevent users from making copies. */
    SingletonDestroyerT(const SingletonDestroyerT&);

    /** Prevent users from making copies. */
    void operator = (const SingletonDestroyerT&);

    T* m_doomed;

  };
}

#endif /* SINGLETONDESTROYERT_H_ */
