// -*- C++ -*-
// CLASSDOC OFF
// $Id: CList.h 10030 2007-03-09 07:51:44Z katayama $
// CLASSDOC ON
//
// This file is a part of what might become CLHEP -
// a Class Library for High Energy Physics.
//
// This is the definition of the HepCList class.
//
// HepCList is a template based list class for storing copies of
// objects.  A HepCList can be made of any class where a copy
// constructor and a operator== is defined.
//
//
// .SS Usage // A list of eg. integers is declared as
//
// .ft B
//      HepCList<int> intlist;
//
//
// .SS See Also
//
// AList.h AIterator.h, ConstAList.h, ConstAIterator.h, AListBase.h,
// AIteratorBase.h
//
// Author: Leif Lonnblad
//

#ifndef BELLE_CLIST_H_
#define BELLE_CLIST_H_

//iw
#ifndef BELLE_CLHEP_CLHEP_H
//iw #include "belleCLHEP/config/CLHEP.h"
#include "tracking/modules/trasan/CLHEP.h"
#endif
//iw #include "belleCLHEP/Alist/AListBase.h"
#include "tracking/modules/trasan/AListBase.h"
//iw #include "tracking/modules/trasan/AList.h"
#include "tracking/modules/trasan/AList.h"
//iw #include "tracking/modules/trasan/ConstAList.h"
#include "tracking/modules/trasan/ConstAList.h"

#ifdef HEP_NO_INLINE_IN_TEMPLATE_DECLARATION
#define inline
#endif

namespace Belle {

template <class T>
class  HepCList : public HepAListBase {

public:

    /// Constructs a list with no objects.
    inline HepCList();

    /// Copy constructor
    inline HepCList(const HepCList<T> &);

    /// Constructor with AList
    inline HepCList(const HepAList<T> &);

    /// Constructor with CAList
    inline HepCList(const HepConstAList<T> &);


    inline ~HepCList();
    // Destroys the list. All objects are destroyed.

    inline void insert(const T&);
    // Inserts an object first in the list.

    inline void insert(const T& e, const T& r);
    // Inserts the object e just before the first occurence of
    // object r in the list.

    inline void insert(const T& e, unsigned pos);
    // Inserts the object e at the position pos in the list. If pos is outside
    // the list, the object will be appended.

    inline void append(const T&);
    inline void operator += (const T&);
    // Appends an object in the end of the list

    inline void append(const T& e, const T& r);
    // Appends the object e just after the last occurrence of object r
    // in the list

    inline void append(const HepCList<T> &);
    inline void operator += (const HepCList<T> &);
    inline void append(const HepAList<T> &);
    inline void operator += (const HepAList<T> &);
    inline void append(const HepConstAList<T> &);
    inline void operator += (const HepConstAList<T> &);
    // Appends copies of all objects of list l to the end of this list.

    inline void remove(const T&);
    // Remove and delete all occurencies equal to  the object from the list.

    inline void remove(const HepCList<T> &);
    inline void remove(const HepConstAList<T> &);
    inline void remove(const HepAList<T> &);
    // Remove and delete all occurencies equal to the objects in list l
    // from this list.

    inline void remove(unsigned);
    // Remove and delete an object in the list.

    inline void removeAll();
    // Remove and delete all objects in the list.

    inline void purge();
    // Remove and delete all duplicate objects in the list.

    inline HepBoolean member(const T&) const;
    inline HepBoolean hasMember(const T&) const;
    // Returns true if the object is equal to a member of the list.

    inline void replace(const T& eo, const T& en);
    // Replace all occurencies of object eo with object en.

    inline T* operator[](unsigned i) const;
    // Returns the i:th object in the list.
    // NOTE! the objects are numbered 0 to n-1.

    inline int index(const T&) const;
    // Returns the index of the last occurrence equal to the object.
    // NOTE! the objects are numbered 0 to n-1.

    inline int fIndex(const T&) const;
    // Returns the index of the first occurence of the object.
    // NOTE! the objects are numbered 0 to n-1.

    inline T* first() const;
    inline T* last() const;
    // Returns a pointer to the first and last object in the list.

    inline void operator = (const HepCList<T> &);
    inline void operator = (const HepAList<T> &);
    inline void operator = (const HepConstAList<T> &);
    // Assignment.

  protected:

    inline T* Nth(unsigned i) const;
    // Returns the i:th object in the list.

  };

#ifdef HEP_NO_INLINE_IN_TEMPLATE_DECLARATION
#undef inline
#endif

#ifdef HEP_SHORT_NAMES
#define CList HepCList
#endif

//iw #include "belleCLHEP/Alist/CList.icc"
//#include "tracking/modules/trasan/CList.icc"
  template <class T>
  inline HepCList<T>::HepCList() {}

  template <class T>
  inline HepCList<T>::~HepCList()
  {
    removeAll();
  }

template <class T>
inline
HepCList<T>::HepCList(const HepCList<T> & l) : HepAListBase() {
    append(l);
}

  template <class T>
  inline HepCList<T>::HepCList(const HepAList<T> & l)
  {
    append(l);
  }

  template <class T>
  inline HepCList<T>::HepCList(const HepConstAList<T> & l)
  {
    append(l);
  }

  template <class T>
  inline void HepCList<T>::insert(const T& e)
  {
    HepAListBase::insert(new T(e));
  }

  template <class T>
  inline void HepCList<T>::insert(const T& e, const T& r)
  {
    HepAListBase::insert(new T(e), fIndex(r));
  }

  template <class T>
  inline void HepCList<T>::insert(const T& e, unsigned pos)
  {
    HepAListBase::insert(new T(e), pos);
  }

  template <class T>
  inline void HepCList<T>::append(const T& e)
  {
    HepAListBase::append(new T(e));
  }

  template <class T>
  inline void HepCList<T>::operator += (const T& e)
  {
    HepAListBase::append(new T(e));
  }

  template <class T>
  inline void HepCList<T>::append(const T& e, const T& r)
  {
    HepAListBase::insert(new T(e), index(r) + 1);
  }

  template <class T>
  inline void HepCList<T>::append(const HepCList<T> & l)
  {
    unsigned oldn = n;
    unsigned ln = l.length();
    n += ln;
    realloc();
    for (unsigned i = 0; i < ln; i++) p[oldn + i] = new T(*(l[i]));
  }

  template <class T>
  inline void HepCList<T>::append(const HepAList<T> & l)
  {
    unsigned oldn = n;
    unsigned ln = l.length();
    n += ln;
    realloc();
    for (unsigned i = 0; i < ln; i++) p[oldn + i] = new T(*(l[i]));
  }

  template <class T>
  inline void HepCList<T>::append(const HepConstAList<T> & l)
  {
    unsigned oldn = n;
    unsigned ln = l.length();
    n += ln;
    realloc();
    for (unsigned i = 0; i < ln; i++) p[oldn + i] = new T(*(l[i]));
  }

  template <class T>
  inline void HepCList<T>::operator += (const HepCList<T> & l)
  {
    append(l);
  }

  template <class T>
  inline void HepCList<T>::operator += (const HepAList<T> & l)
  {
    append(l);
  }

  template <class T>
  inline void HepCList<T>::operator += (const HepConstAList<T> & l)
  {
    append(l);
  }

  template <class T>
  inline void HepCList<T>::remove(const T& e)
  {
    int i;
    while ((i = index(e)) >= 0) HepAListBase::remove(i);
  }

  template <class T>
  inline void HepCList<T>::remove(unsigned i)
  {
    T* e = Nth(i);
    HepAListBase::remove(i);
    delete e;
  }

  template <class T>
  inline void HepCList<T>::remove(const HepCList<T> & l)
  {
    register int i = l.n;
    while (--i >= 0) remove(*(l[i]));
  }

  template <class T>
  inline void HepCList<T>::remove(const HepAList<T> & l)
  {
    register int i = l.length();
    while (--i >= 0) remove(*(l[i]));
  }

  template <class T>
  inline void HepCList<T>::remove(const HepConstAList<T> & l)
  {
    register int i = l.length();
    while (--i >= 0) remove(*(l[i]));
  }

  template <class T>
  inline HepBoolean HepCList<T>::member(const T& e) const
  {
    return HepBoolean(index(e) >= 0);
  }

  template <class T>
  inline HepBoolean HepCList<T>::hasMember(const T& e) const
  {
    return HepBoolean(index(e) >= 0);
  }

  template <class T>
  inline void HepCList<T>::replace(const T& eo, const T& en)
  {
    if (eo != en) {
      int i;
      while ((i = index(eo)) >= 0) {
        delete(T*) p[i];
        p[i] = new T(en);
      }
    }
  }

  template <class T>
  inline T* HepCList<T>::operator[](unsigned i) const
  {
    return Nth(i);
  }

  template <class T>
  inline int HepCList<T>::index(const T& e) const
  {
    register unsigned i = n;
    while (i--) if (*(Nth(i)) == e) return i;
    return -1;
  }

  template <class T>
  inline int HepCList<T>::fIndex(const T& e) const
  {
    register int i = -1;
    while (++i < int(n)) if (*(Nth(i)) == e) return i;
    return -1;
  }

  template <class T>
  inline T* HepCList<T>::first() const
  {
    return (T*) HepAListBase::first();
  }

  template <class T>
  inline T* HepCList<T>::last() const
  {
    return (T*) HepAListBase::last();
  }

  template <class T>
  inline void HepCList<T>::operator = (const HepCList<T> & l)
  {
    removeAll();
    append(l);
  }

  template <class T>
  inline void HepCList<T>::operator = (const HepAList<T> & l)
  {
    removeAll();
    append(l);
  }

  template <class T>
  inline void HepCList<T>::operator = (const HepConstAList<T> & l)
  {
    removeAll();
    append(l);
  }

  template <class T>
  inline void HepCList<T>::removeAll()
  {
    T* e;
    while (n--) {
      e = (T*) p[n];
      delete e;
    }
  }

  template <class T>
  inline T* HepCList<T>::Nth(unsigned i) const
  {
    if ((int)i < n) return (T*) p[i];
    else return 0;
  }

  template <class T>
  inline void HepCList<T>::purge()
  {
    int ie;
    for (int i = 0; i < int(n); i++) {
      while ((ie = index(*(Nth(i)))) != i) HepCList<T>::remove((unsigned)ie);
    }
  }


} // namespace Belle

#endif

