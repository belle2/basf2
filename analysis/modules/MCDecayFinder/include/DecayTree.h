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
    /** Constructor with decaying object (e.g. MCParticle) provided. */
    explicit DecayTree(T* obj);
    /** Destructor. */
    ~DecayTree();
    /** Return list of decay daughters. */
    std::vector< DecayTree<T>* > getDaughters();
    /** Return the decaying object itself, e.g. an MCParticle. */
    T* getObj();
    /** Set the decaying object, e.g. an MCParticle.*/
    void setObj(T* obj);
    /** Frees memory occupied by m_myObject and the daughter objects. */
    void clear();
    /** Add a daughter, which is a DecayTree itself. */
    void append(DecayTree<T>* t);
  };

  template <class T>
  DecayTree<T>::DecayTree() : m_myObject(0), m_daughters() {};

  template <class T>
  DecayTree<T>::DecayTree(T* obj) : m_myObject(obj), m_daughters() {};

  template <class T>
  DecayTree<T>::~DecayTree()
  {
    m_myObject = 0;
    while (!m_daughters.empty()) {
      delete m_daughters.back();
      m_daughters.pop_back();
    }
  };

  template <class T>
  std::vector< DecayTree<T>* > DecayTree<T>::getDaughters()
  {
    return m_daughters;
  }

  template <class T>
  T* DecayTree<T>::getObj()
  {
    return m_myObject;
  }

  template <class T>
  void DecayTree<T>::setObj(T* obj)
  {
    m_myObject = obj;
  }

  template <class T>
  void DecayTree<T>::clear()
  {
    delete m_myObject;
    m_myObject = 0;
    while (!m_daughters.empty()) {
      m_daughters.back().clear();
      m_daughters.pop_back();
    }
  }

  template <class T>
  void DecayTree<T>::append(DecayTree<T>* t)
  {
    m_daughters.push_back(t);
  }
} // namespace Belle2
