#ifndef _Belle2_RunLogMessanger_h
#define _Belle2_RunLogMessanger_h

#include <string>

namespace Belle2 {

  class RunLogMessanger {

  public:
    static const int DEBUG;
    static const int NOTICE;
    static const int ERROR;
    static const int FATAL;

  public:
    RunLogMessanger() {}
    ~RunLogMessanger() {}

  public:
    bool open(const std::string& path, const std::string& mode = "r");
    bool create(const std::string& path, const std::string& mode = "r");
    void close();
    void unlink(const std::string& path = "");
    std::string recieve(int& priority, int timeout = -1);
    bool send(int priority, const std::string& message);

  private:
    int write(const void* v, size_t count);
    int read(void* v, size_t count);

  private:
    std::string _path;
    int _fifo;

  };

}

#endif
