//+
// File : TrackAnaModule.h
// Description : Module to check reconstructed tracks
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 29 - Jul - 2016
//-

#ifndef TRACKANAMODULE_H
#define TRACKANAMODULE_H

#include <framework/core/HistoModule.h>

#include "TH1F.h"

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class TrackAnaModule : public HistoModule {

    // Public functions
  public:

    //! Constructor / Destructor
    TrackAnaModule();
    virtual ~TrackAnaModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    //! Module funcions to define histograms
    virtual void defineHisto();

    // Data members
  private:
    TH1F* h_multi;
    TH1F* h_p[4];
    TH1F* h_e;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
