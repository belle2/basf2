#pragma once

#include <framework/pcore/SeqFile.h>
#include <cstdio>

namespace Belle2 {
  class HLTFile {
  public:
    bool open(const std::string& fileName, bool raw, const char* mode);
    virtual ~HLTFile();

    int put_wordbuf(int* data, int len);
    int put(char* data, int len);
    int get_wordbuf(int* data, int len);
    int get(char* data, int len);

  private:
    std::unique_ptr<SeqFile> m_sfile;
    FILE* m_rfile = nullptr;
  };
}






