/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TObject.h>

namespace Belle2 {


  /** The payload containing all parameters for the PXD and SVD CKF.
      Right now, it is only used for the layerJump parameter
      since this is the only parameter that is NOT constant for all exp/run numbers,
      but this class can be extended to store other parameters, too. */
  class CKFParameters: public TObject {
  public:
    /** Default constructor */
    CKFParameters() : m_layerJumpPtThreshold(-1.), m_layerJumpLowPt(1), m_layerJumpHighPt(1) {}
    /** Destructor */
    ~CKFParameters() {}

    /** Set if you want different parameter values for high/low pt tracks (-1 if not used) */
    void setLayerJumpPtThreshold(double layerJumpPtThreshold)
    {
      m_layerJumpPtThreshold = layerJumpPtThreshold;
    }

    /** Get the pt threshold that is used to determine if you want to use parameter for high/low pt tracks (-1 if not used) */
    double getLayerJumpPtThreshold() const
    {
      return m_layerJumpPtThreshold;
    }

    /** Set layerJump parameter for low momentum tracks */
    void setLayerJumpLowPt(int layerJumpLowPt)
    {
      m_layerJumpLowPt = layerJumpLowPt;
    }

    /** Get layerJump parameter for low momentum tracks */
    int getLayerJumpLowPt() const
    {
      return m_layerJumpLowPt;
    }

    /** Set layerJump parameter for high momentum tracks */
    void setLayerJumpHighPt(int layerJumpHighPt)
    {
      m_layerJumpHighPt = layerJumpHighPt;
    }

    /** Get layerJump parameter for high momentum tracks */
    int getLayerJumpHighPt() const
    {
      return m_layerJumpHighPt;
    }

  private:
    /** The pt threshold that is used to determine if you want to use parameter for high/low pt tracks (-1 if not used) */
    double m_layerJumpPtThreshold;
    /** The layerJump parameter for low momentum tracks */
    int m_layerJumpLowPt;
    /** The layerJump parameter for high momentum tracks */
    int m_layerJumpHighPt;

    ClassDef(CKFParameters, 1);  /**< ClassDef, necessary for ROOT */
  };
}
