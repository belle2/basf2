/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors:                                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef TRACKINGDATABASEIMPORTER_H
#define TRACKINGDATABASEIMPORTER_H

#include <TObject.h>
namespace Belle2 {

  /*
   database importer.
   This module writes muidParameters and other data to database
  */
  class TrackingDatabaseImporter {
  public:

    /**
      * default constructor
     */
    TrackingDatabaseImporter();

    /**
     * Destructor
     */
    virtual ~TrackingDatabaseImporter() {};

    /**
     * Import MuidParameters to the database.
     */
    void importMuidParameters();

    /**
     * Export MuidParameters to the database.
     */
    void exportMuidParameters();

  private:

    /**< ClassDef */
    ClassDef(TrackingDatabaseImporter, 0);

  };


}

#endif




