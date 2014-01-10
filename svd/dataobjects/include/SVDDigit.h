/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Peter Kvasnicka                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVD_DIGIT_H
#define SVD_DIGIT_H

#include <vxd/dataobjects/VxdID.h>

#include <root/TObject.h>

namespace Belle2 {
  /** \addtogroup dataobjects
   * @{
   */

  /**
   * The SVD digit class.
   *
   * The SVDDigit is an APV25 signal sample.
   * This is a development implementation which is intentionally kept
   * somewhat bulky. I record strip coordinates that won't be kept in future.
   * Also the sensor and cell IDs could be somewhat compressed, if desired.
   */

  class SVDDigit : public TObject {

  public:

    /** Useful Constructor.
     * @param sensorID Sensor compact ID.
     * @param isU True if v strip, false if v.
     * @param cellID Strip ID.
     * @param cellPosition Strip mid-line coordinate.
     * @param charge The charge collected on the strip.
     * @param time Index of the sample, e.g., 0 to 5.
     */
    SVDDigit(VxdID sensorID, bool isU, short cellID, float cellPosition,
             float charge, short index):
      m_sensorID(sensorID), m_isU(isU), m_cellID(cellID),
      m_cellPosition(cellPosition), m_charge(charge), m_index(index)
    {}

    /** Default constructor for the ROOT IO. */
    SVDDigit() : SVDDigit(0, true, 0, 0.0, 0.0, 0)
    {}

    /** Get the sensor ID.
     * @return ID of the sensor.
     */
    VxdID getSensorID() const { return m_sensorID; }

    /** Get strip direction.
     * @return true if u, false if v.
     */
    bool isUStrip() const { return m_isU; }

    /** Get strip ID.
     * @return ID of the strip.
     */
    short int getCellID() const { return m_cellID; }

    /** Get coordinate of strip midline.
     * @return coordinate of strip midline.
     */
    float getCellPosition() const { return m_cellPosition; }

    /** Get collected charge.
     * @return charge collected in the strip.
     */
    float getCharge() const { return m_charge; }

    /** Get time when the sample was taken.
     * @return time when the sample was taken.
     */
    short getIndex() const { return m_index; }

  private:

    unsigned short m_sensorID; /**< Compressed sensor identifier.*/
    bool m_isU;                /**< True if U, false if V. */
    short m_cellID;            /**< Strip coordinate in pitch units. */
    float m_cellPosition;      /**< Absolute strip position, -temporary-. */
    float m_charge;            /**< Strip signal. */
    short m_index;             /**< Index of the sample, e.g. 0 to 5. */

    ClassDef(SVDDigit, 2)

  }; // class SVDDigit

  /** @}*/

} // end namespace Belle2

#endif
