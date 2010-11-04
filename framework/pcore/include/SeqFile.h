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

  /*! A class to manage I/O for a chain of blocked files */
  class SeqFile {
  public:
    /*! Constructor */
    SeqFile(const char* filename, const char* access);
    /*! Destructor */
    ~SeqFile();
    /*! Returns status after constructor call. If success, fd is returned. If not, -1 */
    int status(void);

    /*! Write a record to a file.  First word of the record should contain number of words.*/
    int write(char* buf);
    /*! Read a record from a file. The record length is returned. */
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
