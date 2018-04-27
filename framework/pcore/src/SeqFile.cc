//+
// File : SeqFile.cc
// Description : Sequential file I/O privitives with file blocking
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 28 - Jun - 2008
//-

#include <framework/pcore/SeqFile.h>
#include <framework/logging/Logger.h>

#include <ios>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/format.hpp>

using namespace Belle2;
using namespace std;
namespace io = boost::iostreams;

SeqFile::SeqFile(const std::string& filename, const std::string& rwflag, char* streamerinfo, int streamerinfo_size,
                 bool filenameIsPattern):
  m_filename(filename)
{
  if (filename.empty()) {
    B2ERROR("SeqFile: Empty filename given");
    return;
  }
  bool readonly = rwflag.find('w') == std::string::npos;
  // is the file already compressed?
  m_compressed = filename.size() > 3 && filename.compare(filename.size() - 3, 3, ".gz") == 0;
  // strip .gz suffix to add it at the end automatically and correctly for subsequent files
  if (m_compressed) {
    m_filename = filename.substr(0, filename.size() - 3);
  }
  // check if we want different naming scheme using boost::format
  if (filenameIsPattern) {
    m_filenamePattern = m_filename;
    try {
      m_filename = (boost::format(m_filenamePattern) % m_nfile).str();
    } catch (std::exception& e) {
      B2FATAL("Cannot use filename pattern" << m_filenamePattern << ": " << e.what());
    }
  }

  // Store StreamerInfo 2017.5.8
  m_streamerinfo = NULL;
  m_streamerinfo_size = 0;
  if (streamerinfo != NULL && streamerinfo_size > 0) {
    m_streamerinfo_size = streamerinfo_size;
    m_streamerinfo = new char[ m_streamerinfo_size ];
    memcpy(m_streamerinfo, streamerinfo, m_streamerinfo_size);
  }

  // open the file
  openFile(m_filename, readonly);
  // if that fails and it's not already assumed to be compressed try again adding .gz to the name
  if (m_fd < 0 && !m_compressed) {
    B2WARNING("SeqFile: error opening '" << filename << "': " << strerror(errno)
              << ", trying again with '.gz'");
    m_compressed = true;
    openFile(m_filename, readonly);
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

  // add compression suffix if file is supposed to be compressed
  if (m_compressed) filename += ".gz";
  if (!readonly) {
    //open file in create mode and set stream correctly
    m_fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0664);
    auto filter = new io::filtering_ostream();
    if (m_compressed) filter->push(io::gzip_compressor());
    filter->push(io::file_descriptor_sink(m_fd, io::close_handle));
    filter->exceptions(ios_base::badbit | ios_base::failbit);
    m_stream.reset(filter);

    //
    // Write StreamerInfo  (2017.5.8)
    //
    if (m_streamerinfo == NULL || m_streamerinfo_size <= 0) {
      // If you want to use SeqFile for non-sroot file type, please skip this B2FATAL
      B2FATAL("Invalid size of StreamerInfo : " << m_streamerinfo_size << "bytes");
    } else {
      std::ostream* out = dynamic_cast<std::ostream*>(m_stream.get());
      if (!out) {
        B2FATAL("SeqFile::write() called on a file opened in read mode");
      }
      try {
        out->write(m_streamerinfo, m_streamerinfo_size);
        B2INFO("Wrote StreamerInfo at the begenning of the file. : " << m_streamerinfo_size << "bytes");
      } catch (ios_base::failure& e) {

        B2ERROR("SeqFile::openFile() error: " << e.what() << ", " << strerror(errno));
      }
    }

  } else {
    //open file in read mode and set stream correctly
    m_fd = open(filename.c_str(), O_RDONLY);
    auto filter = new io::filtering_istream();
    if (m_compressed) filter->push(io::gzip_decompressor());
    filter->push(io::file_descriptor_source(m_fd, io::close_handle));
    filter->exceptions(ios_base::badbit | ios_base::failbit);
    m_stream.reset(filter);
  }
  // reset number of written bytes (does not include streamerinfo )
  m_nb = 0;
}

SeqFile::~SeqFile()
{
  if (m_streamerinfo != NULL) delete m_streamerinfo;
  B2INFO("Closing SeqFile " << m_nfile);
  //closed automatically by m_stream.
}

int SeqFile::status() const
{
  return m_fd;
}

int SeqFile::write(const char* buf)
{
  // cast stream object
  std::ostream* out = dynamic_cast<std::ostream*>(m_stream.get());
  if (!out) {
    B2FATAL("SeqFile::write() called on a file opened in read mode");
  }
  int insize = *((int*)buf); // nbytes in the buffer at the beginning
  if (insize + m_nb >= c_MaxFileSize && m_filename != "/dev/null") {
    B2INFO("SeqFile: previous file closed (size=" << m_nb << " bytes)");
    m_nfile++;
    auto file = m_filename + '-' + std::to_string(m_nfile);
    if (!m_filenamePattern.empty()) {
      file = (boost::format(m_filenamePattern) % m_nfile).str();
    }
    openFile(file, false);
    if (m_fd < 0) {
      B2FATAL("SeqFile::write() error opening file '" << file << "': " << strerror(errno));
    }
    m_nb = 0;
    B2INFO("SeqFile::write() opened '" << file << "'");
    // update stream pointer since we reopened the file
    out = dynamic_cast<std::ostream*>(m_stream.get());
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
    if (!m_filenamePattern.empty()) {
      nextfile = (boost::format(m_filenamePattern) % m_nfile).str();
    }
    openFile(nextfile, true);
    if (m_fd < 0) return 0;   // End of all files
    // update the stream pointer
    in = dynamic_cast<std::istream*>(m_stream.get());
    B2INFO("SeqFile::read() opened '" << nextfile << "'");
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
