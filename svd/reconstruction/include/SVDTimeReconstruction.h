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
#include <vector>

namespace Belle2 {

  namespace SVD {

    /**
     * Class for SVD Time computation
     */
    class SVDTimeReconstruction : public SVDReconstructionBase {

    public:

      /**
       * all constructors implemented in SVDReconstructionBase are available
       */
      template<class ... T> explicit SVDTimeReconstruction(T ... args): SVDReconstructionBase(args ...) {};

      /**
       * virtual destructor
       */
      virtual ~SVDTimeReconstruction() {};

      /** get first frame and strip time as set in SVDRecoConfiguration payload if timeAlgo = inRecoDBObject*/
      std::pair<int, double> getFirstFrameAndStripTime(const TString& timeAlgo);
      /** get strip time error as set in SVDRecoConfiguration payload if timeAlgo = inRecoDBObject*/
      double getStripTimeError(const TString& timeAlgo);

      /** CoG6 Time */
      double getCoG6Time();
      /** CoG6 Time error*/
      double getCoG6TimeError();
      /** CoG3 First Frame and Time */
      std::pair<int, double> getCoG3FirstFrameAndTime();
      /** CoG3 Time error*/
      double getCoG3TimeError();
      /** ELS3 First Frame Time */
      std::pair<int, double> getELS3FirstFrameAndTime();
      /** ELS3 Time error */
      double getELS3TimeError();

    };

  }

}

