/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Laura Zani, Eugenio Paoloni                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 * WARNING EXPLOSIVE: PLEASE, WEAR PROTECTIVE SUIT BEFORE USE             *
 **************************************************************************/

#pragma once
#include <TObject.h>
#include <vector>
#include <TString.h>
namespace Belle2 {

  template < class T  >
  class SVDCalibrationsBase: public TObject {

    // T::payloadContainerType can be a vector whose length is the
    // number of strips per side or a list of defect on a given side
    // This vector will have length 2.
    // Index 0 for the V side, index 1 for the U side
    typedef std::vector< typename T::payloadContainerType > SVDSensor;

    // An SVDLAdder is a vector of SVDSensors
    typedef std::vector< SVDSensor > SVDLadder;

    // An SVDLayer is a vector of SVDLAdders
    typedef std::vector< SVDLadder > SVDLayer;

    // The SVD is a vector of SVDLayers
    typedef std::vector< SVDLayer > SVD;


    SVD calibrations;


  public:

    // This enumeration assure the same semantic of the
    // isU methods defined by Peter Kv.
    enum E_side { Vindex = 0 , Uindex = 1 };

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
            T::init(calibrations[layer][ladder][sensor][ Uindex ], layer, ladder, sensor, Uindex ,
                    defaultT);
            T::init(calibrations[layer][ladder][sensor][ Vindex ], layer, ladder, sensor, Vindex ,
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
      return T::get(calibrations.at(layer).at(ladder).at(sensor).at(side) , strip);
    }

    /**
     * Set the calibration associated to a given strip.
     * Return the calibration associated to a given strip identified by:
     * @param layer is the layer number (2 to 6 in the present geometry)
     * @param ladder is the ladder number ( 1 to 7 for layer 3, 1 to 10 for layer 4 etc...)
     * @param sensor is the sensor number ( 1 to 2 for layer 3, 1 to 3 for layer 4 etc...)
     * @param side is the sensor view: 1 for Side U ( Side P ), 0 for side V (Side N)
     * @param strip is the strip number: from 1 to 512 or 768 depending on the sensor
     * it throws std::out_of_range if the strip is unknown
     */
    void set(unsigned int layer,
             unsigned int ladder,
             unsigned int sensor,
             unsigned int side,
             unsigned int strip,
             typename T::calibrationType value)
    {
      T::set(calibrations.at(layer).at(ladder).at(sensor).at(side) , strip , value);
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

  private:
    TString m_uniqueID; //The unique identifier is a private member of SVDCalibrationsBase, whose value is assigned in the constructor.

    ClassDef(SVDCalibrationsBase, 2)
  };

}

