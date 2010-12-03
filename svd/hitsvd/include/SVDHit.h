/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDHIT_H
#define SVDHIT_H

#include <pxd/hitpxd/CIDManager.h>

#include <TObject.h>

namespace Belle2 {

  /*
   ** The lean hit class for SVD.
   *
   * The current implementation is not in fact lean, as some necessary
   * normalizations are unclear: normalizations of coordinates and their
   * (co-)variance, and of energy and its error.
   *
   * A first attempt is made to create a lean sensor identifier storing
   * layer/ladder/sensor info. To keep this hit class free of additonal members,
   * the coding/decoding is done via a separate class CIDManager.
   * Just
   *    #include <pxd/hitpxd/CIDManager.h>,
   * and, to decode, do
   *    CIDManager cid(hit.getSensorCID());
   *    int layerID = cid.getLayerID(); etc.,
   * or, to encode, do
   *    CIDManager cid();
   *    cid.setLayerID(myLayerID); etc.,
   *    short int myCID = cid.getCID();
   *
   * For normalization of spatial data, a precision of 1 um should be
   * satisfactory (expected resolutions are ~4 um and more). A 4-byte
   * integer would be more than enough (but a 2-byte int will not). For energy,
   * the normalization depends on the ADCs, but a one or two bytes will
   * be enough. In summary, the target value (exluding TObject) should be
   * 2*4 + 3*4 (coordinates and errors) + 2*2 (energy and errors) + 2 (CID)
   * that is, 26 bytes per lean hit.
   */

  class SVDHit : public TObject {

  public:

    /*
     ** Default constructor for ROOT IO.
     */
    SVDHit() {;}

    /*
     ** Useful Constructor.
     */
    SVDHit(int sensorCID,
           float u,
           float uError,
           float v,
           float vError,
           float uvCov,
           float energyDep,
           float energyDepError):
        m_sensorCID(sensorCID),
        m_u(u), m_uError(uError),
        m_v(v), m_vError(vError),
        m_uvCov(uvCov),
        m_energyDep(energyDep),
        m_energyDepError(energyDepError) {
      /* no action */
    }

    /** Setters.*/

    /** Set compressed layer/ladder/sensor id.*/
    void setSensorCID(int CID) { m_sensorCID = CID; }

    /** Set u coordinate.*/
    void setU(float u) { m_u = u; }

    /** Set u coordinate error.*/
    void setUError(float uError) { m_uError = uError; }

    /** Set v coordinate.*/
    void setV(float v) { m_v = v; }

    /** Set v coordinate error.*/
    void setVError(float vError) { m_vError = vError; }

    /** Set u-v error covariance.*/
    void setUVCov(float uvCov) { m_uvCov = uvCov; }

    /** Set deposited energy.*/
    void setEnergyDep(float energyDep) { m_energyDep = energyDep; }

    /** Set deposited energy error.*/
    void setEnergyDepError(float energyDepError)
    { m_energyDepError = energyDepError; }

    /** Getters.*/

    /** Get the compact ID.*/
    int getSensorCID() const { return m_sensorCID; }

    /** Get u coordinate.*/
    float getU() const { return m_u; }

    /** Get u coordinate error.*/
    float getUError() const { return m_uError; }

    /** Get v coordinate.*/
    float getV() const { return m_v; }

    /** Get v coordinate error.*/
    float getVError() const { return m_vError; }

    /** Get u-v error covariance.*/
    float getUVCov() const { return m_uvCov; }

    /** Get deposited energy. */
    float getEnergyDep() const { return m_energyDep; }

    /** Get deposited energy error. */
    float getEnergyDepError() const { return m_energyDepError; }

  private:

    int m_sensorCID;                /**< Compressed sensor identifier.*/
    float m_u;                      /**< u (azimuthal) coordinate of the hit.*/
    float m_uError;                 /**< u coordinate error.*/
    float m_v;                      /**< v (axial) coordinate of the hit.*/
    float m_vError;                 /**< v coordinate error.*/
    float m_uvCov;                  /**< u-v error covariance.*/
    float m_energyDep;              /**< deposited energy.*/
    float m_energyDepError;         /**< error in dep. energy.*/

    ClassDef(SVDHit, 1);

  };

} // end namespace Belle2

#endif
