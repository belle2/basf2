#ifndef SEQFILE_H
#define SEQFILE_H
//+
// File : seqfile.h
// Description : Blocked file management for sequential binary I/O
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 28 - Aug - 2003
//-

//define BLOCKSIZE 512000000
#define BLOCKSIZE 512000000*4
#define SEQBUFSIZE 4000000

namespace Belle2 {

  class SeqFile {
  public:
    SeqFile(const char* filename, const char* access);
    ~SeqFile();
    int status(void);

    int write(char* buf);
    int read(char* buf, int max);

  private:
    void flush_buffer(int);

  private:
    char m_filename[256];
    int m_fd;
    int m_nb;
    int m_nfile;
    int* m_buf;
    int m_ptr;
  };

} // namespace Belle

#endif
