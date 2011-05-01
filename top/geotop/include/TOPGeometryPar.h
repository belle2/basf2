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
    //! Get width of the quartz bar extension
    double GetQuartzExtWidth() const;
    //! Get quartz bar thickness
    double GetQuartzThickness() const;
    //! Get z position quarz bar backwad
    double GetQuartzBPos() const;
    //! Get leghth of first quartz bar segment
    double GetQuartzLenSeg1() const;
    //! Get leghth of second quartz bar segment
    double GetQuartzLenSeg2() const;
    //! Get leghth of quarz bar to which the mirror is attached
    double GetQuartzLenMir() const;
    //! Get leght of the extension volume on the quartz bar
    double GetQuartzZExt() const;
    //! Get how much extended upwards
    double GetQuartzYExtUp() const;
    //! Get how much extended downwards
    double GetQuartzYExtDown() const;
    //! Get the thickness of the glue between quartz bar and extension volume
    double GetGlueThickness1() const;
    //! Get the thickness of the glue between quartz bar and quarz bar
    double GetGlueThickness2() const;
    //! Get the thickness of the glue between quartz bar and mirror volume
    double GetGlueThickness3() const;
    //! Get mirror center x in the plane quartz bar thickness vs. quartz bar qidth
    double GetMirrorCenterX() const;
    //! Get mirror center y in the plane quartz bar thickness vs. quartz bar qidth
    double GetMirrorCenterY() const;
    //! Get focal length of mirror
    double GetMirrorR() const;
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

//! Storing variables for the function defined above

    //! Rotation of the whole detector (should be same as beam pipe)
    double _DetectorRotation;
    //! Shift in the Z directio of the whole detector (same as beam pipe)
    double _DetectorZShift;
    //!  number of TOP bars
    int _NumberOfBars;
    //!  inner radius of detector tube
    double _DetectorInnerRadius;
    //!  width of the quartz bar
    double _QuartzWidth;
    //!  width of the quartz bar extension
    double _QuartzExtWidth;
    //!  quartz bar thickness
    double _QuartzThickness;
    //!  z position quarz bar backwad
    double _QuartzBPos;
    //!  leghth of first quartz bar segment
    double _QuartzLenSeg1;
    //!  leghth of second quartz bar segment
    double _QuartzLenSeg2;
    //!  leghth of quarz bar to which the mirror is attached
    double _QuartzLenMir;
    //!  leght of the extension volume on the quartz bar
    double _QuartzZExt;
    //!  how much extended upwards
    double _QuartzYExtUp;
    //!  how much extended downwards
    double _QuartzYExtDown;
    //!  the thickness of the glue between quartz bar and extension volume
    double _GlueThickness1;
    //!  the thickness of the glue between quartz bar and quarz bar
    double _GlueThickness2;
    //!  the thickness of the glue between quartz bar and mirror volume
    double _GlueThickness3;
    //!  mirror center x in the plane quartz bar thickness vs. quartz bar qidth
    double _MirrorCenterX;
    //!  mirror center y in the plane quartz bar thickness vs. quartz bar qidth
    double _MirrorCenterY;
    //!  focal length of mirror
    double _MirrorR;
    //!  gap between PMTs in y direction
    double _GapPMTX;
    //!  gap between PMTs in y direction
    double _GapPMTY;
    //!  coordinate of lower left side of PMT array x coordinate
    double _GapPMTX0;
    //!  coordinate of lower left side of PMT array y coordinate
    double _GapPMTY0;
    //!  gap between PMT modules in x direction
    int _PMTNX;
    //!  gap between PMT modules in y direction
    int _PMTNY;
    //!  size of the PMT in the x direction
    double _PMTSizeX;
    //!  size of the PMT in the y direction
    double _PMTSizeY;
    //!  size of the PMT in the y direction
    double _PMTSizeZ;
    //!  sensitive size of the PMT in the x direction
    double _ActiveSizeX;
    //!  sensitive size of the PMT in the y direction
    double _ActiveSizeY;
    //!  sensitive size of the PMT in the z direction
    double _ActiveSizeZ;
    //!  thickness of the PMT window
    double _WindowThickness;
    //!  number of pads in x direction
    int _PadNX;
    //!  number of pads in y direction
    int _PadNY;
    //!  side of PMT bottom thickness
    double _BottomThickness;
    //! The thicknes of the QBB honecomb plate abowe and bellow the quartz
    double _QBBThickness;
    //!  The thicknes of the QBB honecomb plate on the side of the quartz
    double _QBBSideThickness;
    //!  The thicknes of the QBB honecomb plate in front and behind the quartz
    double _QBBFronThickness;
    //!  the air gap between quartz and upper plate
    double _AirGapUp;
    //!  the air gap between quartz and lower plate
    double _AirGapDown;
    //!  the air gap between quartz and side plate
    double _AirGapSide;
    //!  forward position of the QBB support
    double _QBBForwardPos;
    //!  backward position of the QBB support
    double _QBBBackwardPos;


    static TOPGeometryPar* p_B4TOPGeometryParDB; /*!< Pointer that saves the instance of this class. */

  };

  //-----------------------------------------------------------------------------

  //! Rotation of the whole detector (should be same as beam pipe)
  inline double TOPGeometryPar::GetDetectorRotation() const
  {
    return _DetectorRotation;
  }
  //! Shift in the Z directio of the whole detector (same as beam pipe)
  inline double TOPGeometryPar::GetDetectorZShift() const
  {
    return _DetectorZShift;
  }
  //! Returns number of TOP bars
  inline int TOPGeometryPar::GetNumberOfBars() const
  {
    return _NumberOfBars;
  }
  //! Returns inner radius of detector tube
  inline double TOPGeometryPar::GetDetectorInnerRadius() const
  {
    return _DetectorInnerRadius;
  }
  //! Returns width of the quartz bar
  inline double TOPGeometryPar::GetQuartzWidth() const
  {
    return _QuartzWidth;
  }
  //! Returns width of the quartz bar extension
  inline double TOPGeometryPar::GetQuartzExtWidth() const
  {
    return _QuartzExtWidth;
  }
  //! Returns quartz bar thickness
  inline double TOPGeometryPar::GetQuartzThickness() const
  {
    return _QuartzThickness;
  }
  //! Returns z position quarz bar backwad
  inline double TOPGeometryPar::GetQuartzBPos() const
  {
    return _QuartzBPos;
  }
  //! Returns leghth of first quartz bar segment
  inline double TOPGeometryPar::GetQuartzLenSeg1() const
  {
    return _QuartzLenSeg1;
  }
  //! Returns leghth of second quartz bar segment
  inline double TOPGeometryPar::GetQuartzLenSeg2() const
  {
    return _QuartzLenSeg2;
  }
  //! Returns leghth of quarz bar to which the mirror is attached
  inline double TOPGeometryPar::GetQuartzLenMir() const
  {
    return _QuartzLenMir;
  }
  //! Returns leght of the extension volume on the quartz bar
  inline double TOPGeometryPar::GetQuartzZExt() const
  {
    return _QuartzZExt;
  }
  //! Returns how much extended upwards
  inline double TOPGeometryPar::GetQuartzYExtUp() const
  {
    return _QuartzYExtUp;
  }
  //! Returns how much extended downwards
  inline double TOPGeometryPar::GetQuartzYExtDown() const
  {
    return _QuartzYExtDown;
  }
  //! Returns the thickness of the glue between quartz bar and extension volume
  inline double TOPGeometryPar::GetGlueThickness1() const
  {
    return _GlueThickness1;
  }
  //! Returns the thickness of the glue between quartz bar and quarz bar
  inline double TOPGeometryPar::GetGlueThickness2() const
  {
    return _GlueThickness2;
  }
  //! Returns the thickness of the glue between quartz bar and mirror volume
  inline double TOPGeometryPar::GetGlueThickness3() const
  {
    return _GlueThickness3;
  }
  //! Returns mirror center x in the plane quartz bar thickness vs. quartz bar qidth
  inline double TOPGeometryPar::GetMirrorCenterX() const
  {
    return _MirrorCenterX;
  }
  //! Returns mirror center y in the plane quartz bar thickness vs. quartz bar qidth
  inline double TOPGeometryPar::GetMirrorCenterY() const
  {
    return _MirrorCenterY;
  }
  //! Returns focal length of mirror
  inline double TOPGeometryPar::GetMirrorR() const
  {
    return _MirrorR;
  }
  //! Returns gap between PMTs in x direction
  inline double TOPGeometryPar::GetGapPMTX() const
  {
    return _GapPMTX;
  }
  //! Returns gap between PMTs in y direction
  inline double TOPGeometryPar::GetGapPMTY() const
  {
    return _GapPMTY;
  }
  //! Returns coordinate of lower left side of PMT array x coordinate
  inline double TOPGeometryPar::GetGapPMTX0() const
  {
    return _GapPMTX0;
  }
  //! Returns coordinate of lower left side of PMT array y coordinate
  inline double TOPGeometryPar::GetGapPMTY0() const
  {
    return _GapPMTY0;
  }
  //! Returns gap between PMT modules in x direction
  inline int TOPGeometryPar::GetPMTNX() const
  {
    return _PMTNX;
  }
  //! Returns gap between PMT modules in y direction
  inline int TOPGeometryPar::GetPMTNY() const
  {
    return _PMTNY;
  }
  //! Returns size of the PMT in the x direction
  inline double TOPGeometryPar::GetPMTSizeX() const
  {
    return _PMTSizeX;
  }
  //! Returns size of the PMT in the y direction
  inline double TOPGeometryPar::GetPMTSizeY() const
  {
    return _PMTSizeY;
  }
  //! Returns size of the PMT in the y direction
  inline double TOPGeometryPar::GetPMTSizeZ() const
  {
    return _PMTSizeZ;
  }
  //! Returns sensitive size of the PMT in the x direction
  inline double TOPGeometryPar::GetActiveSizeX() const
  {
    return _ActiveSizeX;
  }
  //! Returns sensitive size of the PMT in the y direction
  inline double TOPGeometryPar::GetActiveSizeY() const
  {
    return _ActiveSizeY;
  }
  //! Returns sensitive size of the PMT in the z direction
  inline double TOPGeometryPar::GetActiveSizeZ() const
  {
    return _ActiveSizeZ;
  }
  //! Returns thickness of the PMT window
  inline double TOPGeometryPar::GetWindowThickness() const
  {
    return _WindowThickness;
  }
  //! Returns number of pads in x direction
  inline int TOPGeometryPar::GetPadNX() const
  {
    return _PadNX;
  }
  //! Returns number of pads in y direction
  inline int TOPGeometryPar::GetPadNY() const
  {
    return _PadNY;
  }
  //! Returns side of PMT bottom thickness
  inline double TOPGeometryPar::GetBottomThickness() const
  {
    return _BottomThickness;
  }
  //! The thicknes of the QBB honecomb plate abowe and bellow the quartz
  inline double TOPGeometryPar::GetQBBThickness() const
  {
    return _QBBThickness;
  }
  //! Returns The thicknes of the QBB honecomb plate on the side of the quartz
  inline double TOPGeometryPar::GetQBBSideThickness() const
  {
    return _QBBSideThickness;
  }
  //! Returns The thicknes of the QBB honecomb plate in front and behind the quartz
  inline double TOPGeometryPar::GetQBBFronThickness() const
  {
    return _QBBFronThickness;
  }
  //! Returns the air gap between quartz and upper plate
  inline double TOPGeometryPar::GetAirGapUp() const
  {
    return _AirGapUp;
  }
  //! Returns the air gap between quartz and lower plate
  inline double TOPGeometryPar::GetAirGapDown() const
  {
    return _AirGapDown;
  }
  //! Returns the air gap between quartz and side plate
  inline double TOPGeometryPar::GetAirGapSide() const
  {
    return _AirGapSide;
  }
  //! Returns forward position of the QBB support
  inline double TOPGeometryPar::GetQBBForwardPos() const
  {
    return _QBBForwardPos;
  }
  //! Returns backward position of the QBB support
  inline double TOPGeometryPar::GetQBBBackwardPos() const
  {
    return _QBBBackwardPos;
  }

} // end of namespace Belle2

#endif

