/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Vishal                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLSHOWERTOCLUSTERMODULE_H
#define ECLSHOWERTOCLUSTERMODULE_H

#include <framework/core/Module.h>

#include <vector>
#include <TRandom3.h>
#include <TMatrixFSym.h>




namespace Belle2 {
  namespace ECL {
    class ECLShowertoClusterModule : public Module {

    public:
      /** Constructor.
       */
      ECLShowertoClusterModule();

      /** Destructor.
       */
      ~ECLShowertoClusterModule();


      /** Initialize variables, print info, and start CPU clock. */
      virtual void initialize();

      /** Nothing so far.*/
      virtual void beginRun();

      /** Actual digitization of all hits in the ECL.
       *
       *  The digitized hits are written into the DataStore.
       */
      virtual void event();

      /** Nothing so far. */
      virtual void endRun();

      /** Stopping of CPU clock.*/
      virtual void terminate();

    protected:



    private:
      /** members of ECLbackwardCompt Module
       */
      /** CPU time     */
      double m_timeCPU;
      /** Run number   */
      int    m_nRun;
      /** Event number */
      int    m_nEvent;





    };
  }//ECL
}//Belle2

#endif





