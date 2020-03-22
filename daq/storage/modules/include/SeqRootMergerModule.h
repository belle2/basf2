//+
// File : SeqRootMerger.h
// Description : Module to receive data from outside and store it to DataStore
//
// Author : Tomoyuki Konno, Tokyo Metropolitan University
// Date : 16 - Oct - 2013
//-

#ifndef SEQROOTMERGER_H
#define SEQROOTMERGER_H

#include <framework/core/Module.h>
#include <framework/pcore/DataStoreStreamer.h>
#include <framework/pcore/SeqFile.h>

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */
  class SeqRootMergerModule : public Module {

    // Public functions
  public:
    //! Constructor / Destructor
    SeqRootMergerModule();
    virtual ~SeqRootMergerModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

  private:
    int readFile();

  private:
    std::vector<SeqFile*> m_file;
    DataStoreStreamer* m_streamer;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
