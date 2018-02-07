/*
 * counted_ptr - simple reference counted pointer.
 *
 * The is a non-intrusive implementation that allocates an additional
 * int and pointer for every counted object.
 *
 * From: http://ootips.org/yonat/4dev/
 * Thanks to Yonat Sharon for this code
 *
 */

#ifndef COUNTED_PTR_H
#define COUNTED_PTR_H

#ifdef _GLIBCXX_DEBUG
#include <vector>
#define EUDAQ_DIE std::vector<char>(1)[100]
#endif

/* For ANSI-challenged compilers, you may want to #define
 * NO_MEMBER_TEMPLATES or explicit */

template <class X> class counted_ptr {
public:
  typedef X element_type;

  explicit counted_ptr(X* p = 0) // allocate a new counter
    : itsCounter(0) {if (p) itsCounter = new counter(p);}
  ~counted_ptr()
  {release();}
  counted_ptr(const counted_ptr& r) throw()
  {acquire(r.itsCounter);}
  counted_ptr& operator=(const counted_ptr& r)
  {
    if (this != &r) {
      release();
      acquire(r.itsCounter);
    }
    return *this;
  }
  counted_ptr& operator=(X* p)
  {
    release();
    if (p) itsCounter = new counter(p);
    return *this;
  }
  operator bool () const
  {
    return get() != 0;
  }
#ifndef NO_MEMBER_TEMPLATES
  template <class Y> friend class counted_ptr;
  template <class Y> counted_ptr(const counted_ptr<Y>& r) throw()
  {acquire(r.itsCounter);}
  template <class Y> counted_ptr& operator=(const counted_ptr<Y>& r)
  {
    if (this != &r) {
      release();
      acquire(r.itsCounter);
    }
    return *this;
  }
#endif // NO_MEMBER_TEMPLATES

  X& operator*()  const throw()
  {
#ifdef _GLIBCXX_DEBUG
    if (!itsCounter || !itsCounter->ptr) EUDAQ_DIE;
#endif
    return *itsCounter->ptr;
  }
  X* operator->() const throw()
  {
#ifdef _GLIBCXX_DEBUG
    if (!itsCounter || !itsCounter->ptr) EUDAQ_DIE;
#endif
    return itsCounter->ptr;
  }
  X* get()        const throw()
  {
#ifdef _GLIBCXX_DEBUG
    //if (!itsCounter || !itsCounter->ptr) EUDAQ_DIE;
#endif
    return itsCounter ? itsCounter->ptr : 0;
  }
  bool unique()   const throw()
  {return (itsCounter ? itsCounter->count == 1 : true);}

private:

  struct counter {
    counter(X* p = 0, unsigned c = 1) : ptr(p), count(c) {}
    X*          ptr;
    unsigned    count;
  }* itsCounter;

  void acquire(counter* c) throw()
  {
    // increment the count
    itsCounter = c;
    if (c) ++c->count;
  }

  void release()
  {
    // decrement the count, delete if it is 0
    if (itsCounter) {
      if (--itsCounter->count == 0) {
        delete itsCounter->ptr;
        delete itsCounter;
      }
      itsCounter = 0;
    }
  }
};

#endif // COUNTED_PTR_H

