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


  /** The class for VTX cluster position offset payload
    */

  class  VTXClusterOffsetPar: public TObject {
  public:
    /** Default constructor */
    VTXClusterOffsetPar():
      m_uOffset(0),  m_vOffset(0), m_uOffsetSigma2(0), m_vOffsetSigma2(0), m_uvCovariance(0)
    {}

    /** Constructor.
     * @param uOffset Offset in r-phi
     * @param vOffset Offset in z
     * @param uOffsetSigma2  Sigma squared for offset in r-phi
     * @param vOffsetSigma2  Sigma squared for offset in z
     * @param uvCovariance   Covariance between offsets in r-rphi and z
     */
    VTXClusterOffsetPar(float uOffset, float vOffset, float uOffsetSigma2, float vOffsetSigma2, float uvCovariance):
      m_uOffset(uOffset),  m_vOffset(vOffset), m_uOffsetSigma2(uOffsetSigma2), m_vOffsetSigma2(vOffsetSigma2),
      m_uvCovariance(uvCovariance)
    {}

    /** Destructor */
    ~ VTXClusterOffsetPar() {}

    /** Set offset in r-phi
     */
    void setU(float uOffset) { m_uOffset = uOffset; }

    /** Set v offset in z
     */
    void setV(float vOffset) { m_vOffset = vOffset; }

    /** Set sigma squared for offset in r-phi
     */
    void setUSigma2(float uOffsetSigma2) { m_uOffsetSigma2 = uOffsetSigma2; }

    /** Set sigma squared for offset in z
     */
    void setVSigma2(float vOffsetSigma2) { m_vOffsetSigma2 = vOffsetSigma2; }

    /** Set uv covariance
     */
    void setUVCovariance(float uvCovariance) { m_uvCovariance = uvCovariance; }

    /** Get offset in r-phi
     */
    float getU() const { return m_uOffset; }

    /** Get v offset in z
     */
    float getV() const { return m_vOffset; }

    /** Get sigma squared for offset in r-phi
     */
    float getUSigma2() const { return m_uOffsetSigma2; }

    /** Get sigma squared for offset in z
     */
    float getVSigma2() const { return m_vOffsetSigma2; }

    /** Get uv covariance
     */
    float getUVCovariance() const { return m_uvCovariance; }

  private:

    /** Positon offset in r-phi */
    float m_uOffset;
    /** Positon offset in z */
    float m_vOffset;
    /** Sigma squared for offset in r-phi */
    float m_uOffsetSigma2;
    /** Sigma squared for offset in z */
    float m_vOffsetSigma2;
    /** Covariance between offsets */
    float m_uvCovariance;


    ClassDef(VTXClusterOffsetPar, 2);    /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2
