//+
// File : seqfile.h
// Description : Blocked file management for sequential binary I/O
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 28 - Aug - 2003
//-

#pragma once

#include <iosfwd>
#include <memory>
#include <string>

namespace Belle2 {

  /** A class to manage I/O for a chain of blocked files */
  class SeqFile {
  public:
    /** Constructor.
     *
     * @param rwflag should probably be r or rw
     * @param filenameIsPattern if true interpret the filename as a
     *     boost::format pattern which takes the sequence number as argument
     *     instead of producing .sroot-N files
     */
    SeqFile(const std::string& filename, const std::string& rwflag,
            char* streamerinfo = NULL, int streamerinfo_size = 0,
            bool filenameIsPattern = false);
    /** Destructor */
    ~SeqFile();
    /** Returns status after constructor call. If success, fd is returned. If not, -1 */
    int status() const;

    /** Write a record to a file.  First word of the record should contain number of words.*/
    int write(const char* buf);
    /** Read a record from a file. The record length is returned. */
    int read(char* buf, int max);

  private:
    /** actually open the file */
    void openFile(std::string filename, bool readonly);

  private:
    /** maximal size of one file (in Bytes). */
    const static int c_MaxFileSize {512000000 * 4};

    std::string m_filename; /**< Name of the opened file. */
    std::string m_filenamePattern; /**< Pattern for creating the file from the sequence number */
    int m_fd{ -1}; /**< file descriptor. */
    int m_nb{0}; /**< when saving a file, the total number of bytes written, 0 when reading. */
    int m_nfile{0}; /**< file counter, starting at 0 (files are split after c_MaxFileSize bytes). */
    bool m_compressed{false}; /**< is file gzipped compressed? */
    std::unique_ptr<std::ios> m_stream; /**< pointer to the filtering input or output stream */

    /** StreamerInfo */
    char* m_streamerinfo;

    /** size(bytes) of StreamerInfo */
    int m_streamerinfo_size;

  };

}
