/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vxd/dataobjects/VxdID.h>
#include <TObject.h>

namespace Belle2 {
  /** Namespace to provide code needed by both Vertex Detectors, PXD and SVD */


  /** Data containter to carry Sensor Information for PXD and SVD to/from the database..
   * The data are sensor type, sensor id, width/length/thickness, number of cells.
   */
  class VXDSensorInfoBasePar : public TObject {
  public:
    /** Enum specifing the type of sensor the SensorInfo represents */
    // FIXME: This is repeated, would be good to have only one instance.
    enum SensorType {
      PXD = 0,  /**< PXD Sensor */
      SVD = 1,  /**< SVD Sensor */
      TEL = 2,  /**< Testbeam telescope sensor */
      VXD = -1, /**< Any type of VXD Sensor */
    };

    //! Default constructor
    VXDSensorInfoBasePar() {}

    /** Constructor for a SensorInfo instance
     *
     * this class holds all the information to calculate pixel/strip
     * positions and ids. it can accomodate rectangular and trapezoidal
     * shapes and also has the possibility to segment the strip/pixelsize
     * once in v, thus having two different pixel sizes in one sensor: vcells
     * pixels from start to splitLength and vcells2 pixels from splitLength
     * to length.
     *
     * @param type Type of the Sensor, one of PXD or SVD
     * @param id   VxdID of the Sensor
     * @param width Width of the Sensor in default units
     * @param length Length of the Sensor in default units
     * @param thickness Thickness of the Sensor in default units
     * @param uCells Number of Pixels/Strips in u direction (local x)
     * @param vCells Number of Pixels/Strips in v direction (local y)
     * @param width2 Width of trapezoidal sensor on positive side, <=0 means rectangular
     * @param splitLength Length at which the pixel size changes, starting at
     *        0. <=0 means only one pixel/strip size in v
     * @param vCells2 Number of pixels in v direction after split length.
     */
    VXDSensorInfoBasePar(SensorType type, VxdID id, double width, double length, double thickness,
                         int uCells, int vCells, double width2 = 0, double splitLength = -1, int vCells2 = 0):
      m_type(type), m_id(id), m_width(width), m_length(length), m_thickness(thickness),
      m_width2(width2), m_splitLength(splitLength), m_uCells(uCells), m_vCells(vCells), m_vCells2(vCells2)
    {}

    /** Virtual destructor to make class polymorph */
    virtual ~VXDSensorInfoBasePar() {}

    // Getters: exactly 1 for each data member and nothing more.
    /** Return the Type of the Sensor */
    SensorType getType() const { return m_type; }
    /** Return the ID of the Sensor */
    VxdID getID() const { return m_id; }
    /** Return forward width for a slanted sensor */
    double getWidth2() const { return m_width2; }
    /** Return the (backward) width of the sensor */
    double getWidth() const { return m_width; }
    /** Return the length of the sensor */
    double getLength() const { return m_length; }
    /** Return the splitLength of the sensor */
    double getSplitLength() const { return m_splitLength; }
    /** Return the thickness of the sensor */
    double getThickness() const { return m_thickness; }
    /** Return number of pixel/strips in u direction */
    int getUCells() const { return m_uCells; }
    /** Return number of pixel/strips in v direction */
    int getVCells() const { return m_vCells + m_vCells2; }
    /** Return number of pixel/strips in v direction for first segment */
    int getVCells1() const { return m_vCells; }
    /** Return number of pixel/strips in v direction for second segment */
    int getVCells2() const { return m_vCells2; }


  protected:
    /** Type of the Sensor */
    SensorType m_type;
    /** ID of the Sensor */
    unsigned short m_id;
    /** Width of the sensor, backward width for wedge sensors */
    double m_width;
    /** Length of the Sensor */
    double m_length;
    /** Thickness of the Sensor */
    double m_thickness;
    /** Forward width for wedge sensors, 0 for rectangular sensors */
    double m_width2;
    /** Length at which second pixel size starts, 0 for only one pixel size */
    double m_splitLength;
    /** Number of strips/pixels in u direction */
    int m_uCells;
    /** Number of strips/pixels in v direction (up to splitLength for two pixel sizes) */
    int m_vCells;
    /** Number of strips/pixels in v direction after splitLength, 0 for only one pixel size */
    int m_vCells2;


    ClassDef(VXDSensorInfoBasePar, 6);  /**< ClassDef, must be the last term before the closing {}*/
  };

} //Belle2 namespace

