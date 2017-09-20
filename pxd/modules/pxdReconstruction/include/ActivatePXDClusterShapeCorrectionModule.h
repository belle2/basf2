/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * Prepared for cluster shape correction quality check                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ActivatePXDClusterShapeCorrectionMODULE_H_
#define ActivatePXDClusterShapeCorrectionMODULE_H_

#include <framework/core/Module.h>
#include <pxd/reconstruction/HitCorrector.h>

namespace Belle2 {

  namespace PXD {

    /** PXD DQM Cluster Shape Module */
    class ActivatePXDClusterShapeCorrectionModule : public Module {

    public:

      /** Constructor */
      ActivatePXDClusterShapeCorrectionModule();
      /* Destructor */
      virtual ~ActivatePXDClusterShapeCorrectionModule();

      /** Module functions */
      virtual void initialize();
      virtual void beginRun();
      virtual void event();
      virtual void endRun();
      virtual void terminate();

    private:

      std::string m_ClShCorrectionFileName;     /**< Name of file with corrections for Cluster Shapes */
      std::string m_ClShCorrectionLogFileName;  /**< Name of log file for corrections for Cluster Shapes */


    };  //end class declaration

  }  //end PXD namespace;

}  // end namespace Belle2

#endif  // ActivatePXDClusterShapeCorrectionMODULE_H_

