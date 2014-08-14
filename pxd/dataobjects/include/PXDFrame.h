/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka, Zbynek Drasal, Martin Ritter            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDFRAME_H
#define PXDFRAME_H

#include <vxd/dataobjects/VxdID.h>

#include <TObject.h>

namespace Belle2 {

  /**
   * The PXD frame class.
   *
   * This is a small class that records information about PXD frames for each
   * half-ladder. The frame number is encoded in the sensor ID, similar to
   * other PXD objects.
   */
  class PXDFrame : public TObject {
  public:

    /** Constructor to use when the sensorID already contains frameNr information.
     * @param sensorID Sensor compact ID + frame number in segment field
     * @startRow Start row of the frame
     */
    PXDFrame(VxdID sensorID, unsigned short startRow):
      m_sensorID(sensorID), m_startRow(startRow)
    {}

    /** Constructor to use when frame number is supplied explicitly.
     * @param sensorID Sensor compact ID.
     * @param frameNr number of frame
     * @param startRow Start row of the frame
     */
    PXDFrame(VxdID sensorID, unsigned short frameNr, unsigned short startRow):
      PXDFrame(sensorID, startRow) {
      VxdID id(sensorID);
      id.setSegmentNumber(frameNr);
      m_sensorID = id;
    }

    /** Default constructor for the ROOT IO. */
    PXDFrame(): PXDFrame(0, 0) {}

    /** Get frame number of this frame.
     * @return frame number of the frame.
     */
    short int getFrameNumber() const { return VxdID(m_sensorID).getSegmentNumber(); }

    /** Get the sensor ID.
     * @return ID of the sensor.
     */
    VxdID getSensorID() const { return m_sensorID; }

    /** Get readout start row of the frame.
     * @return start row of the frame.
     */
    short int getStartRow() const { return m_startRow; }

  private:

    unsigned short m_sensorID; /**< Compressed sensor identifier.*/
    unsigned short m_startRow; /**< start row of the frame */

    ClassDef(PXDFrame, 1)

  }; // class PXDFrame


} // end namespace Belle2

#endif
