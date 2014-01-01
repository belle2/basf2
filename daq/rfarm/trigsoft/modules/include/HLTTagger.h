//+
// File : HLTTagger.h
// Description : Module to create HLTTag info
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 31 - Dec - 2013
//-

#ifndef HLTTAGGERMODULE_H
#define HLTTAGGERMODULE_H

#include <string>
#include <vector>

#include <framework/core/Module.h>
#include <rawdata/dataobjects/HLTTag.h>


namespace Belle2 {

  /*! A class definition of HLTTagger module */

  class HLTTaggerModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    HLTTaggerModule();
    virtual ~HLTTaggerModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    // Data members
  private:

    //! No. of HLT events
    int m_nevt;

    //! HLT Unit ID
    int m_hltunit;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
