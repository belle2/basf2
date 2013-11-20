//+
// File : HistoAna.h
// Description : A test module of histogram functions
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 14 - Dec - 2012
//-

#ifndef HISTOANA_H
#define HISTOANA_H

#include <stdlib.h>
#include <string>
#include <vector>

#include <framework/core/HistoModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <generators/dataobjects/MCParticle.h>

#include "TH1F.h"


namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class HistoAnaModule : public HistoModule {

    // Public functions
  public:

    //! Constructor / Destructor
    HistoAnaModule();
    virtual ~HistoAnaModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun() {};
    virtual void event();
    virtual void endRun() {};
    virtual void terminate() {};

    //! Histogram Definitions
    virtual void defineHisto();

    // Data members
  private:
    // Histograms
    TH1F* h_multi;
    TH1F* h_mom[4];
    TH1F* h_vertex[3];


  };

} // end namespace Belle2

#endif // MODULEHELLO_H
