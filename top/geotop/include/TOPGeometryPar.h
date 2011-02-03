/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPGEOMETRYPAR_H
#define TOPGEOMETRYPAR_H

#include <vector>

#include <cmath>
#include <boost/format.hpp>

using namespace std;
using namespace boost;


namespace Belle2 {

//! The Class for TOP Geometry Parameters
  /*! This class provides TOP gemetry paramters for simulation, reconstruction and so on.
      These parameters are gotten from gearbox.
  */

  class TOPGeometryPar {

  public:

    //! Constructor
    TOPGeometryPar();

    //! Destructor
    virtual ~TOPGeometryPar();

    //! Static method to get a reference to the TOPGeometryPar instance.
    /*!
        \return A reference to an instance of this class.
    */
    static TOPGeometryPar* Instance(void);

    //! Clears
    void clear(void);

    //! Print some debug information
    void Print(void) const;

    //! Gets geometry parameters from gearbox.
    void read();

    //! These functions return parameters

    //! Rotation of the whole detector (should be same as beam pipe)
    double GetDetectorRotation() const;
    //! Shift in the Z directio of the whole detector (same as beam pipe)
    double GetDetectorZShift() const;
    //! Get number of TOP bars
    int GetNumberOfBars() const;
    //! Get inner radius of detector tube
    double GetDetectorInnerRadius() const;
    //! Get width of the quartz bar
    double GetQuartzWidth() const;
    //! Get quartz bar thickness
    double GetQuartzThickness() const;
    //! Get z position quartz bar forward
    double GetQuartzFPos() const;
    //! Get z position quarz bar backwad
    double GetQuartzBPos() const;
    //! Get leght of the extension volume on the quartz bar
    double GetQuartzZExt() const;
    //! Get how much extended upwards
    double GetQuartzYExtUp() const;
    //! Get how much extended downwards
    double GetQuartzYExtDown() const;
    //! Get gap between PMTs in x direction
    double GetGapPMTX() const;
    //! Get gap between PMTs in y direction
    double GetGapPMTY() const;
    //! Get coordinate of lower left side of PMT array x coordinate
    double GetGapPMTX0() const;
    //! Get coordinate of lower left side of PMT array y coordinate
    double GetGapPMTY0() const;
    //! Get gap between PMT modules in x direction
    int GetPMTNX() const;
    //! Get gap between PMT modules in y direction
    int GetPMTNY() const;
    //! Get size of the PMT in the x direction
    double GetPMTSizeX() const;
    //! Get size of the PMT in the y direction
    double GetPMTSizeY() const;
    //! Get size of the PMT in the y direction
    double GetPMTSizeZ() const;
    //! Get sensitive size of the PMT in the x direction
    double GetActiveSizeX() const;
    //! Get sensitive size of the PMT in the y direction
    double GetActiveSizeY() const;
    //! Get sensitive size of the PMT in the z direction
    double GetActiveSizeZ() const;
    //! Get thickness of the PMT window
    double GetWindowThickness() const;
    //! Get number of pads in x direction
    int GetPadNX() const;
    //! Get number of pads in y direction
    int GetPadNY() const;
    //! Get side of PMT bottom thickness
    double GetBottomThickness() const;
    //! The thicknes of the QBB honecomb plate abowe and bellow the quartz
    double GetQBBThickness() const;
    //! Get The thicknes of the QBB honecomb plate on the side of the quartz
    double GetQBBSideThickness() const;
    //! Get The thicknes of the QBB honecomb plate in front and behind the quartz
    double GetQBBFronThickness() const;
    //! Get the air gap between quartz and upper plate
    double GetAirGapUp() const;
    //! Get the air gap between quartz and lower plate
    double GetAirGapDown() const;
    //! Get the air gap between quartz and side plate
    double GetAirGapSide() const;
    //! Get forward position of the QBB support
    double GetQBBForwardPos() const;
    //! Get backward position of the QBB support
    double GetQBBBackwardPos() const;

  private:

//! The naming of this variables is defines above
    double _DetectorRotation;    //! Stors the return value for the same name function defined above
    double _DetectorZShift;    //! Stors the return value for the same name function defined above
    int _NumberOfBars;     //! Stors the return value for the same name function defined above
    double _DetectorInnerRadius; //! Stors the return value for the same name function defined above
    double _QuartzWidth;   //! Stors the return value for the same name function defined above
    double _QuartzThickness;   //! Stors the return value for the same name function defined above
    double _QuartzFPos;    //! Stors the return value for the same name function defined above
    double _QuartzBPos;    //! Stors the return value for the same name function defined above
    double _QuartzZExt;    //! Stors the return value for the same name function defined above
    double _QuartzYExtUp;    //! Stors the return value for the same name function defined above
    double _QuartzYExtDown;    //! Stors the return value for the same name function defined above
    double _GapPMTX;     //! Stors the return value for the same name function defined above
    double _GapPMTY;     //! Stors the return value for the same name function defined above
    double _GapPMTX0;      //! Stors the return value for the same name function defined above
    double _GapPMTY0;      //! Stors the return value for the same name function defined above
    int _PMTNX;      //! Stors the return value for the same name function defined above
    int _PMTNY;      //! Stors the return value for the same name function defined above
    double _PMTSizeX;      //! Stors the return value for the same name function defined above
    double _PMTSizeY;      //! Stors the return value for the same name function defined above
    double _PMTSizeZ;      //! Stors the return value for the same name function defined above
    double _ActiveSizeX;   //! Stors the return value for the same name function defined above
    double _ActiveSizeY;   //! Stors the return value for the same name function defined above
    double _ActiveSizeZ;   //! Stors the return value for the same name function defined above
    double _WindowThickness;   //! Stors the return value for the same name function defined above
    int _PadNX;      //! Stors the return value for the same name function defined above
    int _PadNY;      //! Stors the return value for the same name function defined above
    double _BottomThickness;   //! Stors the return value for the same name function defined above
    double _QBBThickness;    //! Stors the return value for the same name function defined above
    double _QBBSideThickness;    //! Stors the return value for the same name function defined above
    double _QBBFronThickness;    //! Stors the return value for the same name function defined above
    double _AirGapUp;      //! Stors the return value for the same name function defined above
    double _AirGapDown;    //! Stors the return value for the same name function defined above
    double _AirGapSide;    //! Stors the return value for the same name function defined above
    double _QBBForwardPos;   //! Stors the return value for the same name function defined above
    double _QBBBackwardPos;    //! Stors the return value for the same name function defined above


    static TOPGeometryPar* p_B4TOPGeometryParDB; /*!< Pointer that saves the instance of this class. */

  };

  //-----------------------------------------------------------------------------


  inline double TOPGeometryPar::GetDetectorRotation() const
  {
    return _DetectorRotation;
  }

  inline double TOPGeometryPar::GetDetectorZShift() const
  {
    return _DetectorZShift;
  }

  inline int TOPGeometryPar::GetNumberOfBars() const
  {
    return _NumberOfBars;
  }

  inline double TOPGeometryPar::GetDetectorInnerRadius() const
  {
    return _DetectorInnerRadius;
  }

  inline double TOPGeometryPar::GetQuartzWidth() const
  {
    return _QuartzWidth;
  }

  inline double TOPGeometryPar::GetQuartzThickness() const
  {
    return _QuartzThickness;
  }

  inline double TOPGeometryPar::GetQuartzFPos() const
  {
    return _QuartzFPos;
  }

  inline double TOPGeometryPar::GetQuartzBPos() const
  {
    return _QuartzBPos;
  }

  inline double TOPGeometryPar::GetQuartzZExt() const
  {
    return _QuartzZExt;
  }

  inline double TOPGeometryPar::GetQuartzYExtUp() const
  {
    return _QuartzYExtUp;
  }

  inline double TOPGeometryPar::GetQuartzYExtDown() const
  {
    return _QuartzYExtDown;
  }

  inline double TOPGeometryPar::GetGapPMTX() const
  {
    return _GapPMTX;
  }

  inline double TOPGeometryPar::GetGapPMTY() const
  {
    return _GapPMTY;
  }

  inline double TOPGeometryPar::GetGapPMTX0() const
  {
    return _GapPMTX0;
  }

  inline double TOPGeometryPar::GetGapPMTY0() const
  {
    return _GapPMTY0;
  }

  inline int TOPGeometryPar::GetPMTNX() const
  {
    return _PMTNX;
  }

  inline int TOPGeometryPar::GetPMTNY() const
  {
    return _PMTNY;
  }

  inline double TOPGeometryPar::GetPMTSizeX() const
  {
    return _PMTSizeX;
  }

  inline double TOPGeometryPar::GetPMTSizeY() const
  {
    return _PMTSizeY;
  }

  inline double TOPGeometryPar::GetPMTSizeZ() const
  {
    return _PMTSizeZ;
  }

  inline double TOPGeometryPar::GetActiveSizeX() const
  {
    return _ActiveSizeX;
  }

  inline double TOPGeometryPar::GetActiveSizeY() const
  {
    return _ActiveSizeY;
  }

  inline double TOPGeometryPar::GetActiveSizeZ() const
  {
    return _ActiveSizeZ;
  }

  inline double TOPGeometryPar::GetWindowThickness() const
  {
    return _WindowThickness;
  }

  inline int TOPGeometryPar::GetPadNX() const
  {
    return _PadNX;
  }

  inline int TOPGeometryPar::GetPadNY() const
  {
    return _PadNY;
  }

  inline double TOPGeometryPar::GetBottomThickness() const
  {
    return _BottomThickness;
  }

  inline double TOPGeometryPar::GetQBBThickness() const
  {
    return _QBBThickness;
  }

  inline double TOPGeometryPar::GetQBBSideThickness() const
  {
    return _QBBSideThickness;
  }

  inline double TOPGeometryPar::GetQBBFronThickness() const
  {
    return _QBBFronThickness;
  }

  inline double TOPGeometryPar::GetAirGapUp() const
  {
    return _AirGapUp;
  }

  inline double TOPGeometryPar::GetAirGapDown() const
  {
    return _AirGapDown;
  }

  inline double TOPGeometryPar::GetAirGapSide() const
  {
    return _AirGapSide;
  }

  inline double TOPGeometryPar::GetQBBForwardPos() const
  {
    return _QBBForwardPos;
  }

  inline double TOPGeometryPar::GetQBBBackwardPos() const
  {
    return _QBBBackwardPos;
  }

} // end of namespace Belle2

#endif

