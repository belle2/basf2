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
#include <framework/logging/Logger.h>
#include <cmath>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <TVector2.h>
#include <G4Transform3D.hh>
#include <G4ThreeVector.hh>
#include <G4RotationMatrix.hh>

using namespace std;
using namespace boost;

#define MAX_N_BARS 16

namespace Belle2 {
  namespace TOP {

    /** The Class for TOP Geometry Parameters.
     *  The aim of this class it to pass the parameters stored in .xml files to the geometry builder and any other module.
     *  Usage of this class asures that all parts of TOP obtain the same information.
     *  All acces to .xml files should be made through usage of this class.
     */

    class TOPGeometryPar {

    public:

      /**< Constructor */
      TOPGeometryPar();

      /**< Destructor */
      virtual ~TOPGeometryPar();

      /** Static method to get a reference to the TOPGeometryPar instance.
       * @return A reference to an instance of this class.
       */
      static TOPGeometryPar* Instance();

      /** Initialize
       * @param  GearBox directory where .xml are stored
       */
      void Initialize(const GearDir& content);

      /**< Generic clear function */
      void clear(void);

      /** Generic print function, possibility to add debug information */
      void Print(void) const;

      /** Reads geometry parameters from gearbox into object
       * @param GearBox directory where .xml are stored
       */
      void read(const GearDir& content);




      //! Functions for returning parameters of the bars

      /** Get number of quartz bars
       * @return number of quartz bars
       */
      int getNbars() const;

      /** Get distance from IP to inner side of the qurtz bar
       * @return distance from IP to inner side of the qurtz bar in mm
       */
      double getRadius() const;

      /** Get the width of the quartz bar
       *@return width of quartz bar in mm
       */
      double getQwidth() const;

      /** Get the thickness of the quartz bar
       * @return thickness of quartz bar in mm
       */
      double getQthickness() const;


      /** Get Backward position of qurtz bar
       * @return barward possition on quartz bar in mm
       */
      double getBposition() const;

      /** Get length of first quartz bar segment. First segment is the backward segmenet.
       * @return length of first quartz bar segment in mm
       */
      double getLength1() const;

      /** Get length of second quartz bar segment. Second segment is the forward segmenet.
       * @return length of second quartz bar segment in mm
       */
      double getLength2() const;

      /** Get length of third quartz bar segment. Third segment is the segment to which the mirror is attached
       * @return length of third quartz bar segment in mm
       */
      double getLength3() const;

      /** Get length of the wedge segment
       * @return length of wedge segment in mm
       */
      double getWLength() const;

      /** Get width of the wedge segment
       * @return width of wedge segment in mm
       */
      double getWwidth() const;

      /** Get wedge extension down measured from the inner side of the bar
       * @return wedge extension down in mm
       */
      double getWextdown() const;

      /** get width of glue between wedge and bar segment 1
       * @return width of the glue joint 1 in mm
       */
      double getGwidth1() const;

      /** get width of glue between bar segment 1 and 2
       * @return width of the glue joint 2 in mm
       */
      double getGwidth2() const;

      /** get width of glue between bar segment 2 and segment 3
       * @return width of the glue joint 3 in mm
       */
      double getGwidth3() const;



      //! Parameters of the PMT

      /** Get gap between PMTs in x direction
       * @return gap between PMTs in x direction in mm
       */
      double getXgap() const;

      /** Get gap between PMTs in y direction
       * @return gap between PMTs in y direction in mm
       */
      double getYgap() const;

      /** Get number of PMTs in the x row
       * @return number of PMTs in the x row
       */
      int getNpmtx() const;

      /** Get number of PMTs in the y column
       * @return number of PMTs in the y column
       */
      int getNpmty() const;

      /** Get module side in x direction
       * @return module side in x direction in mm
       */
      double getMsizex() const;

      /** Get module side in y direction
       * @return module side y direction in mm
       */
      double getMsizey() const;

      /** Get module wall size in in z direction
       * @return module wall size in z direction in mm
       */
      double getMsizez() const;

      /** Get module wall thickness
       * @return module wall thickness in mm
       */
      double getMWallThickness() const;

      /** Get active area size in x direction
       * @return active area size in x direction in mm
       */
      double getAsizex() const;

      /** Get active area size in y direction
       * @return active area size in y direction in mm
       */
      double getAsizey() const;

      /** Get active area size in z direction
       * @return active area size in z direction in mm
       */
      double getAsizez() const;

      /** Get thickness of the PMT window
       * @return  window thickness in mm
       */
      double getWinthickness() const;

      /** Get thickness of PMT bottom
       * @return bottom thickness in mm
       */
      double getBotthickness() const;

      /** Get number of pads in x direction
       * @return number of pads in x direction in mm
       */
      int getNpadx() const;

      /** Get number of pads in y direction
       * @return number of pads in y direction
       */
      int getNpady() const;

      /** Get the thickness of the glue between the PMT stack and the wedge
       * @return the thickness of the glue between the PMT stack and the wedge in mm
       */
      double getdGlue() const;

      /** Get number of TDC bits
       * @return number of TDC bits
       */
      int getTDCbits() const;

      /** Get width of TDC bit
       * @return width of TDC bit in ns
       */
      double getTDCbitwidth() const;



      //! Mirror parameters

      /** Get the dispacement along quartz width for mirror axis
       * @return dispacement along quartz width for mirror axis in mm
       */
      double getMirposx() const;

      /** Get the dispacement along quartz thickness for mirror axis
       * @return dispacement along quartz thickness for mirror axis in mm
       */
      double getMirposy() const;


      /** Get mirror layer thickness
       * @return mirror layer thickness
       */
      double getMirthickness() const;

      /** Get radius of spherical mirror
       * @return radius of spherical mirror
       */
      double getMirradius() const;

      //! Support parameters

      /** Get thickness of the QBB pannel
       * @return thicness of QBB pannel in mm
       */
      double getPannelThickness() const;

      /** Get thickness of the sinde plate
       * @return thickness of side pladte in mm
       */
      double getPlateThickness() const;

      /** Get size of air gap between quartz and support at inner radious
       * @return air gar in mm
       */
      double getLowerGap() const;

      /** Get size of air gap between quartz and support at outer radious
       * @return air gar in mm
       */
      double getUpperGap() const;

      /** Get size of air gap between quartz and support (side plate)
       * @return air gar in mm
       */
      double getSideGap() const;

      /** Get forward possition of support
       * @return forward possition of support in mm
       */
      double getZForward() const;

      /** Get backward possition of support
       * @return backward possition of support in mm
       */
      double getZBackward() const;

      /**< derived function that are part of geometry */

      /** Calculate the channel ID from local possition on the PMT
       * @param position on the sensitive detector plane
       * @param ID of the PMT
       * @return ID of channel for whole bar
       */
      int getChannelID(TVector2 position, int moduleID) const;

      /** Ger backwads possition
       *@return backward possition of the quartz bar
       */
      double getZ1() const;

      /** Get forward possition
       * @return forward possition of the bar (segmen 1+2+3 + glues)
       */
      double getZ2() const;

      /**
       * Get Alignment for given component from the database
       * @param  component Name of the component to align
       * @return Transformation matrix for component, idendity if component
       *         could not be found
       */
      G4Transform3D getAlignment(const std::string& component);


    private:

      //! Parameters for bars


      int _Nbars;                /**< number of quartz bars */
      double _Radius;            /**< distance from IP to inner side of qurtz bar */
      double _Qwidth;            /**< width of the quartz bar */
      double _Qthickness;        /**< thickness of the quartz bar */
      double _Bposition;         /**< backward position of qurtz bar */
      double _Length1;           /**< length of first quartz bar segment possitioned backward */
      double _Length2;           /**< length of second quartz bar segment possitioned forwards */
      double _Length3;           /**< length segment to which the mirror is attached */
      double _WLength;           /**< length wedge segment */
      double _Wwidth;            /**< width wedge segment */
      double _Wextdown;          /**< wedge extension down */
      double _Gwidth1;           /**< width of glue between wedge and segment 1 */
      double _Gwidth2;           /**< width of glue between segment 1 segmen 2 */
      double _Gwidth3;           /**< width of glue between segment 3 and mirror segment */


      //! Parameters for PMT


      double _Xgap;              /**< gap between PMTs in x direction */
      double _Ygap;              /**< gap between PMTs in x direction */
      int _Npmtx;                /**< number of PMTs in one row */
      int _Npmty;                /**< number og PMTs in one column */
      double _Msizex;            /**< module side X */
      double _Msizey;            /**< module side Y */
      double _Msizez;            /**< module wall size in Z */
      double _MWallThickness;    /**< module wall thickness */
      double _Asizex;            /**< active area size in X */
      double _Asizey;            /**< active area size in y */
      double _Asizez;            /**< active area size in z */
      double _Winthickness;      /**< window thickness */
      double _Botthickness;      /**< window thickness */
      int _Npadx;                /**< number of pads in x */
      int _Npady;                /**< number of pads in y */
      double _dGlue;             /**< PMT wedge glue thickness */
      int _NTDC;                 /**< number of TDC bits */
      double _TDCwidth;          /**< width of TDC bit */

      /**< Mirror parameters */


      double _Mirposx;           /**< dispacement along quartz width for mirror axis */
      double _Mirposy;           /**< dispacement along quartz whickness for mirror axis */
      double _Mirthickness;      /**< mirror layer thickness */
      double _Mirradius;         /**< radius of spherical mirror */

      //! Support parameters


      double _PannelThickness;   /**< thicness of QBB pannel */
      double _PlateThickness;    /**< thickness of side pladte */
      double _LowerGap;          /**< lower air gap */
      double _UpperGap;          /**< upper air gap */
      double _SideGap;           /**< side air gap */
      double _ZForward;          /**< forward possition of support */
      double _ZBackward;         /**< backward possition of support */


      //! Technical parameters

      static TOPGeometryPar* p_B4TOPGeometryParDB; /*!< Pointer that saves the instance of this class. */

      //! Holds path to alignment
      GearDir m_alignment;        /**<GearDir used to store alignment path */

    };
    //! functions for bars

    inline int TOPGeometryPar::getNbars() const
    {
      return _Nbars;
    }

    inline double TOPGeometryPar::getRadius() const
    {
      return _Radius / Unit::mm;
    }

    inline double TOPGeometryPar::getQwidth() const
    {
      return _Qwidth / Unit::mm;
    }

    inline double TOPGeometryPar::getQthickness() const
    {
      return _Qthickness / Unit::mm;
    }

    inline double TOPGeometryPar::getBposition() const
    {
      return _Bposition / Unit::mm;
    }

    inline double TOPGeometryPar::getLength1() const
    {
      return _Length1 / Unit::mm;
    }

    inline double TOPGeometryPar::getLength2() const
    {
      return _Length2 / Unit::mm;
    }

    inline double TOPGeometryPar::getLength3() const
    {
      return _Length3 / Unit::mm;
    }

    inline double TOPGeometryPar::getWLength() const
    {
      return _WLength / Unit::mm;
    }

    inline double TOPGeometryPar::getWwidth() const
    {
      return _Wwidth / Unit::mm;
    }

    inline double TOPGeometryPar::getWextdown() const
    {
      return _Wextdown / Unit::mm;
    }

    inline double TOPGeometryPar::getGwidth1() const
    {
      return _Gwidth1 / Unit::mm;
    }

    inline double TOPGeometryPar::getGwidth2() const
    {
      return _Gwidth2 / Unit::mm;
    }

    inline double TOPGeometryPar::getGwidth3() const
    {
      return _Gwidth3 / Unit::mm;
    }


    //! functions for PMT


    inline double TOPGeometryPar::getXgap() const
    {
      return _Xgap / Unit::mm;
    }

    inline double TOPGeometryPar::getYgap() const
    {
      return _Ygap / Unit::mm;
    }

    inline int TOPGeometryPar::getNpmtx() const
    {
      return _Npmtx;
    }

    inline int TOPGeometryPar::getNpmty() const
    {
      return _Npmty;
    }

    inline double TOPGeometryPar::getMsizex() const
    {
      return _Msizex / Unit::mm;
    }

    inline double TOPGeometryPar::getMsizey() const
    {
      return _Msizey / Unit::mm;
    }

    inline double TOPGeometryPar::getMsizez() const
    {
      return _Msizez / Unit::mm;
    }

    inline double TOPGeometryPar::getMWallThickness() const
    {
      return _MWallThickness / Unit::mm;
    }

    inline double TOPGeometryPar::getAsizex() const
    {
      return _Asizex / Unit::mm;
    }

    inline double TOPGeometryPar::getAsizey() const
    {
      return _Asizey / Unit::mm;
    }

    inline double TOPGeometryPar::getAsizez() const
    {
      return _Asizez / Unit::mm;
    }

    inline double TOPGeometryPar::getWinthickness() const
    {
      return _Winthickness / Unit::mm;
    }

    inline double TOPGeometryPar::getBotthickness() const
    {
      return _Botthickness / Unit::mm;
    }

    inline int TOPGeometryPar::getNpadx() const
    {
      return _Npadx;
    }

    inline int TOPGeometryPar::getNpady() const
    {
      return _Npmty;
    }

    inline double TOPGeometryPar::getdGlue() const
    {
      return _dGlue / Unit::mm;
    }

    inline int TOPGeometryPar::getTDCbits() const
    {
      return _NTDC;
    }

    inline double TOPGeometryPar::getTDCbitwidth() const
    {
      return _TDCwidth / Unit::ns;
    }

    //! Mirror functions


    inline double TOPGeometryPar::getMirposx() const
    {
      return _Mirposx / Unit::mm;
    }

    inline double TOPGeometryPar::getMirposy() const
    {
      return _Mirposy / Unit::mm;
    }

    inline double TOPGeometryPar::getMirthickness() const
    {
      return _Mirthickness / Unit::mm;
    }

    inline double TOPGeometryPar::getMirradius() const
    {
      return _Mirradius / Unit::mm;
    }

    //! Support parameters


    inline double TOPGeometryPar::getPannelThickness() const
    {
      return _PannelThickness / Unit::mm;
    }

    inline double TOPGeometryPar::getPlateThickness() const
    {
      return _PlateThickness / Unit::mm;
    }

    inline double TOPGeometryPar::getLowerGap() const
    {
      return _LowerGap / Unit::mm;
    }

    inline double TOPGeometryPar::getUpperGap() const
    {
      return _UpperGap / Unit::mm;
    }

    inline double TOPGeometryPar::getSideGap() const
    {
      return _SideGap / Unit::mm;
    }

    inline double TOPGeometryPar::getZForward() const
    {
      return _ZForward / Unit::mm;
    }

    inline double TOPGeometryPar::getZBackward() const
    {
      return _ZBackward / Unit::mm;
    }


    //! Derived functions which are part of geometry


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

    inline double TOPGeometryPar::getZ1() const
    {
      return _Bposition / Unit::mm - _Gwidth1 / Unit::mm;
    }

    inline double TOPGeometryPar::getZ2() const
    {
      return _Bposition / Unit::mm + _Length1 / Unit::mm + _Gwidth2 / Unit::mm + _Length2 / Unit::mm + _Gwidth3 / Unit::mm + _Length3 / Unit::mm;
    }

    //! Alignement reader

    inline G4Transform3D TOPGeometryPar::getAlignment(const string& component)
    {
      //! Format the path using BOOST
      string path = (boost::format("Align[@component='%1%']/") % component).str();
      //! Appendt path to alignement path
      GearDir params(m_alignment, path);
      //! Check if parameter exists
      if (!params) {
        B2WARNING("Could not find alignment parameters for component " << component);
        return G4Transform3D();
      }
      //! Read the translations
      double dU = params.getLength("du") / Unit::mm;
      double dV = params.getLength("dv") / Unit::mm;
      double dW = params.getLength("dw") / Unit::mm;
      //! Read the rotations
      double alpha = params.getAngle("alpha");
      double beta  = params.getAngle("beta");
      double gamma = params.getAngle("gamma");
      //! Combine rotations and tralstions
      G4RotationMatrix rotation(alpha, beta, gamma);
      G4ThreeVector translation(dU, dV, dW);
      //! Return combine matrix
      return G4Transform3D(rotation, translation);
    }


  } //! end of namespace TOP
} //! end of namespace Belle2
#endif
