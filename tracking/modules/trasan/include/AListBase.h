// -*- C++ -*-
// CLASSDOC OFF
// $Id: AListBase.h 10030 2007-03-09 07:51:44Z katayama $
// CLASSDOC ON
//
// This file is a part of what might become CLHEP -
// a Class Library for High Energy Physics.
//
// This is the definition of the HepAListBase class which is the base class
// used in the template classes HepAList and HepConstAList.
//
// .SS See Also
//
// AList.h, AIterator.h, ConstAList.h, ConstAIterator.h, AIteratorBase.h
//
// .SS History
// HepAListBase was developed from an original (non-template) list class
// written by Dag Bruck.
//
// Author: Leif Lonnblad
//

#ifndef BELLE_ALISTBASE_H_
#define BELLE_ALISTBASE_H_

#ifdef GNUPRAGMA
#pragma interface
#endif

//iw
#ifndef BELLE_CLHEP_CLHEP_H
//iw #include "belleCLHEP/config/CLHEP.h"
#include "tracking/modules/trasan/CLHEP.h"
#endif

#ifdef HEP_NO_INLINE_IN_DECLARATION
#define inline
#endif

namespace Belle {

  extern "C" {
    typedef int (*AListSortCompareFunc)(const void*, const void*);
  }

  class  HepAListBase {

    friend class HepAListIteratorBase;

  protected:

    inline HepAListBase();
    // Constructs a list with no objects.

    HepAListBase(const HepAListBase&);
    // Copy constructor

    ~HepAListBase();
    // Destroys the list. The objects in the list are not destroyed.
    // Use HepAListDeleteAll(HepAList<T> &) to destroy all objects in the list.

    inline void insert(void*);
    // Inserts an object first in the list.

    inline void insert(void* e, void* r);
    // Inserts the object e just before the first occurence of
    // object r in the list.

    inline void insert(void* e, unsigned pos);
    // Inserts the object e at the position pos in the list. If pos is outside
    // the list, the object will be appended.

    inline void append(void*);
    // Appends an object in the end of the list

    void append(void*, void*);
    // Appends the object e just after the last occurrence of object r
    // in the list

    void append(const HepAListBase&);
    // Appends all objects of list l to the end of this list.

    void remove(void*);
    // Remove all occurencies of the object from the list.

    void remove(const HepAListBase& l);
    // Remove all occurencies of the objects in list l from this list.

    inline HepBoolean hasMember(void*) const;
    // Returns true if the object is a member of the list.

    void replace(void*, void*);
    // Replace all occurencies of object eo with object en.

    inline void* operator[](unsigned) const;
    // return a pointer to the object in position i (the first element has i=0).

    int index(void*) const;
    // Returns the index of the last occurrence of the object.
    // NOTE! the objects are numbered 0 to n-1.

    int fIndex(void*) const;
    // Returns the index of the first occurence of the object.
    // NOTE! the objects are numbered 0 to n-1.

    inline void* first() const;
    inline void* last() const;
    // Returns a pointer to the first and last object in the list.

    void operator = (const HepAListBase&);
    // Assignment.

  public:

    inline void remove(unsigned);
    // Remove an object from the list.

    inline void removeAll();
    // Remove all objects from the list.

    void purge();
    // Remove all duplicate objects in the list.

    void reverse();
    // Reverse the order in the list.

    void swap(unsigned i1, unsigned i2);
    // Swap the position of objects number i1 and i2.

    inline int length() const;
    // Returns the number of objects in the list

    inline HepBoolean empty() const;
    inline HepBoolean isEmpty() const;
    // Returns true if the list is empty.

    void realloc();
    // reallocates the array of pointers in the list

    void sort(AListSortCompareFunc compfunc);
    // sort the list. compfunc is a function used by standard C qsort routine.

  protected:

    void** p;
    // Array of pointers to actual member objects.

    int n;
    // Number of objects in the list.

    void copyArray(register void** dst, register void** src, register int n);
    // Internal function for fast copying of arrays.

    void*& newInsert(register int);
    // Allocate space for a new object before entry number ir

    void*& newAppend(register int);
    // Allocate space for a new object after entry number ir

    void removeEntry(int);
    // Remove one entry in the list.

  };

#ifdef HEP_NO_INLINE_IN_DECLARATION
#undef inline
#endif

#ifndef HEP_DEBUG_INLINE
//iw #include "belleCLHEP/Alist/AListBase.icc"
//#include "tracking/modules/trasan/AListBase.icc"

// -*- C++ -*-
// $Id: AListBase.icc 7749 2002-02-21 23:51:20Z katayama $
//
// This file is a part of what might become the CLHEP -
// a Class Library for High Energy Physics.
//
// This is the definitions of the inline member functions of the
// HepAListBase class
//

#ifdef HEP_DEBUG_INLINE
#define inline
#endif

  inline HepAListBase::HepAListBase()
    : p(0), n(0) {}

  inline void HepAListBase::insert(void* e)
  {
    if (e) newInsert(0) = e;
  }

  inline void HepAListBase::insert(void* e, void* r)
  {
    if (e) newInsert(fIndex(r)) = e;
  }

  inline void HepAListBase::append(void* e)
  {
    if (e) newAppend(n) = e;
  }

  inline void HepAListBase::insert(void* e, unsigned pos)
  {
    if (e && (int)pos < n) newInsert(pos) = e;
    else append(e);
  }

  inline HepBoolean HepAListBase::hasMember(void* e) const
  {
    return HepBoolean(index(e) >= 0);
  }

  inline void* HepAListBase::operator[](unsigned i) const
  {
    return ((int)i < n) ? p[i] : 0;
  }

  inline void* HepAListBase::first() const
  {
    return n ? p[0] : 0;
  }

  inline void* HepAListBase::last() const
  {
    return n ? p[n - 1] : 0;
  }

  inline void HepAListBase::removeAll()
  {
    n = 0;
  }

  inline void HepAListBase::remove(unsigned i)
  {
    if ((int)i < n) removeEntry(i);
  }

  inline int HepAListBase::length() const
  {
    return n;
  }

  inline HepBoolean HepAListBase::empty() const
  {
    return HepBoolean(n == 0);
  }

  inline HepBoolean HepAListBase::isEmpty() const
  {
    return HepBoolean(n == 0);
  }

#ifdef HEP_DEBUG_INLINE
#undef inline
#endif

#endif

} // namespace Belle

#endif
