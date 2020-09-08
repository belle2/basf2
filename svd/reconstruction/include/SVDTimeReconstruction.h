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

      /** get strip time as set in SVDRecoConfiguration payload if timeAlgo = inRecoDBObjcte*/
      double getStripTime(TString timeAlgo);
      /** get strip time error as set in SVDRecoConfiguration payload if timeAlgo = inRecoDBObjcte*/
      double getStripTimeError(TString timeAlgo);

      /**get first frame*/
      int getFirstFrame()  { return m_firstFrame; };

      /** CoG6 Time */
      double getCoG6Time();
      double getCoG6TimeError();
      /** CoG3 Time */
      double getCoG3Time();
      double getCoG3TimeError();
      /** ELS3 Time */
      double getELS3Time();
      double getELS3TimeError();

    };

  }

}

