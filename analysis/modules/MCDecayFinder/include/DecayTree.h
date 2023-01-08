/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
#include <vector>

namespace Belle2 {

  /** This is a helper class for the MCDecayFinderModule. It is designed to
  hold MCParticle objects when trying to match a given DecayString. */
  template<class T>
  class DecayTree {
  private:
    /** The decaying object, e.g. MCParticle. */
    T* m_myObject;
    /** Decay daughters of m_myObject. */
    std::vector< DecayTree<T>* > m_daughters;
  public:
    /** Default constructor. */
    DecayTree();
    /** Destructor. */
    ~DecayTree();
    /** Return list of decay daughters. */
    std::vector< DecayTree<T>* > getDaughters() const;
    /** Return the decaying object itself, e.g. an MCParticle. */
    T* getObj() const;
    /** Set the decaying object, e.g. an MCParticle.*/
    void setObj(T* obj);
    /** Add a daughter, which is a DecayTree itself. */
    void append(DecayTree<T>* t);
  };

  /** Default constructor. */
  template <class T>
  DecayTree<T>::DecayTree() : m_myObject(0), m_daughters() {};

  /** Destructor. */
  template <class T>
  DecayTree<T>::~DecayTree()
  {
    m_myObject = 0;
    while (!m_daughters.empty()) {
      delete m_daughters.back();
      m_daughters.pop_back();
    }
  };

  /** Return a vector of daughters. */
  template <class T>
  std::vector< DecayTree<T>* > DecayTree<T>::getDaughters() const
  {
    return m_daughters;
  }

  /** Return the main object. */
  template <class T>
  T* DecayTree<T>::getObj() const
  {
    return m_myObject;
  }

  /** Set the main object. */
  template <class T>
  void DecayTree<T>::setObj(T* obj)
  {
    m_myObject = obj;
  }

  /** Append a daughter. */
  template <class T>
  void DecayTree<T>::append(DecayTree<T>* t)
  {
    m_daughters.push_back(t);
  }
} // namespace Belle2
