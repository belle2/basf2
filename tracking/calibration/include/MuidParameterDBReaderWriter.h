/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Yinghui Guan                                             *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/
#pragma once

#include <TObject.h>
namespace Belle2 {

  /**
   This class writes muidParameters to database and read muidParameters from database
  */
  class MuidParameterDBReaderWriter {
  public:

    /**
     * default constructor
     */
    MuidParameterDBReaderWriter() = default;

    /**
     * Destructor
     */
    ~MuidParameterDBReaderWriter() = default;

    /**
     * Write MuidParameters to the database.
     */
    void writeMuidParameters();

    /**
     * Read MuidParameters from the database.
     */
    void readMuidParameters();

  private:

    /**< ClassDef */
    ClassDef(MuidParameterDBReaderWriter, 0);

  };


}

