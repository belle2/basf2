/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <svd/reconstruction/SVDReconstructionBase.h>
#include <svd/calibration/SVDCoGTimeCalibrations.h>
#include <svd/calibration/SVD3SampleCoGTimeCalibrations.h>
#include <svd/calibration/SVD3SampleELSTimeCalibrations.h>
#include <vector>

namespace Belle2 {

  namespace SVD {

    /**
     * Class for SVD Time computation
     */
    class SVDTimeReconstruction : public SVDReconstructionBase {

    public:

      /**
       * set the trigger bin
       */
      void setTriggerBin(const int triggerBin)
      { m_triggerBin = triggerBin; };

      /**
       * virtual destructor
       */
      virtual ~SVDTimeReconstruction() {};

      /** get strip time as set in SVDRecoConfiguration payload*/
      double getStripTime();
      /** get strip time error as set in SVDRecoConfiguration payload*/
      double getStripTimeError();

      /**get first frame*/
      int getFirstFrame() { return m_firstFrame; };

      /** CoG6 Time */
      double getCoG6Time();
      double getCoG6TimeError();
      /** CoG3 Time */
      double getCoG3Time() {return 0;}
      double getCoG3TimeError() {return 0;}
      /** ELS3 Time */
      double getELS3Time() {return 0;}
      double getELS3TimeError() {return 0;}

    protected:

      /** trigger bin */
      int m_triggerBin = -1;

      /** first frame */
      int m_firstFrame = -1;

      /** CoG6 time calibration wrapper*/
      SVDCoGTimeCalibrations m_CoG6TimeCal;
      /** CoG3 time calibration wrapper*/
      SVD3SampleCoGTimeCalibrations m_CoG3TimeCal;
      /** ELS3 time calibration wrapper*/
      SVD3SampleELSTimeCalibrations m_ELS3TimeCal;

    };

  }

}

