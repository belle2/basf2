// -*- C++ -*-
// CLASSDOC OFF
// $Id: AList.h 10030 2007-03-09 07:51:44Z katayama $
// CLASSDOC ON
//
// This file is a part of what might become CLHEP -
// a Class Library for High Energy Physics.
//
// This is the definition of the HepAList class.
//
// HepAList is a template based list class for storing (pointers to) objects.
// An object can be a member of many lists at a time. Before an object is
// destructed, it must explicitly be taken out of any lists (otherwise there
// will be pointers left pointing to deallocated space). Objects does not
// have to be allocated on the free store, however if they are not, calling
// the function HepAListDeleteAll(HepAList<T> &) is not to recommended.
//
// .SS Usage
// A list of eg. integers is declared as
//
// .ft B
//      HepAList<int> intlist;
//
//
// .SS Note
// Only pointers to objects are stored in the list although there are methods
// to insert object references. Hence no object is copied in the
// process of handeling the list.
//
// .SS Performance
// The list is implemented as an array of pointers. Appending an object to the
// end of the list (append() and operator +=) is cheap. Removing objects
// (except for the last) and inserting at the beginning of the list is more
// expenive. HepAList is designed to be very space effective; The number of
// bytes needed for a list is (3 + number of objects) * sizeof (void*).
//
// .SS See Also
//
// AIterator.h, ConstAList.h, ConstAIterator.h, AListBase.h, AIteratorBase.h
//
// .SS History
// HepAList was developed from an original (non-template) list class written
// by Dag Bruck.
//
// Author: Leif Lonnblad
//
// modif. Luca Lista 30/1/1996
//  - Added AListSort and AListReverseSort utilities to sort the AList
//

#ifndef BELLE_ALIST_H_
#define BELLE_ALIST_H_

#include <cstdlib>

//iw
#ifndef BELLE_CLHEP_CLHEP_H
//iw #include "belleCLHEP/config/CLHEP.h"
#include "tracking/modules/trasan/CLHEP.h"
#endif
//iw #include "belleCLHEP/Alist/AListBase.h"
#include "tracking/modules/trasan/AListBase.h"

#ifdef HEP_NO_INLINE_IN_TEMPLATE_DECLARATION
#define inline
#endif

namespace Belle {

  template <class T>
  class  HepAList : public HepAListBase {

  public:

    inline HepAList();
    // Constructs a list with no objects.

    inline HepAList(const HepAList<T> &);
    // copy constructor

    inline ~HepAList();
    // Destroys the list. The objects in the list are not destroyed.
    // Use HepAListDeleteAll(HepAList<T> &) to destroy all objects in the list.

    inline void insert(T*);
    inline void insert(T&);
    // Inserts an object first in the list.

    inline void insert(T* e, T* r);
    inline void insert(T& e, T& r);
    // Inserts the object e just before the first occurence of
    // object r in the list.

    inline void insert(T* e, unsigned pos);
    inline void insert(T& e, unsigned pos);
    // Inserts the object e at the position pos in the list. If pos is outside
    // the list, the object will be appended.

    inline void append(T*);
    inline void append(T&);
    inline void operator += (T*);
    inline void operator += (T&);
    // Appends an object in the end of the list

    inline void append(T* e, T* r);
    inline void append(T& e, T& r);
    // Appends the object e just after the last occurrence of object r
    // in the list

    inline void append(const HepAList<T> &);
    inline void operator += (const HepAList<T> &);
    // Appends all objects of list l to the end of this list.

    inline void remove(T*);
    inline void remove(T&);
    // Remove all occurencies of the object from the list.

    inline void remove(const HepAList<T> &);
    // Remove all occurencies of the objects in list l from this list.

    inline void remove(unsigned);
    // Remove an object from the list.

//   void deleteAll();
    // Remove all elements in the list and delete the corresponding objects.
    // **Note** this member function is disabled due to problems with the gcc
    // compiler. Please use the global HepAListDeleteAl function instead.

    inline HepBoolean member(T*) const;
    inline HepBoolean member(T&) const;
    inline HepBoolean hasMember(T*) const;
    inline HepBoolean hasMember(T&) const;
    // Returns true if the object is a member of the list.

    inline void replace(T* eo, T* en);
    inline void replace(T& eo, T& en);
    // Replace all occurencies of object eo with object en.

    inline T* operator[](unsigned i) const;
    // Returns the i:th object in the list.
    // NOTE! the objects are numbered 0 to n-1.

    inline int index(T*) const;
    inline int index(T&) const;
    // Returns the index of the last occurrence of the object.
    // NOTE! the objects are numbered 0 to n-1.

    inline int fIndex(T*) const;
    inline int fIndex(T&) const;
    // Returns the index of the first occurence of the object.
    // NOTE! the objects are numbered 0 to n-1.

    inline T* first() const;
    inline T* last() const;
    // Returns a pointer to the first and last object in the list.

    inline void operator = (const HepAList<T> &);
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

  template <class T> inline void AListSort(HepAList<T> &);
  // Sort the list if the operator ">" is defined on T

  template <class T> inline void AListReverseSort(HepAList<T> &);
  // Reverse sort the list if the operator "<" is defined on T

  template <class T>
  inline void HepAListDeleteAll(HepAList<T> & l)
  {
// Deletes all elements in a list.
// NOTE! Use this function with care. Make sure that all elements in the
// list has been allocated with 'new'
    T* e;
    while ((e = l.last()) != 0) {
      l.remove(e);
      delete e;
    }
  }

#ifdef HEP_SHORT_NAMES
#define AList HepAList
#endif

//iw #include "belleCLHEP/Alist/AList.icc"
//#include "tracking/modules/trasan/AList.icc"
// -*- C++ -*-
// $Id: AList.icc 9932 2006-11-12 14:26:53Z katayama $
//
// This file is a part of what might become the CLHEP -
// a Class Library for High Energy Physics.
//
// This is the definitions of the inline member functions of the
// HepAList class
//

  template <class T>
  inline HepAList<T>::HepAList() {}

  template <class T>
  inline HepAList<T>::~HepAList() {}

  template <class T>
  inline HepAList<T>::HepAList(const HepAList<T> & l)
    : HepAListBase(l) {}

  template <class T>
  inline void HepAList<T>::insert(T* e)
  {
    HepAListBase::insert(e);
  }

  template <class T>
  inline void HepAList<T>::insert(T& e)
  {
    HepAListBase::insert(&e);
  }

  template <class T>
  inline void HepAList<T>::insert(T* e, T* r)
  {
    HepAListBase::insert(e, r);
  }

  template <class T>
  inline void HepAList<T>::insert(T& e, T& r)
  {
    HepAListBase::insert(&e, &r);
  }

  template <class T>
  inline void HepAList<T>::insert(T* e, unsigned pos)
  {
    HepAListBase::insert(e, pos);
  }

  template <class T>
  inline void HepAList<T>::insert(T& e, unsigned pos)
  {
    HepAListBase::insert(&e, pos);
  }

  template <class T>
  inline void HepAList<T>::append(T* e)
  {
    HepAListBase::append(e);
  }

  template <class T>
  inline void HepAList<T>::append(T& e)
  {
    HepAListBase::append(&e);
  }

  template <class T>
  inline void HepAList<T>::operator += (T* e)
  {
    HepAListBase::append(e);
  }

  template <class T>
  inline void HepAList<T>::operator += (T& e)
  {
    HepAListBase::append(&e);
  }

  template <class T>
  inline void HepAList<T>::append(T* e, T* r)
  {
    HepAListBase::append(e, r);
  }

  template <class T>
  inline void HepAList<T>::append(T& e, T& r)
  {
    HepAListBase::append(&e, &r);
  }

  template <class T>
  inline void HepAList<T>::append(const HepAList<T> & l)
  {
    HepAListBase::append(l);
  }

  template <class T>
  inline void HepAList<T>::operator += (const HepAList<T> & l)
  {
    HepAListBase::append(l);
  }

  template <class T>
  inline void HepAList<T>::remove(T* e)
  {
    HepAListBase::remove(e);
  }

  template <class T>
  inline void HepAList<T>::remove(T& e)
  {
    HepAListBase::remove(&e);
  }

  template <class T>
  inline void HepAList<T>::remove(unsigned i)
  {
    HepAListBase::remove(i);
  }

  template <class T>
  inline void HepAList<T>::remove(const HepAList<T> & l)
  {
    HepAListBase::remove(l);
  }

  template <class T>
  inline HepBoolean HepAList<T>::member(T* e) const
  {
    return HepAListBase::hasMember(e);
  }

  template <class T>
  inline HepBoolean HepAList<T>::member(T& e) const
  {
    return HepAListBase::hasMember(&e);
  }

  template <class T>
  inline HepBoolean HepAList<T>::hasMember(T* e) const
  {
    return HepAListBase::hasMember(e);
  }

  template <class T>
  inline HepBoolean HepAList<T>::hasMember(T& e) const
  {
    return HepAListBase::hasMember(&e);
  }

  template <class T>
  inline void HepAList<T>::replace(T* eo, T* en)
  {
    HepAListBase::replace(eo, en);
  }

  template <class T>
  inline void HepAList<T>::replace(T& eo, T& en)
  {
    HepAListBase::replace(&eo, &en);
  }

  template <class T>
  inline T* HepAList<T>::operator[](unsigned i) const
  {
    return (T*) HepAListBase::operator[](i);
  }

  template <class T>
  inline int HepAList<T>::index(T* e) const
  {
    return HepAListBase::index(e);
  }

  template <class T>
  inline int HepAList<T>::index(T& e) const
  {
    return HepAListBase::index(&e);
  }

  template <class T>
  inline int HepAList<T>::fIndex(T* e) const
  {
    return HepAListBase::fIndex(e);
  }

  template <class T>
  inline int HepAList<T>::fIndex(T& e) const
  {
    return HepAListBase::fIndex(&e);
  }

  template <class T>
  inline T* HepAList<T>::first() const
  {
    return (T*) HepAListBase::first();
  }

  template <class T>
  inline T* HepAList<T>::last() const
  {
    return (T*) HepAListBase::last();
  }

  template <class T>
  inline void HepAList<T>::operator = (const HepAList<T> & l)
  {
    HepAListBase::operator=(l);
  }

#if defined(__GNUG__)
  template <class T>
  inline void HepAList<T>::sort(int (*compfunc)(const T**, const T**))
  {
    std::qsort(p, n, sizeof(void*), (int (*)(const void*, const void*))compfunc);
  }
#else
  template <class T>
  inline void HepAList<T>::sort(AListSortCompareFunc f)
  {
    qsort(p, n, sizeof(void*), f);
  }
#endif


// template <class T>
// inline void HepAList<T>::deleteAll() {
//   T * e;
//   while ( e = last() ) {
//     remove(e);
//     delete e;
//   }
// }
  template <class T>
  inline void AListSort(HepAList<T> &l)
  {
    if (l.length() > 1) {
      unsigned int i, j;

      for (i = l.length() - 1; i >= 1; i--) {
        for (j = 0; j < i; j++) {
          if (*(l[j]) > (*l[j + 1])) l.swap(j, j + 1);
// Yes it's slower. But it does not have to be a friend.
//           {
//              T *tmp = (T *)l.p[j];
//              l.p[j] = (T *)l.p[j+1];
//              l.p[j+1] = (T *)tmp;
//           }
        }
      }
    }
  }

  template <class T>
  inline void AListReverseSort(HepAList<T> &l)
  {
    if (l.length() > 1) {
      unsigned int i, j;

      for (i = l.length() - 1; i >= 1; i--) {
        for (j = 0; j < i; j++) {
          if ((*l[j]) < * (l[j + 1])) l.swap(j, j + 1);
// Yes it's slower. But it does not have to be a friend.
//       {
//          T *tmp = (T *)l.p[j];
//          l.p[j] = (T *)l.p[j+1];
//          l.p[j+1] = (T *)tmp;
//       };
        }
      }
    }
  }

} // namespace Belle

#endif

