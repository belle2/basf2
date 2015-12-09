/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Eugenio Paoloni                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

//framework:
#include <framework/core/Module.h>

#include <vector>
#include <string>
#include <tuple>

namespace Belle2 {

  class VXDTFSecMap;
  class SectorMapConfig;

  /** The SegmentFilterConverterModule is a module able to read the
   *  cutoff values for filtering the Segments written in the
   *  xml gear box and write it in the new redesigned format
   */
  class SectorMapBootstrapModule : public Module {

  public:

    //! Constructor
    SectorMapBootstrapModule();

    //! Destructor
    virtual ~SectorMapBootstrapModule() { };

    virtual void initialize()   ;
    virtual void beginRun()     ;
    virtual void event()        ;
    virtual void endRun()       ;
    virtual void terminate() { };

  private:
    void bootstrapSectorMap(void);
    void bootstrapSectorMap(const SectorMapConfig& config);

  };
} // Belle2 namespace
