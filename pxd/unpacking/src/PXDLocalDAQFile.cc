//+
// File : PXDLocalDAQFile.cc
// Description : Sequential file I/O privitives with file blocking
//
// Author : Bjoern Spruck/ Maiko Takahashi
// Date : 2020
//-

#include <pxd/unpacking/PXDLocalDAQFile.h>
#include <framework/logging/Logger.h>

#include <ios>
#include <fcntl.h>
#include <cerrno>

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/format.hpp>

using namespace Belle2;
using namespace std;
namespace io = boost::iostreams;

PXDLocalDAQFile::PXDLocalDAQFile(const std::string& filename):
  m_filename(filename)
{
  if (filename.empty()) {
    B2ERROR("PXDLocalDAQFile: Empty filename given");
    return;
  }
  // is the file already compressed?
  m_compressed = filename.size() > 4 && filename.compare(filename.size() - 4, 4, ".bz2") == 0;

  // open the file
  openFile(m_filename);
  if (m_fd < 0 && !m_compressed) {
    B2WARNING("PXDLocalDAQFile: error opening '" << filename << "': " << strerror(errno)
              << ", trying again with '.bz2'");
    m_compressed = true;
    openFile(m_filename);
  }
  // is the file open now?
  if (m_fd < 0) {
    B2ERROR("PXDLocalDAQFile: error opening '" << filename << "': " << strerror(errno));
  } else {
    B2INFO("PXDLocalDAQFile: " << m_filename << " opened (fd=" << m_fd << ")");
  }

}

void PXDLocalDAQFile::openFile(std::string filename)
{

  //open file in read mode and set stream correctly
  m_fd = open(filename.c_str(), O_RDONLY);
  auto* filter = new io::filtering_istream();
  if (m_compressed) filter->push(io::bzip2_decompressor());
  filter->push(io::file_descriptor_source(m_fd, io::close_handle));
  filter->exceptions(ios_base::badbit | ios_base::failbit);
  m_stream.reset(filter);

}

PXDLocalDAQFile::~PXDLocalDAQFile()
{
  B2INFO("Closing PXDLocalDAQFile ");
  //closed automatically by m_stream.
}

int PXDLocalDAQFile::status() const
{
  return m_fd;
}

int PXDLocalDAQFile::read(char* buf, int size)
{
  // cast stream object
  auto in = dynamic_cast<std::istream*>(m_stream.get());
  if (!in) {
    B2FATAL("PXDLocalDAQFile::read() cannot get input file");
  }
  //trigger eof if there's nothing left int the file. Could throw an error on decompress failure
  try {
    in->peek();
  } catch (ios_base::failure& e) {
    B2ERROR("PXDLocalDAQFile::read() cannot read file: " << e.what());
    return -1;
  }
  // return -1 if eof
  if (in->eof()) {
    B2DEBUG(29, "PXDLocalDAQFile::read() eof");
    return -1;
  }

  // read from file
  try {
    in->read(buf, size);
  } catch (ios_base::failure& e) {
    B2ERROR("PXDLocalDAQFile::read() " << e.what() << ": could only read " << in->gcount() << " bytes, expected " << size);
    return -1;
  }

  return in->gcount();
}

int PXDLocalDAQFile::read_data(char* data, size_t len)
{
  size_t l = read(data, len);
  if (l != len) return 0;
  return l;
}

