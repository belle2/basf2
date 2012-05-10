/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jan Bauer                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCHOUGHTRACKINGMODULE_H
#define CDCHOUGHTRACKINGMODULE_H

#include <framework/core/Module.h>
#include <string>

namespace Belle2 {

  /** Module for pattern recognition of digitized CDC hits.
   *  Creates StoreArray of GFTrackCand Objects.
   *
   */

  class CDCHoughTrackingModule : public Module {

  public:

    CDCHoughTrackingModule();

    virtual ~CDCHoughTrackingModule();

    virtual void initialize();

    virtual void beginRun();

    virtual void event();

    virtual void endRun();

    virtual void terminate();

  private:
    std::string m_cdcDigiHitCollectionName;
    std::string m_gfTrackCandCollectionName;
    int m_precision;
    int m_zfinder;
    int m_pfinder;
    int m_pBins;
    int m_cBins;
  };

} // end namespace Belle2
#endif
