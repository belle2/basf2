/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#pragma once

#include <framework/core/Module.h>
#include <framework/pcore/DataStoreStreamer.h>
#include <framework/pcore/SeqFile.h>

#include <string>

#include <sys/time.h>

namespace Belle2 {
  /** Module to read files produced by SeqRootOutputModule. */
  class SeqRootInputModule : public Module {
  public:

    //! Constructor / Destructor
    SeqRootInputModule();
    virtual ~SeqRootInputModule();

    //! Module functions to be called from main process
    virtual void initialize() override;

    //! Module functions to be called from event process
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
    virtual void terminate() override;

  private:
    //! File name
    std::string m_inputFileName{""};
    //! List of all file names to read
    std::vector<std::string> m_filelist;

    //! If true the output filename will be interpreted as a boost::format pattern
    bool m_fileNameIsPattern{false};

    //! Is the input real data?
    bool m_realData{false};

    //! Blocked file handler
    SeqFile* m_file{nullptr};

    //! Number of files to read (aka m_filelist.size())
    int m_nfile{0};
    //! Index of current file in m_filelist
    int m_fileptr{0};

    //! Total nr. of events in the file
    int m_nevt{ -1};

    //! DataStoreStreamer
    DataStoreStreamer* m_streamer{nullptr};

    //! Time
    struct timeval m_t0; /**< time at begin of current run. */
    struct timeval m_tend; /**< time at end of current run. */

    // Data flow
    double m_size{0}; /**< total transferred data, in kB. */
    double m_size2{0}; /**< sum of squares of data transferred in each event, in kB^2. */
  };
} // end namespace Belle2
