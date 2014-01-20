#ifndef H_EUDAQ_MUTEX
#define H_EUDAQ_MUTEX

namespace eudaq {

  class Mutex {
  public:
    Mutex();
    ~Mutex();
    int Lock();
    int TryLock();
    int UnLock();
  private:
    class Impl;
    Impl* m_impl;
  };

  class MutexLock {
  public:
    MutexLock(Mutex& m, bool lock = true);
    ~MutexLock();
    void Lock();
    void Release();
  private:
    Mutex& m_mutex;
    bool m_locked;
  };

//   class MutexTryLock {
//   public:
//     MutexTryLock(Mutex & m);
//     ~MutexTryLock();
//     void Release();
//     operator bool () const;
//   private:
//     Mutex & m_mutex;
//     bool m_locked;
//   };

}

#endif // H_EUDAQ_MUTEX
