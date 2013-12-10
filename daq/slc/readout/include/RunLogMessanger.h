#ifndef _Belle2_RunLogMessanger_h
#define _Belle2_RunLogMessanger_h

#include <daq/slc/system/FifoWriter.h>
#include <daq/slc/system/FifoReader.h>
#include <daq/slc/base/SystemLog.h>

namespace Belle2 {

  class RunLogMessanger {

  public:
    RunLogMessanger() {}
    ~RunLogMessanger() {}

  public:
    bool open(const std::string& path, const std::string& mode = "r");
    bool create(const std::string& path, const std::string& mode = "r");
    void close();
    void unlink(const std::string& path = "");
    SystemLog recieveLog() throw(IOException) {
      SystemLog log;
      _reader.readObject(log);
      return log;
    }
    void sendLog(const SystemLog& log) throw(IOException) {
      _writer.writeObject(log);
    }

  private:
    std::string _path;
    Fifo _fifo;
    FifoWriter _writer;
    FifoReader _reader;

  };

}

#endif
