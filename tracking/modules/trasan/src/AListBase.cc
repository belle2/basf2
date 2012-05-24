// -*- C++ -*-
// $Id: AListBase.cc 10002 2007-02-26 06:56:17Z katayama $
//
// This file is a part of what might become the CLHEP -
// a Class Library for High Energy Physics.
//
// This is the implementation of the HepAListBase class.
//

#ifdef GNUPRAGMA
#pragma implementation
#endif

//iw
#include <stdlib.h>
#include <stdio.h>

//iw #include "belleCLHEP/config/CLHEP.h"
#include "tracking/modules/trasan/CLHEP.h"
//iw #include "belleCLHEP/Alist/AListBase.h"
#include "tracking/modules/trasan/AListBase.h"

#ifdef HEP_DEBUG_INLINE
//iw #include "belleCLHEP/Alist/AListBase.icc"
#include "tracking/modules/trasan/AListBase.icc"
#endif

namespace Belle {

  HepAListBase::~HepAListBase()
  {
    if (p) {
      free(p);
    }
    p = 0;
  }

  void HepAListBase::sort(AListSortCompareFunc compfunc)
  {
    qsort(p, n, sizeof(void*), compfunc);
  }

  void HepAListBase::realloc()
  {
//cnv  if (NULL==n) {
    if (0 == n) {
      if (p) free(p);
      p = NULL;
    } else {
      if (NULL == (p = (void**)(p ? ::realloc(p, n * sizeof(void*)) :
                                malloc(n * sizeof(void*))))) {
        perror("$Source$:p:malloc");
        exit(1);
      }
    }
  }

  void HepAListBase::copyArray(register void** dst, register void**   src,
                               register int arraySize)
  {
    while (--arraySize >= 0) *(dst++) = *(src++);
  }

  void*& HepAListBase::newInsert(register int ir)
  {
    n++; realloc();
    register int i;
    for (i = n - 1; i > ir; i--)
      p[i] = p[i - 1];
    return i >= 0 ? p[i] : p[0];
  }

  void*& HepAListBase::newAppend(register int ir)
  {
    n++; realloc();
    register int i;
    for (i = n - 1; i > ir; i--)
      p[i] = p[i - 1];
    return p[i];
  }

  void HepAListBase::removeEntry(int ir)
  {
    for (register int i = ir + 1; i < int(n); i++) p[i - 1] = p[i];
    n--;
    realloc();
  }

  void HepAListBase::append(void* e, void* r)
  {
    if (e) {
      int ir = index(r);
      newAppend(ir >= 0 ? ir + 1 : n) = e;
    }
  }

  void HepAListBase::append(const HepAListBase& l)
  {
    int ln = l.n;
    n += ln;
    realloc();
    copyArray(p + n - ln, l.p, ln);
  }

  void HepAListBase::operator = (const HepAListBase& l)
  {
    if (this != &l) {
      n = l.n;
      realloc();
      copyArray(p, l.p, n);
    }
  }

  HepAListBase::HepAListBase(const HepAListBase& l)
    : p((void**) malloc(l.n* sizeof(void*))), n(l.n)
  {
    if (NULL == p) {
      perror("$Id: AListBase.cc 10002 2007-02-26 06:56:17Z katayama $:p:malloc");
      exit(1);
    }
    copyArray(p, l.p, n);
  }

  int HepAListBase::fIndex(void* e) const
  {
    register int i = -1;
    while (++i < int(n)) if (p[i] == e) return i;
    return -1;
  }

  int HepAListBase::index(void* e) const
  {
    register int i = n;
    while (--i >= 0 && p[i] != e);
    return i;
  }

  void HepAListBase::purge()
  {
    int ie;
    for (int i = 0; i < int(n); i++) {
      while ((ie = index(p[i])) != i) removeEntry(ie);
    }
  }

  void HepAListBase::remove(const HepAListBase& l)
  {
    register int i = l.n;
    while (--i >= 0) remove(l.p[i]);
  }

  void HepAListBase::remove(void* e)
  {
    if (e && n) {
      int ir;
      while ((ir = index(e)) >= 0) removeEntry(ir);
    }
  }

  void HepAListBase::replace(register void* eo, register void* en)
  {
    register int i = n;
    register void** q = p;
    while (--i >= 0) {
      if (*q == eo) *q = en;
      q++;
    }
  }

  void HepAListBase::reverse()
  {

    register int i = n / 2;
    register void** pf = p;
    register void** pl = p + n - 1;
    register void* t;

    while (i-- > 0) {
      t = *pf;
      *(pf++) = *pl;
      *(pl--) = t;
    }
  }

  void HepAListBase::swap(unsigned i1, unsigned i2)
  {
    if (i1 >= (unsigned) n || i2 >= (unsigned) n || i1 == i2) return;
    void* e = p[i1];
    p[i1] = p[i2];
    p[i2] = e;
  }

} // namespace Belle
