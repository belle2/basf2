/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
/* Additional Info:
* This Module is in an early stage of developement. The comments are mainly for temporal purposes
* and will be changed and corrected in later stages of developement. So please ignore them.
*/

#ifndef PXD_DATA_REDUCTION_MODULE_H_
#define PXD_DATA_REDUCTION_MODULE_H_

#include <framework/core/Module.h>
#include <tracking/pxdDataReductionClasses/PXDInterceptor.h>
#include <tracking/pxdDataReductionClasses/ROIPixelTranslator.h>
#include <string>
#include <TTree.h>
#include <TFile.h>
#include <time.h>

namespace Belle2 {

  /** The PXD Data Reduction Module
   *
   * this module performs the reduction of the PXD data output. A list of tracks (reconstructed using SVD hits) is loaded from the event and then each track is extrapolated on the PXD plans; for each interception, a region around the predicted intersection is identified and the list of pixels belonging to that region is produced
   *
   */

  class PXDDataReductionModule : public Module {

  public:

    /**
     * Constructor of the module.
     */
    PXDDataReductionModule();

    /**
     * Destructor of the module.
     */
    virtual ~PXDDataReductionModule();

    /**
     *Initializes the Module.
     */
    virtual void initialize();

    virtual void beginRun();

    virtual void event();

    virtual void endRun();

    /**
     * Termination action.
     */
    virtual void terminate();

  protected:

    ROIPixelTranslator* thePixelTranslator;
    PXDInterceptor* thePXDInterceptor;
    std::string m_gfTrackCandsColName;
    std::string m_PXDInterceptListName;
    std::string m_ROIListName;

    int m_numIterKalmanFilter; /**< number of iterations of the Kalman Filter*/

    double m_sigmaSystU;
    double m_sigmaSystV;
    double m_numSigmaTotU;
    double m_numSigmaTotV;
    double m_maxWidthU;
    double m_maxWidthV;

  private:

  };
}

#endif /* PXDDataReductionModule_H_ */
