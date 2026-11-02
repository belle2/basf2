/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/pcore/SeqFile.h>
#include <cstdio>

namespace Belle2 {
  class HLTFile {
  public:
    bool open(const std::string& fileName, bool raw, const char* mode);
    virtual ~HLTFile();
    /** Default constructor. */
    HLTFile() = default;

    /** Copy constructor (disabled). */
    HLTFile(const HLTFile&) = delete;

    /** Operator = (disabled). */
    HLTFile& operator=(const HLTFile&) = delete;

    int put_wordbuf(const int* data, int len);
    int put(const char* data, int len);
    int get_wordbuf(int* data, int len);
    int get(char* data, int len);

  private:
    std::unique_ptr<SeqFile> m_sfile;
    FILE* m_rfile = nullptr;
  };
}






