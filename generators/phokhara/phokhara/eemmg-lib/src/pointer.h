/*
 * pointer.h - smart pointer and array classes
 *
 * this file is part of PJFry library
 * Copyright 2011 Valery Yundin
 */

#ifndef QUL_POINTER_H
#define QUL_POINTER_H

#include <memory>
#include <cstring>

class SRefCnt {
public:
  SRefCnt() : count(0) { }

protected:
  int count;
};

template <typename T>
class SPtr {
public:
  T* operator-> () const { return pObj; }
  bool operator== (const T* pobj) const { return pobj == pObj; }
  bool operator!= (const T* pobj) const { return pobj != pObj; }

  bool operator== (const SPtr<T>& spobj) const { return spobj.pObj == pObj; }
  bool operator!= (const SPtr<T>& spobj) const { return spobj.pObj != pObj; }

  SPtr(T* pobj = 0) : pObj(pobj)
  {
    if (pObj) { pObj->count++; }
  }
  SPtr(const SPtr& ptr) : pObj(ptr.pObj)
  {
    if (pObj) { pObj->count++; }
  }

  SPtr& operator= (const SPtr& ptr)
  {
    if (this == &ptr) { return *this; }
    if (pObj && --(pObj->count) == 0) { delete pObj; }
    if ((pObj = ptr.pObj)) { pObj->count++; }
    return *this;
  }

  ~SPtr()
  {
    if (pObj && --(pObj->count) == 0) { delete pObj; }
  }

private:
  T* pObj;
};

template <typename T, int N> class DArray;
// Iterator for DArray class
template <typename T, int N>
class NIter {
  friend class DArray<T, N>;
public:
  inline T& operator* () { return ptr[idx % N]; }
  inline T* operator-> () { return &ptr[idx % N]; }
  inline NIter& operator++ () { idx++; return *this;}
  inline NIter& operator+= (int n) { idx += n; return *this;}

  inline bool operator== (const NIter& iter) { return idx == iter.idx && ptr == iter.ptr; }
  inline bool operator!= (const NIter& iter) { return idx != iter.idx || ptr != iter.ptr; }

  NIter(T* begin, int last) : ptr(begin), idx(last) {}
private:
  T* ptr;
  int idx;
};

// DArray - static array with stack-like iterator
template <typename T, int N>
class DArray {
public:
  DArray() : last(N), len(0) {}

  typedef NIter<T, N> iterator;
  iterator begin() { return iterator(elems, last); }
  iterator end()   { return iterator(elems, last + len); }

  T& insert(const T& el)
  {
    len = (len == N ? len : len + 1);
    last = ((last - 1) + N) % N;
    elems[last] = el;
    return elems[last];
  }

#ifdef USE_SMART_INSERT
  void remove(iterator& it)
  {
//       assert(it.ptr==elems);
    int i = it.idx % N;
    elems[i] = T();
    if (i >= last) {
      memmove(&elems[last + 1], &elems[last], (i - last)*sizeof(T));
      memset(&elems[last], 0, sizeof(T));
      last = (last + 1) % N;
    } else {
      memmove(&elems[i], &elems[i + 1], (last - i - 1)*sizeof(T));
      memset(&elems[last - 1], 0, sizeof(T));
    }
    len = len - 1;
  }
#endif

  void reset()
  {
#ifndef USE_DIRTY_RESET
    for (int i = 0; i < len; i++) {
      elems[i] = T();
    }
#endif
    last = N;
    len = 0;
  }

  static const int size = N;
  const T& operator [](const int idx) const { return elems[idx]; }
  T& operator [](const int idx) { return elems[idx]; }

private:
  T elems[N];
  int last;
  int len;
};

// CArray - simple array with trivial iterator
template <typename T, int N>
class CArray {
public:
  typedef std::auto_ptr<CArray> Ptr;

  CArray(T dval = T())
  {
    for (iterator i = begin(); i != end(); ++i) {
      *i = dval;
    }
  }

  typedef T* iterator;
  iterator begin() { return &elems[0]; }
  iterator end()   { return &elems[N]; }


  static const int size = N;

  const T& operator [](const int idx) const { return elems[idx]; }
  T& operator [](const int idx) { return elems[idx]; }

private:
  T elems[N];
};

#endif /* QUL_POINTER_H */

/*
class Test: public SRefCnt
{
  public:
    friend class SPtr<Test>;
    typedef SPtr<Test> Ptr;

    static Ptr create(int x) { return Ptr(new Test(x)); };

    ~Test() { };

  private:
    Test() { };
};

*/
