// -*- C++ -*-
// CLASSDOC OFF
// $Id: ConstAList.h 10030 2007-03-09 07:51:44Z katayama $
// CLASSDOC ON
//
// This file is a part of what might become CLHEP -
// a Class Library for High Energy Physics.
//
// This is the definition of the HepConstAList class. It is used in the same
// way as the HepAList class but the object are stored as const pointers.
// HepAListIterator cannot be used for this list, use HepConstAListIterator
// instead.
//
// .SS See Also
// AIterator.h, List.h, ConstAIterator.h, AListBase.h, AIteratorBase.h
//
// .SS History
// HepConstAList was developed from an original (non-template) list class
// written by Dag Bruck.
//
// Author: Leif Lonnblad
//

#ifndef BELLE_CONSTALIST_H_
#define BELLE_CONSTALIST_H_

//iw
#ifndef BELLE_CLHEP_CLHEP_H
//iw #include "belleCLHEP/config/CLHEP.h"
#include "tracking/modules/trasan/CLHEP.h"
#endif
//iw #include "tracking/modules/trasan/AList.h"
#include "tracking/modules/trasan/AList.h"

#ifdef HEP_NO_INLINE_IN_TEMPLATE_DECLARATION
#define inline
#endif

namespace Belle {

  template <class T>
  class  HepConstAList : public HepAListBase {

  public:

    inline HepConstAList();
    // Constructs a list with no objects.

    inline HepConstAList(const HepAList<T> &);
    // Copy constructor from non-const HepAList

    inline HepConstAList(const HepConstAList<T> &);
    // Copy constructor

    inline ~HepConstAList();
    // Destroys the list. The objects in the list are not destroyed.

    inline void insert(const T*);
    inline void insert(const T&);
    // Inserts an object first in the list.

    inline void insert(const T* e, const T* r);
    inline void insert(const T& e, const T& r);
    // Inserts the object e just before the first occurence of
    // object r in the list.

    inline void insert(const T* e, unsigned pos);
    inline void insert(const T& e, unsigned pos);
    // Inserts the object e at the position pos in the list. If pos is outside
    // the list, the object will be appended.

    inline void append(const T*);
    inline void append(const T& e);
    inline void operator += (const T*);
    inline void operator += (const T&);
    // Appends an object in the end of the list

    inline void append(const T* e, const T* r);
    inline void append(const T& e, const T& r);
    // Appends the object e just after the last occurrence of object r
    // in the list

    inline void append(const HepAList<T> & l);
    inline void append(const HepConstAList<T> & l);
    inline void operator += (const HepAList<T> & l);
    inline void operator += (const HepConstAList<T> & l);
    // Appends all objects of list l to the end of this list.

    inline void remove(const T*);
    inline void remove(const T&);
    // Remove all occurencies of the object from the list.

    inline void remove(unsigned);
    // Remove an object from the list.

    inline void remove(const HepAList<T> &);
    inline void remove(const HepConstAList<T> &);
    // Remove all occurencies of the objects in list l from this list.

    inline HepBoolean member(const T*) const;
    inline HepBoolean member(const T&) const;
    inline HepBoolean hasMember(const T*) const;
    inline HepBoolean hasMember(const T&) const;
    // Returns true if the object is a member of the list.

    inline void replace(const T* eo, const T* en);
    inline void replace(const T& eo, const T& en);
    // Replace all occurencies of object eo with object en.

    inline const T* operator[](unsigned i) const;
    // Returns the i:th object in the list.
    // NOTE! the objects are numbered 0 to n-1.

    inline int index(const T*) const;
    inline int index(const T&) const;
    // Returns the index of the last occurrence of the object.
    // NOTE! the objects are numbered 0 to n-1.

    inline int fIndex(const T*) const;
    inline int fIndex(const T&) const;
    // Returns the index of the first occurence of the object.
    // NOTE! the objects are numbered 0 to n-1.

    inline const T* first() const;
    inline const T* last() const;
    // Returns a pointer to the first and last object in the list.

    inline void operator = (const HepAList<T> &);
    inline void operator = (const HepConstAList<T> &);
    // Assignment.

#if defined(__GNUG__)
    inline void sort(int (*compfunc)(const T**, const T**));
#else
    inline void sort(AListSortCompareFunc);
#endif
    // sort the list according to the function.
    // the function is the same as one used by the C standard routine qsort.

  };

#ifdef HEP_NO_INLINE_IN_TEMPLATE_DECLARATION
#undef inline
#endif

#ifdef HEP_SHORT_NAMES
#define CAList HepConstAList
#endif

//iw #include "belleCLHEP/Alist/ConstAList.icc"
//#include "tracking/modules/trasan/ConstAList.icc"
// -*- C++ -*-
// $Id: ConstAList.icc 6497 2000-05-08 13:04:32Z katayama $
//
// This file is a part of what might become the CLHEP -
// a Class Library for High Energy Physics.
//
// This is the definitions of the inline member functions of the
// HepConstAList class
//

  template <class T>
  inline HepConstAList<T>::HepConstAList() {}

  template <class T>
  inline HepConstAList<T>::HepConstAList(const HepAList<T> & l)
    : HepAListBase(l) {}

  template <class T>
  inline HepConstAList<T>::HepConstAList(const HepConstAList<T> & l)
    : HepAListBase(l) {}

  template <class T>
  inline HepConstAList<T>::~HepConstAList() {}

  template <class T>
  inline void HepConstAList<T>::insert(const T* e)
  {
    HepAListBase::insert((void*) e);
  }

  template <class T>
  inline void HepConstAList<T>::insert(const T& e)
  {
    HepAListBase::insert((void*) &e);
  }

  template <class T>
  inline void HepConstAList<T>::insert(const T* e, const T* r)
  {
    HepAListBase::insert((void*) e, (void*) r);
  }

  template <class T>
  inline void HepConstAList<T>::insert(const T& e, const T& r)
  {
    HepAListBase::insert((void*) &e, (void*) &r);
  }

  template <class T>
  inline void HepConstAList<T>::insert(const T* e, unsigned pos)
  {
    HepAListBase::insert((void*) e, pos);
  }

  template <class T>
  inline void HepConstAList<T>::insert(const T& e, unsigned pos)
  {
    HepAListBase::insert((void*) &e, pos);
  }

  template <class T>
  inline void HepConstAList<T>::append(const T* e)
  {
    HepAListBase::append((void*) e);
  }

  template <class T>
  inline void HepConstAList<T>::append(const T& e)
  {
    HepAListBase::append((void*) &e);
  }

  template <class T>
  inline void HepConstAList<T>::operator += (const T* e)
  {
    HepAListBase::append((void*) e);
  }

  template <class T>
  inline void HepConstAList<T>::operator += (const T& e)
  {
    HepAListBase::append((void*) &e);
  }

  template <class T>
  inline void HepConstAList<T>::append(const T* e, const T* r)
  {
    HepAListBase::append((void*) e, (void*) r);
  }

  template <class T>
  inline void HepConstAList<T>::append(const T& e, const T& r)
  {
    HepAListBase::append((void*) &e, (void*) &r);
  }

  template <class T>
  inline void HepConstAList<T>::append(const HepAList<T> & l)
  {
    HepAListBase::append(l);
  }

  template <class T>
  inline void HepConstAList<T>::append(const HepConstAList<T> & l)
  {
    HepAListBase::append(l);
  }

  template <class T>
  inline void HepConstAList<T>::operator += (const HepAList<T> & l)
  {
    HepAListBase::append(l);
  }

  template <class T>
  inline void HepConstAList<T>::operator += (const HepConstAList<T> & l)
  {
    HepAListBase::append(l);
  }

  template <class T>
  inline void HepConstAList<T>::remove(const T* e)
  {
    HepAListBase::remove((void*) e);
  }

  template <class T>
  inline void HepConstAList<T>::remove(const T& e)
  {
    HepAListBase::remove((void*) &e);
  }

  template <class T>
  inline void HepConstAList<T>::remove(unsigned i)
  {
    HepAListBase::remove(i);
  }

  template <class T>
  inline void HepConstAList<T>::remove(const HepAList<T> & l)
  {
    HepAListBase::remove(l);
  }

  template <class T>
  inline void HepConstAList<T>::remove(const HepConstAList<T> & l)
  {
    HepAListBase::remove(l);
  }

  template <class T>
  inline HepBoolean HepConstAList<T>::member(const T* e) const
  {
    return HepAListBase::hasMember((void*) e);
  }

  template <class T>
  inline HepBoolean HepConstAList<T>::member(const T& e) const
  {
    return HepAListBase::hasMember((void*) &e);
  }

  template <class T>
  inline HepBoolean HepConstAList<T>::hasMember(const T* e) const
  {
    return HepAListBase::hasMember((void*) e);
  }

  template <class T>
  inline HepBoolean HepConstAList<T>::hasMember(const T& e) const
  {
    return HepAListBase::hasMember((void*) &e);
  }

  template <class T>
  inline void HepConstAList<T>::replace(const T* eo, const T* en)
  {
    HepAListBase::replace((void*) eo, (void*) en);
  }

  template <class T>
  inline void HepConstAList<T>::replace(const T& eo, const T& en)
  {
    HepAListBase::replace((void*) &eo, (void*) &en);
  }

  template <class T>
  inline const T* HepConstAList<T>::operator[](unsigned i) const
  {
    return (const T*) HepAListBase::operator[](i);
  }

  template <class T>
  inline int HepConstAList<T>::index(const T* e) const
  {
    return HepAListBase::index((void*) e);
  }

  template <class T>
  inline int HepConstAList<T>::index(const T& e) const
  {
    return HepAListBase::index((void*) &e);
  }

  template <class T>
  inline int HepConstAList<T>::fIndex(const T* e) const
  {
    return HepAListBase::fIndex((void*) e);
  }

  template <class T>
  inline int HepConstAList<T>::fIndex(const T& e) const
  {
    return HepAListBase::fIndex((void*) &e);
  }

  template <class T>
  inline const T* HepConstAList<T>::first() const
  {
    return (const T*) HepAListBase::first();
  }

  template <class T>
  inline const T* HepConstAList<T>::last() const
  {
    return (const T*) HepAListBase::last();
  }

  template <class T>
  inline void HepConstAList<T>::operator = (const HepAList<T> & l)
  {
    HepAListBase::operator=(l);
  }

  template <class T>
  inline void HepConstAList<T>::operator = (const HepConstAList<T> & l)
  {
    HepAListBase::operator=(l);
  }

#if defined(__GNUG__)
  template <class T>
  inline void HepConstAList<T>::sort(int (*compfunc)(const T**, const T**))
  {
    qsort(p, n, sizeof(void*), (int (*)(const void*, const void*))compfunc);
  }
#else
  template <class T>
  inline void HepConstAList<T>::sort(AListSortCompareFunc f)
  {
    qsort(p, n, sizeof(void*), f);
  }
#endif


} // namespace Belle

#endif
