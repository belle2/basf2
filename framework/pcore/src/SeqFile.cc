//+
// File : SeqFile.cc
// Description : Sequential file I/O privitives with file blocking
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 28 - Jun - 2008
//-

#include <framework/pcore/SeqFile.h>
#include <framework/logging/Logger.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_stream.hpp>

using namespace Belle2;
using namespace std;
namespace io = boost::iostreams;

SeqFile::SeqFile(const std::string& filename, const std::string& rwflag):
  m_filename(filename)
{
  if (filename.empty()) {
    B2ERROR("SeqFile: Empty filename given");
    return;
  }
  bool readonly = rwflag.find('w') == std::string::npos;
  // is the file already compressed?
  m_compressed = filename.compare(filename.size() - 3, 3, ".gz") == 0;
  // strip .gz suffix to add it at the end automatically and correctly for subsequent files
  if (m_compressed) {
    m_filename = filename.substr(0, filename.size() - 3);
  }
  // open the file
  openFile(m_filename, readonly);
  // if that fails and it's not already assumed to be compressed try again adding .gz to the name
  if (m_fd < 0 && !m_compressed) {
    B2WARNING("SeqFile: error opening '" << filename << "': " << strerror(errno)
              << ", trying again with '.gz'");
    m_compressed = true;
    openFile(filename, readonly);
  }
  // is the file open now?
  if (m_fd < 0) {
    B2ERROR("SeqFile: error opening '" << filename << "': " << strerror(errno));
  } else {
    B2INFO("SeqFile: " << m_filename << " opened (fd=" << m_fd << ")");
  }
}

void SeqFile::openFile(std::string filename, bool readonly)
{
  close(m_fd);
  // add compression suffix if file is supposed to be compressed
  if (m_compressed) filename += ".gz";
  if (!readonly) {
    //open file in create mode and set stream correctly
    m_fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0664);
    auto filter = new io::filtering_ostream();
    if (m_compressed) filter->push(io::gzip_compressor());
    filter->push(io::file_descriptor_sink(m_fd, io::never_close_handle));
    filter->exceptions(ios_base::badbit | ios_base::failbit);
    m_stream.reset(filter);
  } else {
    //open file in read mode and set stream correctly
    m_fd = open(filename.c_str(), O_RDONLY);
    auto filter = new io::filtering_istream();
    if (m_compressed) filter->push(io::gzip_decompressor());
    filter->push(io::file_descriptor_source(m_fd, io::never_close_handle));
    filter->exceptions(ios_base::badbit | ios_base::failbit);
    m_stream.reset(filter);
  }
  // reset number of written bytes
  m_nb = 0;
}

SeqFile::~SeqFile()
{
  close(m_fd);
  B2INFO("Seq File " << m_nfile << " closed");
}

int SeqFile::status()
{
  return m_fd;
}

int SeqFile::write(char* buf)
{
  int insize = *((int*)buf); // nbytes in the buffer at the beginning
  if (insize + m_nb >= BLOCKSIZE && m_filename != "/dev/null") {
    B2INFO("SeqFile: previous file closed (size=" << m_nb << " bytes)");
    m_nfile++;
    auto file = m_filename + '-' + std::to_string(m_nfile);
    openFile(file, false);
    if (m_fd < 0) {
      B2FATAL("SeqFile::write() error opening file '" << file << "': " << strerror(errno));
    }
    m_nb = 0;
    B2INFO("SeqFile::write() opened '" << file << "'");
  }
  // cast stream object
  std::ostream* out = dynamic_cast<std::ostream*>(m_stream.get());
  if (!out) {
    B2FATAL("SeqFile::write() called on a file opened in read mode");
  }
  try {
    out->write(buf, insize);
    m_nb += insize;
    return insize;
  } catch (ios_base::failure& e) {
    B2ERROR("SeqFile::write() error: " << e.what() << ", " << strerror(errno));
    return 0;
  }
}

int SeqFile::read(char* buf, int size)
{
  // cast stream object
  std::istream* in = dynamic_cast<std::istream*>(m_stream.get());
  if (!in) {
    B2FATAL("SeqFile::read() called on a file opened in write mode");
  }
  //trigger eof if there's nothing left int the file. Could throw an error on decompress failure
  try {
    in->peek();
  } catch (ios_base::failure& e) {
    B2ERROR("SeqFile::read() cannot read file: " << e.what());
    return -1;
  }
  //ok, now we can open the next file reliably
  if (in->eof()) {
    // EOF of current file, search for next file
    m_nfile++;
    auto nextfile = m_filename + '-' + std::to_string(m_nfile);
    openFile(nextfile, true);
    if (m_fd < 0) return 0;   // End of all files
    // update the stream pointer
    in = dynamic_cast<std::istream*>(m_stream.get());
  }
  try {
    // Obtain new header
    in->read(buf, sizeof(int));
  } catch (ios_base::failure& e) {
    B2ERROR("SeqFile::read() " << e.what() << ": couldn't read next record size");
    return -1;
  }
  // Normal processing, extract the record size from the first 4 bytes
  int recsize = *((int*)buf);
  if (recsize > size) {
    B2ERROR("SeqFile::read() buffer too small, need at least " << recsize << " bytes");
    return -1;
  }
  try {
    in->read(buf + sizeof(int), recsize - sizeof(int));
  } catch (ios_base::failure& e) {
    B2ERROR("SeqFile::read() " << e.what() << ": could only read " << in->gcount() << " bytes, expected " << recsize);
    return -1;
  }
  return recsize;
}
