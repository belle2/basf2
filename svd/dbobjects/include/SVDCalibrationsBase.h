/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <framework/logging/Logger.h>

#include <TObject.h>
#include <vector>
#include <TString.h>

namespace Belle2 {

  /** base class for calibrations classes*/
  template < class T  >
  class SVDCalibrationsBase: public TObject {

    /** T::payloadContainerType can be a vector whose length is the
     * number of strips per side or a list of defect on a given side
     * This vector will have length 2.
     * Index 0 for the V side, index 1 for the U side
     * Please, please, pleaseeeee use SVDCalibrationBase<...>::UIndex
     * and SVDCalibrationBase<...>::VIndex instead of  1 and 0 for better
     * code readibility
     */
    typedef std::vector< typename T::payloadContainerType > SVDSensor;

    /**An SVDLadder is a vector of SVDSensors */
    typedef std::vector< SVDSensor > SVDLadder;

    /** An SVDLayer is a vector of SVDLadders */
    typedef std::vector< SVDLadder > SVDLayer;

    /** The SVD is a vector of SVDLayers */
    typedef std::vector< SVDLayer > SVD;

    /** an SVD calibration*/
    SVD calibrations;


  public:

    /** This enumeration assure the same semantic of the
    isU methods defined by Peter Kv.*/
    enum E_side { Vindex = 0, Uindex = 1 };

    /** The default constructor initialize all the vectors
     */
    SVDCalibrationsBase(typename T::calibrationType defaultT =
                          typename T::calibrationType(),
                        const TString& uniqueID = ""): m_uniqueID(uniqueID)    // Add a string as unique identifier for a given configuration dataset
    {

      calibrations.resize(7); // Layers 0 1 2 3 4 5 6
      int laddersOnLayer[] = { 0, 0, 0, 8, 11, 13, 17 };
      for (unsigned int layer = 0 ; layer < calibrations.size() ; layer ++) {
        calibrations[layer].resize(laddersOnLayer[layer]);
        int sensorsOnLadder[] = {0, 0, 0, 3, 4, 5, 6};
        for (unsigned int ladder = 1; ladder < calibrations[layer].size(); ladder ++) {
          calibrations[layer][ladder].resize(sensorsOnLadder[ layer ]);
          for (unsigned int sensor = 1; sensor < calibrations[layer][ladder].size() ; sensor ++) {
            calibrations[layer][ladder][sensor].resize(2);
            T::init(calibrations[layer][ladder][sensor][ Uindex ], layer, ladder, sensor, Uindex,
                    defaultT);
            T::init(calibrations[layer][ladder][sensor][ Vindex ], layer, ladder, sensor, Vindex,
                    defaultT);
          }
        }
      }
    }

    /** Simple destructor
     */
    ~SVDCalibrationsBase() {  };

    /**
     * Return the calibration associated to a given strip.
     * Return the calibration associated to a given strip identified by:
     * @param layer is the layer number (2 to 6 in the present geometry)
     * @param ladder is the ladder number ( 1 to 7 for layer 3, 1 to 10 for layer 4 etc...)
     * @param sensor is the sensor number ( 1 to 2 for layer 3, 1 to 3 for layer 4 etc...)
     * @param side is the sensor view: 1 for Side U ( Side P ), 0 for side V (Side N)
     * @param strip is the strip number: from 1 to 512 or 768 depending on the sensor
     * it throws std::out_of_range if the strip is unknown
     */
    typename T::calibrationType get(unsigned int layer,
                                    unsigned int ladder,
                                    unsigned int sensor,
                                    unsigned int side,
                                    unsigned int strip) const
    {
      if (calibrations.size() <= layer) {
        B2FATAL("Layers vector is smaller than " << layer);
      }
      const auto& ladders = calibrations[layer];
      if (ladders.size() <= ladder) {
        B2FATAL("Ladders vector is smaller than " << ladder);
      }
      const auto& sensors = ladders[ladder];
      if (sensors.size() <= sensor) {
        B2FATAL("Sensors vector is smaller than " << sensor);
      }
      const auto& sides = sensors[sensor];
      if (sides.size() <= side) {
        B2FATAL("Sides vector is smaller than " << side);
      }

      return T::get(sides[side], strip);
    }

    /**
     * Return a reference to the calibration associated to a given strip.
     * Return a reference to the calibration associated to a given strip identified by:
     * @param layer is the layer number (2 to 6 in the present geometry)
     * @param ladder is the ladder number ( 1 to 7 for layer 3, 1 to 10 for layer 4 etc...)
     * @param sensor is the sensor number ( 1 to 2 for layer 3, 1 to 3 for layer 4 etc...)
     * @param side is the sensor view: 1 for Side U ( Side P ), 0 for side V (Side N)
     * @param strip is the strip number: from 1 to 512 or 768 depending on the sensor
     * it throws std::out_of_range if the strip is unknown
     */
    const typename T::calibrationType& getReference(unsigned int layer,
                                                    unsigned int ladder,
                                                    unsigned int sensor,
                                                    unsigned int side,
                                                    unsigned int strip) const
    {
      if (calibrations.size() <= layer) {
        B2FATAL("Layers vector is smaller than " << layer);
      }
      const auto& ladders = calibrations[layer];
      if (ladders.size() <= ladder) {
        B2FATAL("Ladders vector is smaller than " << ladder);
      }
      const auto& sensors = ladders[ladder];
      if (sensors.size() <= sensor) {
        B2FATAL("Sensors vector is smaller than " << sensor);
      }
      const auto& sides = sensors[sensor];
      if (sides.size() <= side) {
        B2FATAL("Sides vector is smaller than " << side);
      }

      return T::getReference(sides[side], strip);
    }

    /**
     * Get the unique ID  of the calibration
     */
    TString get_uniqueID() const {return m_uniqueID;}

    /**
     * Set the calibration associated to a given strip.
     * Return the calibration associated to a given strip identified by:
     * @param layer is the layer number (2 to 6 in the present geometry)
     * @param ladder is the ladder number ( 1 to 7 for layer 3, 1 to 10 for layer 4 etc...)
     * @param sensor is the sensor number ( 1 to 2 for layer 3, 1 to 3 for layer 4 etc...)
     * @param side is the sensor view: 1 for Side U ( Side P ), 0 for side V (Side N)
     * @param strip is the strip number: from 1 to 512 or 768 depending on the sensor
     * @param value
     * it throws std::out_of_range if the strip is unknown
     */
    void set(unsigned int layer,
             unsigned int ladder,
             unsigned int sensor,
             unsigned int side,
             unsigned int strip,
             typename T::calibrationType value)
    {
      if (calibrations.size() <= layer) {
        B2FATAL("Layers vector is smaller than " << layer);
      }
      auto& ladders = calibrations[layer];
      if (ladders.size() <= ladder) {
        B2FATAL("Ladders vector is smaller than " << ladder);
      }
      auto& sensors = ladders[ladder];
      if (sensors.size() <= sensor) {
        B2FATAL("Sensors vector is smaller than " << sensor);
      }
      auto& sides = sensors[sensor];
      if (sides.size() <= side) {
        B2FATAL("Sides vector is smaller than " << side);
      }

      return T::set(sides[side], strip, value);
    }

    /**
     * Return the array index of the side
     * @param isU
     *
     */
    E_side sideIndex(bool isU) const
    {
      if (isU)
        return Uindex;
      return Vindex;
      // tertium non datur
    }

    typedef T t_perSideContainer; /**< typedef of the container of each side*/
  private:

    TString m_uniqueID; /**<The unique identifier is a private member of SVDCalibrationsBase, whose value is assigned in the constructor.*/

    ClassDef(SVDCalibrationsBase, 2) /**< needed by root*/
  };

}
