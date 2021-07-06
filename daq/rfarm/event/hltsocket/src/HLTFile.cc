/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <daq/rfarm/event/hltsocket/HLTFile.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

bool HLTFile::open(const std::string& fileName, bool raw, const char* mode)
{
  if (m_rfile) {
    fclose(m_rfile);
  }

  if (raw) {
    m_rfile = std::fopen(fileName.c_str(), mode);
    if (!m_rfile) {
      B2ERROR("File opening failed! " << strerror(errno));
      return false;
    }
  } else {
    m_sfile.reset(new SeqFile(fileName, mode));
    if (m_sfile->status() <= 0) {
      B2ERROR("File opening failed! " << strerror(errno));
      return false;
    }
  }

  return true;
}

HLTFile::~HLTFile()
{
  if (m_rfile) {
    fclose(m_rfile);
  }
}

int HLTFile::put(char* data, int len)
{
  if (not m_sfile) {
    B2ERROR("Trying to write to a closed file");
  }
  const int bcount = m_sfile->write(data);
  if (bcount <= 0) {
    B2ERROR("Error in sending the data: " << strerror(errno));
    return bcount;
  }
  B2ASSERT("Written buffer size != buffer size in data!", bcount == len);
  return bcount;
}

int HLTFile::put_wordbuf(int* data, int len)
{
  if (not m_rfile) {
    B2ERROR("Trying to write to a closed file!");
    return -1;
  }

  const int gcount = data[0];
  B2ASSERT("The first entry in the data must be the buffer size!", gcount == len);

  int bcount = std::fwrite(data, sizeof(char), len * sizeof(int), m_rfile);
  if (bcount <= 0) {
    B2ERROR("Error in writing the data: " << strerror(errno));
    return bcount;
  }
  bcount = ((bcount - 1) / sizeof(int) + 1);

  B2ASSERT("Written buffer size != buffer size in data!" << bcount << " " << len, bcount == len);

  // ATTENTION: the returned size is size / 4
  return bcount;
}

int HLTFile::get(char* data, int len)
{
  const int bcount = m_sfile->read(data, len);
  if (bcount <= 0) {
    B2ERROR("Error in getting the data: " << strerror(errno));
    return bcount;
  }
  return bcount;
}

int HLTFile::get_wordbuf(int* wrdbuf, int len)
{
  if (not m_rfile) {
    B2ERROR("Trying to read from a closed file!");
    return -1;
  }
  int br = std::fread(wrdbuf, sizeof(int), 1, m_rfile);
  if (br <= 0) {
    if (std::feof(m_rfile)) {
      return 0;
    }
    B2ERROR("Error in getting the size: " << strerror(errno));
    return br;
  }

  const int gcount = (wrdbuf[0] - 1);
  if (gcount > len) {
    B2ERROR("buffer too small! " << gcount << " < " << len);
    return -1;
  }
  // ATTENTION: the send size is size / 4
  const int bcount = std::fread(&wrdbuf[1], sizeof(int), gcount, m_rfile);
  if (bcount <= 0) {
    if (std::feof(m_rfile)) {
      return 0;
    }
    B2ERROR("Error in getting the data: " << strerror(errno));
    return bcount;
  }

  B2ASSERT("Read buffer size != buffer size in data: " << bcount << " != " << gcount, bcount == gcount);
  return (wrdbuf[0]);
}
