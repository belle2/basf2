/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVD_DIGIT_H
#define SVD_DIGIT_H

#include <vxd/dataobjects/VxdID.h>
#include <framework/dataobjects/DigitBase.h>

#include <sstream>
#include <string>

namespace Belle2 {

  /**
   * The SVD digit class.
   *
   * The SVDDigit is an APV25 signal sample.
   * This is a development implementation which is intentionally kept
   * somewhat bulky. I record strip coordinates that won't be kept in future.
   * Also the sensor and cell IDs could be somewhat compressed, if desired.
   */

  class SVDDigit : public DigitBase {

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
             float charge, short index, short parent = -1):
      m_sensorID(sensorID), m_isU(isU), m_cellID(cellID),
      m_cellPosition(cellPosition), m_charge(charge), m_index(index), m_parentTpDigit(parent)
    {
      m_time = 0.0;
      m_prev_id = -1;
      m_next_id = -1;
    }

    /** Default constructor for the ROOT IO. */
    SVDDigit() : SVDDigit(0, true, 0, 0.0, 0.0, 0, -1)
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

    /** Get index of the sample
     * @return index of the sample
     */
    short getIndex() const { return m_index; }

    /** Get time when the sample was taken.
     * @return time when the sample was taken.
     */
    short getTime() const { return m_time; }

    /** Get previous charge.
     * @return charge of the previous sample
     */
    short getPrevID() const { return m_prev_id; }

    /** Get next charge.
     * @return charge of the next sample
     */
    short getNextID() const { return m_next_id; }

    /** Set time when the sample was taken.
     * @param time when the sample was taken.
     */
    void setTime(float time) { m_time = time; }

    /** Set previous charge.
     * @param charge of the previous sample
     */
    void setPrevID(short prev_id) { m_prev_id = prev_id; }

    /** Set next charge.
     * @param charge of the next sample
     */
    void setNextID(short next_id) { m_next_id = next_id; }

    /** Get parent SVDTransparentDigit index.
     * @return: Index value of SVDTransparentDigit object
     *        : in DataArray which created this SVDDigit.
     *        : If it is not created from SVDTransparentDigit,
     *        : return -1.
     */
    short getParentTpIndex() const { return m_parentTpDigit; }

    /** Display main parameters in this object
     */
    std::string toString() const
    {
      VxdID thisSensorID = static_cast<VxdID>(m_sensorID);
      VxdID::baseType layer   = thisSensorID.getLayerNumber();
      VxdID::baseType ladder  = thisSensorID.getLadderNumber();
      VxdID::baseType sensor  = thisSensorID.getSensorNumber();
      VxdID::baseType segment = thisSensorID.getSegmentNumber();

      std::ostringstream os;
      os << "VXDID:" << m_sensorID << "=" << layer << "/" << ladder
         << "/" << sensor << "#" << segment << " strip: "
         << ((m_isU) ? "U-" : "V-") << m_cellID << " sample " << m_index
         << "/" << m_charge << std::endl;

      return os.str();
    }

    /**
    * Implementation of the base class function.
    * Enables BG overlay module to identify uniquely the physical channel of this Digit.
    * @return unique channel ID, composed of VxdID (1 - 16), strip side (17), strip number (18-28) and sample number 29-32).
    */
    unsigned int getUniqueChannelID() const
    { return m_index + (m_cellID << 4) + ((m_isU ? 1 : 0) << 15) + (m_sensorID << 16); }
    /**
    * Implementation of the base class function.
    * Pile-up method.
    * @param bg BG digit
    * @return append status
    */
    DigitBase::EAppendStatus addBGDigit(const DigitBase* bg)
    {
      m_charge += dynamic_cast<const SVDDigit*>(bg)->getCharge();
      return DigitBase::c_DontAppend;
    }

  private:

    unsigned short m_sensorID; /**< Compressed sensor identifier.*/
    bool m_isU;                /**< True if U, false if V. */
    short m_cellID;            /**< Strip coordinate in pitch units. */
    float m_cellPosition;      /**< Absolute strip position, -temporary-. */
    float m_charge;            /**< Strip signal. */
    short m_index;             /**< Index of the sample, e.g. 0 to 5. */
    float m_time;             /**< Time at the sample. */
    short m_parentTpDigit; /**< Index of the SVDTransparentDigit creating this SVDDigit. This index is valid only if this SVDDigit is produced with transparent data, otherwise this value is -1. */

    short m_prev_id;     /**< SVDDigit index in StoreArray of previous sample. */
    short m_next_id;     /**< SVDDigit index in StoreArray of next sample. */

    ClassDef(SVDDigit, 5)

  }; // class SVDDigit


} // end namespace Belle2

#endif
