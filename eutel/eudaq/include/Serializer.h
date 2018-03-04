#ifndef EUDAQ_INCLUDED_Serializer
#define EUDAQ_INCLUDED_Serializer

#include <string>
#include <vector>
#include <map>
#include <eutel/eudaq/Serializable.h>
#include <eutel/eudaq/counted_ptr.h>
#include <eutel/eudaq/Time.h>
#include <eutel/eudaq/Exception.h>

namespace eudaq {

  class InterruptedException : public std::exception {
    const char* what() const throw() { return "InterruptedException"; }
  };

  class Serializer {
  public:
    virtual void Flush() {}
    void write(const Serializable& t)
    {
      t.Serialize(*this);
    }
    template<typename T>
    void write(const T& t);
    template<typename T>
    void write(const std::vector<T>& t);
    template<typename T, typename U>
    void write(const std::map<T, U>& t);
    template<typename T, typename U>
    void write(const std::pair<T, U>& t);

    virtual ~Serializer() {}
  private:
    template <typename T>
    friend struct WriteHelper;
    virtual void Serialize(const unsigned char*, size_t) = 0;
  };

  template <typename T>
  struct WriteHelper {
    typedef void (*writer)(Serializer& ser, const T& v);
    static writer GetFunc(Serializable*) { return write_ser; }
    static writer GetFunc(float*) { return write_float; }
    static writer GetFunc(double*) { return write_double; }
    static writer GetFunc(...) { return write_int; }

    static void write_ser(Serializer& sr, const T& v)
    {
      v.Serialize(sr);
    }
    static void write_int(Serializer& sr, const T& v)
    {
      T t = v;
      unsigned char buf[sizeof t];
      for (size_t i = 0; i < sizeof t; ++i) {
        buf[i] = static_cast<unsigned char>(t & 0xff);
        t >>= 8;
      }
      sr.Serialize(buf, sizeof t);
    }
    static void write_float(Serializer& sr, const float& v)
    {
      unsigned t = *(unsigned*)&v;
      unsigned char buf[sizeof t];
      for (size_t i = 0; i < sizeof t; ++i) {
        buf[i] = t & 0xff;
        t >>= 8;
      }
      sr.Serialize(buf, sizeof t);
    }
    static void write_double(Serializer& sr, const double& v)
    {
      unsigned long long t = *(unsigned long long*)&v;
      unsigned char buf[sizeof t];
      for (size_t i = 0; i < sizeof t; ++i) {
        buf[i] = t & 0xff;
        t >>= 8;
      }
      sr.Serialize(buf, sizeof t);
    }
  };

  template <typename T>
  inline void Serializer::write(const T& v)
  {
    WriteHelper<T>::GetFunc(static_cast<T*>(0))(*this, v);
  }

  template <>
  inline void Serializer::write(const std::string& t)
  {
    write((unsigned)t.length());
    Serialize(reinterpret_cast<const unsigned char*>(&t[0]), t.length());
  }

  template <>
  inline void Serializer::write(const Time& t)
  {
    write((int)timeval(t).tv_sec);
    write((int)timeval(t).tv_usec);
  }

  template <typename T>
  inline void Serializer::write(const std::vector<T>& t)
  {
    unsigned len = t.size();
    write(len);
    for (size_t i = 0; i < len; ++i) {
      write(t[i]);
    }
  }

  template <>
  inline void Serializer::write<unsigned char>(const std::vector<unsigned char>& t)
  {
    write((unsigned)t.size());
    Serialize(&t[0], t.size());
  }

  template <>
  inline void Serializer::write<char>(const std::vector<char>& t)
  {
    write((unsigned)t.size());
    Serialize(reinterpret_cast<const unsigned char*>(&t[0]), t.size());
  }

  template <typename T, typename U>
  inline void Serializer::write(const std::map<T, U>& t)
  {
    unsigned len = (unsigned)t.size();
    write(len);
    for (typename std::map<T, U>::const_iterator i = t.begin(); i != t.end(); ++i) {
      write(i->first);
      write(i->second);
    }
  }

  template <typename T, typename U>
  inline void Serializer::write(const std::pair<T, U>& t)
  {
    write(t->first);
    write(t->second);
  }

  class Deserializer {
  public:
    Deserializer() : m_interrupting(false) {}
    virtual bool HasData() = 0;
    void Interrupt() { m_interrupting = true; }

    template <typename T>
    void read(T& t);

    template <typename T>
    void read(std::vector<T>& t);

    template <typename T, typename U>
    void read(std::map<T, U>& t);

    template <typename T, typename U>
    void read(std::pair<T, U>& t);

    template <typename T>
    T read()
    {
      T t;
      read(t);
      return t;
    }

    virtual ~Deserializer() {}
  protected:
    bool m_interrupting;
  private:
    template <typename T>
    friend struct ReadHelper;
    virtual void Deserialize(unsigned char*, size_t) = 0;
  };

  template <typename T>
  struct ReadHelper {
    typedef T(*reader)(Deserializer& ser);
    static reader GetFunc(Serializable*) { return read_ser; }
    static reader GetFunc(float*) { return read_float; }
    static reader GetFunc(double*) { return read_double; }
    static reader GetFunc(...) { return read_int; }

    static T read_ser(Deserializer& ds)
    {
      return T(ds);
    }
    static T read_int(Deserializer& ds)
    {
      unsigned char buf[sizeof(T)];
      ds.Deserialize(buf, sizeof(T));
      T t = 0;
      for (size_t i = 0; i < sizeof t; ++i) {
        t <<= 8;
        t += buf[sizeof t - 1 - i];
      }
      return t;
    }
    static float read_float(Deserializer& ds)
    {
      unsigned char buf[sizeof(float)];
      ds.Deserialize(buf, sizeof buf);
      unsigned t = 0;
      for (size_t i = 0; i < sizeof t; ++i) {
        t <<= 8;
        t += buf[sizeof t - 1 - i];
      }
      return *(float*)&t;
    }
    static double read_double(Deserializer& ds)
    {
      union { double d; unsigned long long i; unsigned char b[sizeof(double)]; } u;
      //unsigned char buf[sizeof (double)];
      ds.Deserialize(u.b, sizeof u.b);
      unsigned long long t = 0;
      for (size_t i = 0; i < sizeof t; ++i) {
        t <<= 8;
        t += u.b[sizeof t - 1 - i];
      }
      u.i = t;
      return u.d;
    }
  };

  template <typename T>
  inline void Deserializer::read(T& t)
  {
    t = ReadHelper<T>::GetFunc(static_cast<T*>(0))(*this);
  }

  template <>
  inline void Deserializer::read(std::string& t)
  {
    unsigned len = 0;
    read(len);
    t = std::string(len, ' ');
    if (len) Deserialize(reinterpret_cast<unsigned char*>(&t[0]), len);
  }

  template <>
  inline void Deserializer::read(Time& t)
  {
    int sec, usec;
    read(sec);
    read(usec);
    t = Time(sec, usec);
  }

  template <typename T>
  inline void Deserializer::read(std::vector<T>& t)
  {
    unsigned len = 0;
    read(len);
    t.reserve(len);
    for (size_t i = 0; i < len; ++i) {
      t.push_back(read<T>());
    }
  }

  template <>
  inline void Deserializer::read<unsigned char>(std::vector<unsigned char>& t)
  {
    unsigned len = 0;
    read(len);
    t.resize(len);
    Deserialize(&t[0], len);
  }

  template <>
  inline void Deserializer::read<char>(std::vector<char>& t)
  {
    unsigned len = 0;
    read(len);
    t.resize(len);
    Deserialize(reinterpret_cast<unsigned char*>(&t[0]), len);
  }

  template<typename T, typename U>
  inline void Deserializer::read(std::map<T, U>& t)
  {
    unsigned len = 0;
    read(len);
    for (size_t i = 0; i < len; ++i) {
      std::string name = read<std::string>();
      std::string val = read<std::string>();
      t[name] = val;
    }
  }

  template<typename T, typename U>
  inline void Deserializer::read(std::pair<T, U>& t)
  {
    t.first  = read<T>();
    t.second = read<U>();
  }

}

#endif // EUDAQ_INCLUDED_Serializer
