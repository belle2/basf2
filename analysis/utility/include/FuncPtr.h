/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Mikihiko Nakao (KEK), Pablo Goldenzweig (KIT)           *
 *   Original module writen by M. Nakao for Belle                         *
 *   Ported to Belle II by P. Goldenzweig                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// ----------------------------------------------------------------------
// MemberFunc - retrieve the pointer to a member function
// example:
//   list<Particle> pl;
//   Vector3 t = thrust(pl.begin(), pl.end(), MemberFunc(&Particle::vec));
//
//   list<Particle *> pp;
//   Vector3 t = thrust(pp.begin(), pp.end(), MemberFunc(&Particle::vec));
// ----------------------------------------------------------------------

#ifndef FUNCPTR_H
#define FUNCPTR_H

#include <vector>
#include <map>
#include <string>
#include <utility>
#include <iostream>
#include <sstream>

#include <analysis/utility/VariableManager.h>

namespace Belle2 {

  template <class T, class Result>
  class ptr_to_member_func {
  protected:
    typedef Result(T::*pfun)() const;
  public:
    ptr_to_member_func() {};
    ptr_to_member_func(Result(T::*x)()) : ptr(x) {};
    ptr_to_member_func(const Result(T::*x)() const) : ptr(x) {};
    const Result operator()(T& t) const { return (t.*ptr)(); }
    const Result operator()(T* t) const { return ((*t).*ptr)(); }
    const Result operator()(const T& t) const { return (t.*ptr)(); }
    const Result operator()(const T* t) const { return ((*t).*ptr)(); }
  protected:
    const pfun ptr;
  };

  template <class T, class Result>
  ptr_to_member_func<T, Result> MemberFunc(const Result(T::*x)() const)
  {
    return ptr_to_member_func<T, Result>(x);
  }

// ----------------------------------------------------------------------
// MemberFunc for iterators
// example:
//   list<list<Particle>::iterator> pi;
//   Vector3 t = thrust(pi.begin(), pi.end(),
//               MemberFunc(&Particle::vec, list<Particle>::iterator()));
// ----------------------------------------------------------------------

  template <class T, class Result, class Iter>
  class ptr_to_member_func_from_iter {
  protected:
    typedef Result(T::*pfun)() const;
  public:
    ptr_to_member_func_from_iter() {};
    ptr_to_member_func_from_iter(Result(T::*x)()) : ptr(x) {};
    ptr_to_member_func_from_iter(const Result(T::*x)() const) : ptr(x) {};
    const Result operator()(Iter& it) const { return ((*it).*ptr)(); }
    const Result operator()(const Iter& it) const { return ((*it).*ptr)(); }
  protected:
    const pfun ptr;
  };

  template <class T, class Result, class Iter>
  ptr_to_member_func_from_iter<T, Result, Iter>
  MemberFunc(const Result(T::*x)() const, const Iter& it)
  {
    return ptr_to_member_func_from_iter<T, Result, Iter>(x);
  }

// ----------------------------------------------------------------------
// SelfFunc - retrieve the pointer to a function which returns itself
// example:
//   list<Vector3> vl;
//   Vector3 t = thrust(vl.begin(), vp.end(), SelfFunc(Vector3()));
//
//   list<Vector3 *> vp;
//   Vector3 t = thrust(vp.begin(), vp.end(), SelfFunc(Vector3()));
// ----------------------------------------------------------------------

  template <class T>
  class ptr_to_self_func {
  protected:
    typedef T(*pfun)(T&);
  public:
    ptr_to_self_func() : ptr(NULL) {};
    T operator()(T& t) const { return t; };
    T operator()(T* t) const { return *t; };
    const T operator()(const T& t) const { return t; };
    const T operator()(const T* t) const { return *t; };
  protected:
    const pfun ptr;
  };

  template <class T>
  ptr_to_self_func<T> SelfFunc(const T&)
  {
    return ptr_to_self_func<T>();
  }

// ----------------------------------------------------------------------
// SelfFunc for iterators
// example:
//   list<list<Vector3>::iterator> pi;
//   Vector3 t = thrust(pi.begin(), pi.end(),
//               SelfFunc(Vector3(), list<Vector3>::iterator()));
// ----------------------------------------------------------------------

  template <class T, class Iter>
  class ptr_to_self_func_from_iter {
  protected:
    typedef T(*pfun)(T&);
  public:
    ptr_to_self_func_from_iter() : ptr(NULL) {};
    T operator()(Iter& it) const { return *it; };
    const T operator()(const Iter& it) const { return *it; };
  protected:
    const pfun ptr;
  };

  template <class T, class Iter>
  ptr_to_self_func_from_iter<T, Iter> SelfFunc(const T& x, const Iter& it)
  {
    return ptr_to_self_func_from_iter<T, Iter>();
  }

} // Belle2 namespace

#endif
