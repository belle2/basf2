/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCTRACKINGMODULE_H
#define CDCTRACKINGMODULE_H

#include <framework/core/Module.h>

namespace Belle2 {

  /** Make Track-CDCHit Relation from all hits in the CDC.
   */

  class CDCDummyModule : public Module {

  public:

    /** Constructor.
     */

    CDCDummyModule();

    /** Destructor.
     */
    virtual ~CDCDummyModule();

    /** Initialize the Module.
     */

    virtual void initialize();

    /** Called when entering a new run;
     */

    virtual void beginRun();

    /** Form the damn Relation.
     */

    virtual void event();

    /** This method is called if the current run ends.
     */

    virtual void endRun();

    /** This method is called at the end of the event processing.
     */

    virtual void terminate();

  protected:


  private:

    std::string m_cdcRecoHitCollectionName;
    std::string m_trackToCDCRecoHitCollectionName;

  };
} // end namespace Belle2
#endif

