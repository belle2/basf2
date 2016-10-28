/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FTList_H
#define FTList_H

#include <cstdlib>
#include <framework/logging/Logger.h>

#define FTLIST_DEBUG

namespace Belle2 {

  //! class template of list for the Level-3 Fast Track / Cluster Finder
  template <class T>
  class FTList {
  public:

    //! default constructor
    explicit FTList(const int lengthToAlloc = 100)
      : m_length(0),
        m_remain(lengthToAlloc),
        m_lengthToAlloc(lengthToAlloc),
        m_obj(static_cast<T*>(std::malloc(lengthToAlloc * sizeof(T))))
    {
      if (!m_obj) B2FATAL("FTList::m_obj malloc failed");
    }

    //! copy constructor
    FTList(const FTList<T>& src)
      : m_length(src.m_length),
        m_remain(src.m_remain),
        m_lengthToAlloc(src.m_lengthToAlloc)
    {
      m_obj = static_cast<T*>(std::malloc((m_length + m_remain) * sizeof(T)));
      if (!m_obj) B2FATAL("FTList::m_obj malloc failed");
      T* srcObj = src.m_obj;
      for (int i = 0; i < m_length; i++) {
        *(m_obj + i) = *(srcObj + i);
      }
    }

    //! destructor
    ~FTList()
    {
      free(m_obj);
    }

    //! append an object into the end of the list
    int append(const T x);

    //! append objects into the end of the list
    int append(const FTList<T>&);

    //! remove objects by index and returns decremented index and length
    int remove(int&);

    //! remove objects by index
    void remove2(const int);

    //! replace index-th object by the object src
    void replace(const int i, const T src);

    //! delete objects by index and returns decremented index and length
    int deleteObj(int&);

    //! remove just last objects of the list
    void removeLast(void);

    //! clear lists but the allocated memory remains same
    void clear(void);

    //! clear lists and free memory
    void removeAll(void);

    //! delete all object and clear(allocated memory remains same)
    void deleteAll(void);

    //! re-allocate memory to reduce size
    void resize(void);

    //! returns a object by index
    T operator[](const int i) const;

    //! returns the reference of a object by index
    T& operator()(const int i) const;

    //! returns the first object in the list
    T first(void) const;

    //! returns the pointer of first object
    T* firstPtr(void) const;

    //! returns the pointer of last object
    T* lastPtr(void) const;

    //! returns the length of the list
    int length(void) const;

  private:
    //! length of the list
    int m_length;

    //! allocated length - m_length
    int m_remain;

    //! length to allocate
    int m_lengthToAlloc;

    //! array of the object
    T* m_obj;
  };

  template <class T>
  inline
  int
  FTList<T>::append(const T src)
  {
    if (!m_remain) {
      m_obj = static_cast<T*>(std::realloc(m_obj, (m_length + m_lengthToAlloc) * sizeof(T)));
      if (!m_obj) B2FATAL("FTList::m_obj realloc failed");
      m_remain = m_lengthToAlloc;
    }
    *(m_obj + (m_length++)) = src;
    m_remain--;
    return m_length;
  }

  template <class T>
  inline
  int
  FTList<T>::remove(int& i)
  {
#ifdef FTLIST_DEBUG
    if (i < 0 || i > m_length) B2FATAL("FTList overrun!!");
#endif
    *(m_obj + (i--)) = *(m_obj + (--m_length));
    m_remain++;
    return m_length;
  }

  template <class T>
  inline
  void
  FTList<T>::remove2(const int i)
  {
#ifdef FTLIST_DEBUG
    if (i < 0 || i >= m_length) B2FATAL("FTList overrun!!");
#endif
    *(m_obj + i) = *(m_obj + (--m_length));
    m_remain++;
  }

  template <class T>
  inline
  void
  FTList<T>::replace(const int i, const T src)
  {
#ifdef FTLIST_DEBUG
    if (i < 0 || i >= m_length) B2FATAL("FTList overrun!!");
#endif
    *(m_obj + i) = src;
  }

  template <class T>
  inline
  int
  FTList<T>::deleteObj(int& i)
  {
#ifdef FTLIST_DEBUG
    if (i < 0 || i >= m_length) B2FATAL("FTList overrun!!");
#endif
    delete *(m_obj + i);
    *(m_obj + (i--)) = *(m_obj + (--m_length));
    m_remain++;
    return m_length;
  }

  template <class T>
  inline
  void
  FTList<T>::removeLast(void)
  {
    if (m_length) {
      m_length--;
      m_remain++;
    }
  }

  template <class T>
  inline
  void
  FTList<T>::clear(void)
  {
    m_remain += m_length;
    m_length = 0;
  }

  template <class T>
  inline
  void
  FTList<T>::removeAll(void)
  {
    free(m_obj);
    m_remain = 0;
    m_length = 0;
    m_obj = NULL;
  }


  template <class T>
  inline
  void
  FTList<T>::resize(void)
  {
    m_obj = static_cast<T*>(std::realloc(m_obj, m_length * sizeof(T)));
    if (!m_obj) B2FATAL("FTList::m_obj realloc failed");
    m_remain = 0;
    m_lengthToAlloc = m_length;
  }

  template <class T>
  inline
  T
  FTList<T>::operator[](const int i) const
  {
#ifdef FTLIST_DEBUG
    if (i < 0 || i >= m_length) B2FATAL("FTList overrun!!");
#endif
    return *(m_obj + i);
  }

  template <class T>
  inline
  T&
  FTList<T>::operator()(const int i) const
  {
#ifdef FTLIST_DEBUG
    if (i < 0 || i >= m_length) B2FATAL("FTList overrun!!");
#endif
    return *(m_obj + i);
  }

  template <class T>
  inline
  T
  FTList<T>::first(void) const
  {
    return *m_obj;
  }

  template <class T>
  inline
  T*
  FTList<T>::firstPtr(void) const
  {
    return m_obj;
  }

  template <class T>
  inline
  T*
  FTList<T>::lastPtr(void) const
  {
    if (!m_length) return NULL;
    return m_obj + (m_length - 1);
  }

  template <class T>
  inline
  int
  FTList<T>::length(void) const
  {
    return m_length;
  }

  template <class T>
  int FTList<T>::append(const FTList<T>& src)
  {
    int srcLength = src.m_length;
    T* srcObj = src.m_obj;
    int i = 0;
    if (m_remain < srcLength) {
      m_obj = static_cast<T*>(std::realloc(m_obj, (m_length + m_remain + srcLength) * sizeof(T)));
      if (!m_obj) B2FATAL("FTList::m_obj realloc failed");
      while (i ^ srcLength) *(m_obj + (m_length++)) = *(srcObj + (i++));
    } else {
      while (i ^ srcLength) *(m_obj + (m_length++)) = *(srcObj + (i++));
      m_remain -= srcLength;
    }
    return m_length;
  }

  template <class T>
  void FTList<T>::deleteAll(void)
  {
    int i = m_length;
    while (i) delete *(m_obj + (--i));
    clear();
  }

}

#endif /* FTList_FLAG_ */
