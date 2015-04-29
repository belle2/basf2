/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PRESTFRAMELABTRANSFORM_H
#define PRESTFRAMELABTRANSFORM_H

#include <TLorentzRotation.h>
#include <TLorentzVector.h>

namespace Belle2 {

  /**
   * Class to hold Lorentz transformations from/to current rest frame and boost vector
   */
  class PRestFrameLabTransform {

  public:

    /**
     * Set current rest frame
     * @param boost boost vector
     */
    void setRestFrame(TVector3 boost)
    {
      m_lab2restframe = TLorentzRotation(-boost);
      m_restframe2lab = m_lab2restframe.Inverse();
    }

    /**
     * Set current rest frame
     * @param boost boost vector
     */
    void resetRestFrame()
    {
      m_lab2restframe = TLorentzRotation();
      m_restframe2lab = TLorentzRotation();
    }

    /**
     * Returns Lorentz transformation from CMS to Lab
     * @return const reference to Lorentz rotation matrix
     */
    const TLorentzRotation& rotateRestFrameToLab() const
    {
      return m_restframe2lab;
    }

    /**
     * Returns Lorentz transformation from Lab to CMS
     * @return const reference to Lorentz rotation matrix
     */
    const TLorentzRotation& rotateLabToRestFrame() const
    {
      return m_lab2restframe;
    }

    /**
     * Transforms Lorentz vector into rest frame System
     * @param vec Lorentz vector in Laboratory System
     * @return Lorentz vector in rest frame System
     */
    static TLorentzVector labToRestFrame(const TLorentzVector& vec);

    /**
     * Transforms Lorentz vector into Laboratory System
     * @param vec Lorentz vector in rest frame System
     * @return Lorentz vector in Laboratory System
     */
    static TLorentzVector restFrameToLab(const TLorentzVector& vec);

  private:
    static TLorentzRotation m_lab2restframe;  /**< from CMS to current rest frame */
    static TLorentzRotation m_restframe2lab;  /**< from rest frame to CMS */
  };

} // Belle2 namespace

#endif