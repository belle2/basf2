/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef PARTIALSEQROOTREADER_H
#define PARTIALSEQROOTREADER_H

#include <framework/core/Module.h>
#include <framework/pcore/DataStoreStreamer.h>

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */
  class PartialSeqRootReaderModule : public Module {

    // Public functions
  public:
    //! Constructor / Destructor
    PartialSeqRootReaderModule();
    virtual ~PartialSeqRootReaderModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

  private:
    int openFile(int fileno);
    int readFile();

  private:
    std::string m_path;
    int m_fd;
    int m_filemin;
    int m_filemax;
    char* m_buf;
    int m_fileno;
    DataStoreStreamer* m_streamer;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
