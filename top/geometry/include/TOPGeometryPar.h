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

#include <map>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <cmath>
#include <boost/format.hpp>
#include <TVector2.h>

using namespace std;
using namespace boost;

#define MAX_N_BARS 16

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
    static TOPGeometryPar* Instance();

    //! Initialize
    void Initialize(const GearDir& content);

    //! Clears
    void clear(void);

    //! Print some debug information
    void Print(void) const;

    //! gets geometry parameters from gearbox.
    void read(const GearDir& content);

    //! Parameters for quarz bar

    //! get number of quartz bars
    int getNbars() const;
    //! get distance from IP to inner side of qurtz bar
    double getRadius() const;
    //! get width of the quartz bar
    double getQwidth() const;
    //! get thickness of the quartz bar
    double getQthickness() const;
    //! get Backward position of qurtz bar
    double getBposition() const;
    //! get length of first quartz bar segment which at one side is positioned at Bposition
    double getLength1() const;
    //! get length of second quartz bar segment which at one side is positioned at Bposition+Length1
    double getLength2() const;
    //! get length segment to which the mirror is attached
    double getLength3() const;
    //! get length wedge segment
    double getWLength() const;
    //! get width wedge segment
    double getWwidth() const;
    //! get wedge extension up
    double getWextup() const;
    //! get wedge extension down
    double getWextdown() const;
    //! get width of glue between wedge and segment 1
    double getGwidth1() const;
    //! get width of glue between segment 1 segmen 2
    double getGwidth2() const;
    //! get width of glue between segment 3 and mirror segment
    double getGwidth3() const;


    //! Parameters for PMT

    //! get gap between PMTs in x direction
    double getXgap() const;
    //! get gap between PMTs in x direction
    double getYgap() const;
    //! get number of PMTs in one row
    int getNpmtx() const;
    //! get number of PMTs in one column
    int getNpmty() const;
    //! get module side X
    double getMsizex() const;
    //! get module side Y
    double getMsizey() const;
    //! get module wall size in Z
    double getMWsizez() const;
    //! get active area size in X
    double getAsizex() const;
    //! get active area size in y
    double getAsizey() const;
    //! get active area size in z
    double getAsizez() const;
    //! get window thickness
    double getWinthickness() const;
    //! get window thickness
    double getBotthickness() const;
    //! get number of pads in x
    int getNpadx() const;
    //! get number of pads in y
    int getNpady() const;
    //! get the thickness of the glue between the PMT stack and the wedge
    double getdGlue() const;
    //! get number of TDC bits
    int getTDCbits() const;
    //! get width of TDC bit
    double getTDCbitwidth() const;


    //! Mirror parameters

    //! dispacement along quartz width for mirror axis
    double getMirposx() const;
    //! dispacement along quartz whickness for mirror axis
    double getMirposy() const;
    //! get mirror layer thickness
    double getMirthickness() const;
    //! get radius of spherical mirror
    double getMirradius() const;

    //! derived function that are part of geometry

    //! conversion of locaton possition to channel ID
    int getChannelID(TVector2 position, int moduleID) const;

    //!Backwads possition
    double getZ1() const;

    //!Forward possition
    double getZ2() const;

  private:
    //! Parameters for bars


    //! number of quartz bars
    int _Nbars;
    //! distance from IP to inner side of qurtz bar
    double _Radius;
    //! width of the quartz bar
    double _Qwidth;
    //! thickness of the quartz bar
    double _Qthickness;
    //! Backward position of qurtz bar
    double _Bposition;
    //! length of first quartz bar segment which at one side is positioned at Bposition
    double _Length1;
    //! length of second quartz bar segment which at one side is positioned at Bposition+Length1
    double _Length2;
    //! length segment to which the mirror is attached
    double _Length3;
    //! length wedge segment
    double _WLength;
    //! width wedge segment
    double _Wwidth;
    //! wedge extension up
    double _Wextup;
    //! wedge extension down
    double _Wextdown;
    //! width of glue between wedge and segment 1
    double _Gwidth1;
    //! width of glue between segment 1 segmen 2
    double _Gwidth2;
    //! width of glue between segment 3 and mirror segment
    double _Gwidth3;


    //! Parameters for PMT

    //! gap between PMTs in x direction
    double _Xgap;
    //! gap between PMTs in x direction
    double _Ygap;
    //! number of PMTs in one row
    int _Npmtx;
    //! number og PMTs in one column
    int _Npmty;
    //! module side X
    double _Msizex;
    //! module side Y
    double _Msizey;
    //! module wall size in Z
    double _MWsizez;
    //! active area size in X
    double _Asizex;
    //! active area size in y
    double _Asizey;
    //! active area size in z
    double _Asizez;
    //! window thickness
    double _Winthickness;
    //! window thickness
    double _Botthickness;
    //! number of pads in x
    int _Npadx;
    //! number of pads in y
    int _Npady;
    //! PMT wedge glue thickness
    double _dGlue;
    //! number of TDC bits
    int _NTDC;
    //! width of TDC bit
    double _TDCwidth;

    //! Mirror parameters

    //! dispacement along quartz width for mirror axis
    double _Mirposx;
    //! dispacement along quartz whickness for mirror axis
    double _Mirposy;
    //! mirror layer thickness
    double _Mirthickness;
    //! radius of spherical mirror
    double _Mirradius;

    //! Support parameters


    static TOPGeometryPar* p_B4TOPGeometryParDB; /*!< Pointer that saves the instance of this class. */

  };
  //!functions for bars

  //! Return number of quartz bars
  inline int TOPGeometryPar::getNbars() const
  {
    return _Nbars;
  }
  //! Return distance from IP to inner side of qurtz bar
  inline double TOPGeometryPar::getRadius() const
  {
    return _Radius / Unit::mm;
  }
  //! Return width of the quartz bar
  inline double TOPGeometryPar::getQwidth() const
  {
    return _Qwidth / Unit::mm;
  }
  //! Return thickness of the quartz bar
  inline double TOPGeometryPar::getQthickness() const
  {
    return _Qthickness / Unit::mm;
  }
  //! Return Backward position of qurtz bar
  inline double TOPGeometryPar::getBposition() const
  {
    return _Bposition / Unit::mm;
  }
  //! Return length of first quartz bar segment which at one side is positioned at Bposition
  inline double TOPGeometryPar::getLength1() const
  {
    return _Length1 / Unit::mm;
  }
  //! Return length of second quartz bar segment which at one side is positioned at Bposition+Length1
  inline double TOPGeometryPar::getLength2() const
  {
    return _Length2 / Unit::mm;
  }
  //! Return length segment to which the mirror is attached
  inline double TOPGeometryPar::getLength3() const
  {
    return _Length3 / Unit::mm;
  }
  //! Return length wedge segment
  inline double TOPGeometryPar::getWLength() const
  {
    return _WLength / Unit::mm;
  }
  //! Return width wedge segment
  inline double TOPGeometryPar::getWwidth() const
  {
    return _Wwidth / Unit::mm;
  }
  //! Return wedge extension up
  inline double TOPGeometryPar::getWextup() const
  {
    return _Wextup / Unit::mm;
  }
  //! Return wedge extension down
  inline double TOPGeometryPar::getWextdown() const
  {
    return _Wextdown / Unit::mm;
  }
  //! Return width of glue between wedge and segment 1
  inline double TOPGeometryPar::getGwidth1() const
  {
    return _Gwidth1 / Unit::mm;
  }
  //! Return width of glue between segment 1 segmen 2
  inline double TOPGeometryPar::getGwidth2() const
  {
    return _Gwidth2 / Unit::mm;
  }
  //! Return width of glue between segment 3 and mirror segment
  inline double TOPGeometryPar::getGwidth3() const
  {
    return _Gwidth3 / Unit::mm;
  }


  //! functions for PMT

  //! Return gap between PMTs in x direction
  inline double TOPGeometryPar::getXgap() const
  {
    return _Xgap / Unit::mm;
  }
  //! Return gap between PMTs in x direction
  inline double TOPGeometryPar::getYgap() const
  {
    return _Ygap / Unit::mm;
  }
  //! Return number of PMTs in one row
  inline int TOPGeometryPar::getNpmtx() const
  {
    return _Npmtx;
  }
  //! Return number og PMTs in one column
  inline int TOPGeometryPar::getNpmty() const
  {
    return _Npmty;
  }
  //! Return module side X
  inline double TOPGeometryPar::getMsizex() const
  {
    return _Msizex / Unit::mm;
  }
  //! Return module side Y
  inline double TOPGeometryPar::getMsizey() const
  {
    return _Msizey / Unit::mm;
  }
  //! Return module wall size in Z
  inline double TOPGeometryPar::getMWsizez() const
  {
    return _MWsizez / Unit::mm;
  }
  //! Return active area size in X
  inline double TOPGeometryPar::getAsizex() const
  {
    return _Asizex / Unit::mm;
  }
  //! Return active area size in y
  inline double TOPGeometryPar::getAsizey() const
  {
    return _Asizey / Unit::mm;
  }
  //! Return active area size in z
  inline double TOPGeometryPar::getAsizez() const
  {
    return _Asizez / Unit::mm;
  }
  //! Return window thickness
  inline double TOPGeometryPar::getWinthickness() const
  {
    return _Winthickness / Unit::mm;
  }
  //! Return window thickness
  inline double TOPGeometryPar::getBotthickness() const
  {
    return _Botthickness / Unit::mm;
  }
  //! Return number of pads in x
  inline int TOPGeometryPar::getNpadx() const
  {
    return _Npadx;
  }
  //! Return number of pads in y
  inline int TOPGeometryPar::getNpady() const
  {
    return _Npmty;
  }
  //! Return glue thickness between PMT stack and weedge
  inline double TOPGeometryPar::getdGlue() const
  {
    return _dGlue / Unit::mm;
  }
  //! Return number of TDC bits
  inline int TOPGeometryPar::getTDCbits() const
  {
    return _NTDC;
  }
  //! Return width of TDC bit
  inline double TOPGeometryPar::getTDCbitwidth() const
  {
    return _TDCwidth / Unit::ns;
  }

  //! Mirror functions

  //! dispacement along quartz width for mirror axis
  inline double TOPGeometryPar::getMirposx() const
  {
    return _Mirposx / Unit::mm;
  }
  //! dispacement along quartz whickness for mirror axis
  inline double TOPGeometryPar::getMirposy() const
  {
    return _Mirposy / Unit::mm;
  }
  //! Return mirror layer thickness
  inline double TOPGeometryPar::getMirthickness() const
  {
    return _Mirthickness / Unit::mm;
  }
  //! Return radius of spherical mirror
  inline double TOPGeometryPar::getMirradius() const
  {
    return _Mirradius / Unit::mm;
  }

  //! Derived functions which are part of geometry

  //Convert possition on PMT into channel ID
  inline int TOPGeometryPar::getChannelID(TVector2 position, int moduleID) const
  {

    double padx = _Asizex / (double)_Npadx;
    double pady = _Asizey / (double)_Npady;

    int ix = int((position.X() + _Asizex / 2.0) / padx);
    int iy = int((position.Y() + _Asizey / 2.0) / pady);

    if (ix > _Npadx - 1 || iy > _Npady - 1) return -1;

    int pmtID = ix + _Npadx * iy;

    int chID = pmtID + moduleID * _Npadx * _Npady;

    return chID;
  }
  //! return backwards posstition
  inline double TOPGeometryPar::getZ1() const
  {
    return _Bposition - _Gwidth1;
  }
  //return forwards posstiton
  inline double TOPGeometryPar::getZ2() const
  {
    return _Bposition - _Length1 + _Gwidth2 + _Length2 + _Gwidth3 + _Length3;
  }

} // end of namespace Belle2

#endif
