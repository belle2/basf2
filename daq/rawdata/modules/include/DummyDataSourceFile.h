/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef DUMMYDATASOURCEFILE_H
#define DUMMYDATASOURCEFILE_H

#include <daq/rawdata/modules/DummyDataSource.h>


namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class DummyDataSourceFileModule : public DummyDataSourceModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DummyDataSourceFileModule();
    virtual ~DummyDataSourceFileModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void event();


  protected :


  };

} // end namespace Belle2

#endif // MODULEHELLO_H
