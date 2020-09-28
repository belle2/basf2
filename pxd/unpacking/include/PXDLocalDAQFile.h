//+
// File : PXDLocalDAQFile.h
// Description : Blocked file management for sequential binary I/O
//
// Author : Bjoern Spruck/ Maiko Takahashi
// Date : 2020
//-

#pragma once

#include <iosfwd>
#include <memory>
#include <string>

namespace Belle2 {

  /** A class to manage I/O for a chain of blocked files */
  class PXDLocalDAQFile {
  public:
    /** Constructor */
    PXDLocalDAQFile(const std::string& filename);
    /** Destructor */
    ~PXDLocalDAQFile();
    /** No copying */
    PXDLocalDAQFile(const PXDLocalDAQFile&) = delete;
    /** No assignment */
    PXDLocalDAQFile& operator=(const PXDLocalDAQFile&) = delete;
    /** Returns status after constructor call. If success, fd is returned. If not, -1 */
    int status() const;

    /** Read a record from a file. Returns the data size read.*/
    int read(char* buf, int max);
    /** Read a record from a file. Returns the data size if successfully read, otherwise 0.*/
    int read_data(char* data, size_t len);

  private:
    /** actually open the file */
    void openFile(std::string filename);

  private:

    std::string m_filename; /**< Name of the opened file. */
    int m_fd{ -1}; /**< file descriptor. */
    bool m_compressed{false}; /**< is file bzip2 compressed? */
    std::unique_ptr<std::ios> m_stream; /**< pointer to the filtering input or output stream */

  };

}
