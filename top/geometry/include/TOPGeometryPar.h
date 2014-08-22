/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric, Marko Staric                               *
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

#include <top/geometry/TOPQbar.h>


#define MAXPTS_TTS 10
#define MAXPTS_QE 100

namespace Belle2 {
  namespace TOP {

    /** The Class for TOP Geometry Parameters.
     *  To pass the parameters from .xml files to geometry builder and any other module.
     *  Usage of this class asures that all parts of TOP obtain the same information.
     *  All acces to .xml files should be made through usage of this class.
     */

    class TOPGeometryPar {

    public:

      /**< Destructor */
      virtual ~TOPGeometryPar();

      /** Static method to obtain a pointer to the TOPGeometryPar instance.
       * @return pointer to the instance of this class.
       */
      static TOPGeometryPar* Instance();

      /** Initialize
       * @param  GearBox directory where .xml are stored
       */
      void Initialize(const GearDir& content);


      /** Generic print function, possibility to add debug information */
      void Print(void) const;

      /** Reads geometry parameters from gearbox into object
       * @param GearBox directory where .xml are stored
       */
      void read(const GearDir& content);

      /** to return values in the default units */
      void setBasfUnits() {m_unit = Unit::cm;}

      /** to return values in Gean units */
      void setGeanUnits() {m_unit = Unit::mm;}

      /** Set electronic jitter
       * @param jitter rms of electronic jitter in ns
       */
      void setELjitter(double jitter) {m_ELjitter = jitter;}

      /** Set electronic efficiency
       * @param effi efficiency
       */
      void setELefficiency(double effi) {m_ELefficiency = effi;}

      //! Selectors

      /** Get number of quartz bars
       * @return number of quartz bars
       */
      int getNbars() const {return m_Nbars;}

      /** Get distance from IP to inner side of the qurtz bar
       * @return distance from IP to inner side of the qurtz bar
       */
      double getRadius() const {return m_Radius / m_unit;}

      /** Get azimuthal angle of the first bar
       * @return azimuthal angle (radians)
       */
      double getPhi0() const {return m_phi0;}

      /** Get the width of the quartz bar
       *@return width of quartz bar
       */
      double getQwidth() const {return m_Qwidth / m_unit; }

      /** Get the thickness of the quartz bar
       * @return thickness of quartz bar
       */
      double getQthickness() const {return m_Qthickness / m_unit; }

      /** Get Backward position of qurtz bar
       * @return barward possition on quartz bar
       */
      double getBposition() const {return m_Bposition / m_unit; }

      /** Get length of first quartz bar segment.
       * @return length of first quartz bar segment
       */
      double getLength1() const {return m_Length1 / m_unit; }

      /** Get length of second quartz bar segment.
       * @return length of second quartz bar segment
       */
      double getLength2() const {return m_Length2 / m_unit; }

      /** Get length of third quartz bar segment.
       * @return length of third quartz bar segment
       */
      double getLength3() const {return m_Length3 / m_unit; }

      /** Get length of the wedge
       * @return length of wedge
       */
      double getWLength() const {return m_WLength / m_unit; }

      /** Get width of the wedge segment
       * @return width of wedge segment
       */
      double getWwidth() const {return m_Wwidth / m_unit; }

      /** Get wedge extension down measured from the inner side of the bar
       * @return wedge extension down
       */
      double getWextdown() const {return m_Wextdown / m_unit; }

      /** Get length of the wedge flat part at exit window
       * @return length of the flat part
       */
      double getWflat() const {return m_Wflat / m_unit; }

      /** Get width of glue between wedge and bar segment 1
       * @return width of the glue joint 1
       */
      double getGwidth1() const {return m_Gwidth1 / m_unit; }

      /** Get width of glue between bar segment 1 and 2
       * @return width of the glue joint 2
       */
      double getGwidth2() const {return m_Gwidth2 / m_unit; }

      /** Get width of glue between bar segment 2 and segment 3
       * @return width of the glue joint 3
       */
      double getGwidth3() const {return m_Gwidth3 / m_unit; }

      /** Get sigma alpha parameter describing surface roughness of quartz
       * @return sigma alpha of quartz
       */
      double getSigmaAlpha() const {return m_SigmaAlpha; }

      /**
       * Returns Q-bar geometry object
       * @param ID Q-bar ID
       * @return pointer to Q-bar or NULL
       */
      const TOPQbar* getQbar(int ID) const {
        if (ID < 1 || ID > (int) m_bars.size()) return NULL;
        return &m_bars[ID - 1];
      }

      //! Parameters of the PMT

      /** Get gap between PMTs in x direction
       * @return gap between PMTs in x direction
       */
      double getXgap() const {return m_Xgap / m_unit; }

      /** Get gap between PMTs in y direction
       * @return gap between PMTs in y direction
       */
      double getYgap() const {return m_Ygap / m_unit; }

      /** Get number of PMTs in the x row
       * @return number of PMTs in the x row
       */
      int getNpmtx() const {return m_Npmtx; }

      /** Get number of PMTs in the y column
       * @return number of PMTs in the y column
       */
      int getNpmty() const {return m_Npmty; }

      /** Get module side in x direction
       * @return module side in x direction
       */
      double getMsizex() const {return m_Msizex / m_unit; }

      /** Get module side in y direction
       * @return module side y direction
       */
      double getMsizey() const {return m_Msizey / m_unit; }

      /** Get module wall size in in z direction
       * @return module wall size in z direction
       */
      double getMsizez() const {return m_Msizez / m_unit; }

      /** Get module wall thickness
       * @return module wall thickness
       */
      double getMWallThickness() const {return m_MWallThickness / m_unit; }

      /** Get active area size in x direction
       * @return active area size in x direction
       */
      double getAsizex() const {return m_Asizex / m_unit; }

      /** Get active area size in y direction
       * @return active area size in y direction
       */
      double getAsizey() const {return m_Asizey / m_unit; }

      /** Get active area size in z direction
       * @return active area size in z direction
       */
      double getAsizez() const {return m_Asizez / m_unit; }

      /** Get thickness of the PMT window
       * @return  window thickness
       */
      double getWinthickness() const {return m_Winthickness / m_unit; }

      /** Get thickness of PMT bottom
       * @return bottom thickness
       */
      double getBotthickness() const {return m_Botthickness / m_unit; }

      /** Get number of pads in x direction
       * @return number of pads in x direction
       */
      int getNpadx() const {return m_Npadx; }

      /** Get number of pads in y direction
       * @return number of pads in y direction
       */
      int getNpady() const {return m_Npady; }

      /** Get pad size in x direction
       * @return pad size in x
       */
      double getPadx() const {return m_padx; }

      /** Get pad size in y direction
       * @return pad size in x
       */
      double getPady() const {return m_pady; }

      /** Get the thickness of the glue between the PMT stack and the wedge
       * @return the thickness of the glue between the PMT stack and the wedge
       */
      double getdGlue() const {return m_dGlue / m_unit; }

      /** Get offset of PMT array
       * @return offset in x
       */
      double getPMToffsetX() const {return m_pmtOffsetX / m_unit;}

      /** Get offset of PMT array
       * @return offset in y
       */
      double getPMToffsetY() const {return m_pmtOffsetY / m_unit;}

      /** Get number of TDC bits
       * @return number of TDC bits
       */
      int getTDCbits() const {return m_NTDC; }

      /** Get width of TDC bit
       * @return width of TDC bit in ns
       */
      double getTDCbitwidth() const {return m_TDCwidth; }

      /** Get electronic jitter
       * @return rms of electronic jitter in ns
       */
      double getELjitter() const {return m_ELjitter; }

      /** Get electronic efficiency
       * @return efficiency
       */
      double getELefficiency() const {return m_ELefficiency; }

      /** Get number of Gaussian terms for TTS distribution
       * @return number of Gaussian terms
       */
      int getNgaussTTS() const {return m_NgaussTTS; }

      /** Get fraction of Gaussian term i
       * @return fraction i
       */
      double getTTSfrac(int i) const {return m_TTSfrac[i]; }

      /** Get mean of Gaussian term i
       * @return mean i
       */
      double getTTSmean(int i) const {return m_TTSmean[i]; }

      /** Get sigma of Gaussian term i
       * @return sigma i
       */
      double getTTSsigma(int i) const {return m_TTSsigma[i]; }

      /** Get PMT collection efficiency
       * @return collection efficiency
       */
      double getColEffi() const {return m_ColEffi; }

      /** Get lambda of the first QE data point
       * @return lambda min [nm]
       */
      double getLambdaFirst() const {return m_LambdaFirst; }

      /** Get lambda step [nm]
       * @return step
       */
      double getLambdaStep() const {return m_LambdaStep; }

      /** Get number of QE data points
       * @return number of data points
       */
      int getNpointsQE() const {return m_NpointsQE; }

      /** Get quantum efficiency point i
       * @return quantum efficiency point i
       */
      double getQE(int i) const {return m_QE[i]; }

      /** Get quantum efficiency for photon energy e [eV] (linear interpolation)
       * @return quantum efficiency at e
       */
      double QE(double e) const;

      //! Mirror parameters

      /** Get radius of spherical mirror
       * @return radius of spherical mirror
       */
      double getMirradius() const {return m_Mirradius / m_unit; }

      /** Get the dispacement along quartz width for mirror axis
       * @return dispacement along quartz width for mirror axis
       */
      double getMirposx() const {return m_Mirposx / m_unit; }

      /** Get the dispacement along quartz thickness for mirror axis
       * @return dispacement along quartz thickness for mirror axis
       */
      double getMirposy() const {return m_Mirposy / m_unit; }

      /** Get mirror layer thickness
       * @return mirror layer thickness
       */
      double getMirthickness() const {return m_Mirthickness / m_unit; }

      //! Support parameters

      /** Get thickness of the QBB pannel
       * @return thicness of QBB pannel
       */
      double getPannelThickness() const {return m_PannelThickness / m_unit; }

      /** Get thickness of the sinde plate
       * @return thickness of side pladte
       */
      double getPlateThickness() const {return m_PlateThickness / m_unit; }

      /** Get size of air gap between quartz and support at inner radious
       * @return air gar
       */
      double getLowerGap() const {return m_LowerGap / m_unit; }

      /** Get size of air gap between quartz and support at outer radious
       * @return air gar
       */
      double getUpperGap() const {return m_UpperGap / m_unit; }

      /** Get size of air gap between quartz and support (side plate)
       * @return air gar
       */
      double getSideGap() const {return m_SideGap / m_unit; }

      /** Get forward possition of support
       * @return forward possition of support
       */
      double getZForward() const {return m_ZForward / m_unit; }

      /** Get backward possition of support
       * @return backward possition of support
       */
      double getZBackward() const {return m_ZBackward / m_unit; }

      /** Calculate the channel ID from local possition on the PMT
       * @param position on the sensitive detector plane
       * @param ID of the PMT
       * @return ID of channel for whole bar
       */
      int getChannelID(double x, double y, int pmtID) const;

      /** Ger backwads possition
       *@return backward possition of the quartz bar
       */
      double getZ1() const {return (m_Bposition  - m_Gwidth1) / m_unit; }

      /** Get forward possition
       * @return forward possition of the bar (segmen 1+2+3 + glues)
       */
      double getZ2() const {
        return (m_Bposition + m_Length1 + m_Gwidth2 + m_Length2 + m_Gwidth3 + m_Length3)
               / m_unit;
      }

      /** Get Alignment for given component from the database
       * @param  component Name of the component to align
       * @return Transformation matrix for component, idendity if component
       *         could not be found
       */
      G4Transform3D getAlignment(const std::string& component);


    private:

      /**< Constructor */
      TOPGeometryPar();

      /**< Generic clear function */
      void clear(void);

      //! Quartz bar parameters

      int m_Nbars;          /**< number of bars */
      double m_Radius;      /**< distance to inner surface */
      double m_phi0;        /**< azimuthal angle of first bar */
      double m_Qwidth;      /**< bar width */
      double m_Qthickness;  /**< bar thickness */
      double m_Bposition;   /**< bar backward position */
      double m_Length1;     /**< length of the first bar segment (backward) */
      double m_Length2;     /**< length of the second bar segment (middle) */
      double m_Length3;     /**< length of the third bar segment (forward) */
      double m_WLength;     /**< wedge length */
      double m_Wwidth;      /**< wedge width */
      double m_Wextdown;    /**< wedge extension down */
      double m_Wflat;       /**< length of flat portion of wedge at exit window */
      double m_Gwidth1;     /**< glue thickness between wedge and segment 1 */
      double m_Gwidth2;     /**< glue thickness between segments 1 and 2 */
      double m_Gwidth3;     /**< glue thickness between segment 3 and mirror */
      double m_SigmaAlpha;  /**< surface roughness of quartz */
      std::vector<TOPQbar> m_bars; /**< geometry of the bars */

      //! PMT parameters

      int m_Npmtx;                /**< number of PMT columns  */
      int m_Npmty;                /**< number of PMT rows */
      double m_Msizex;            /**< PMT size in x */
      double m_Msizey;            /**< PMT size in y */
      double m_Msizez;            /**< PMT wall size in z */
      double m_MWallThickness;    /**< PMT wall thickness */
      double m_Xgap;              /**< gap between PMTs in x direction */
      double m_Ygap;              /**< gap between PMTs in y direction */
      double m_Asizex;            /**< active area in x */
      double m_Asizey;            /**< active area in y */
      double m_Asizez;            /**< active area thickness */
      double m_Winthickness;      /**< window thickness */
      double m_Botthickness;      /**< window thickness */
      int m_Npadx;                /**< number of pads in x */
      int m_Npady;                /**< number of pads in y */
      double m_padx;              /**< pad size in x */
      double m_pady;              /**< pad size in y */
      double m_AsizexHalf;        /**< active area half size in x */
      double m_AsizeyHalf;        /**< active area half size in y */
      double m_dGlue;             /**< PMT wedge glue thickness */
      double m_pmtOffsetX;        /**< PMT array offset in x */
      double m_pmtOffsetY;        /**< PMT array offset in y */

      //! TDC parameters

      int m_NTDC;                 /**< number of TDC bits */
      double m_TDCwidth;          /**< width of a bit in [ns] */

      //! electronics jitter
      double m_ELjitter;          /**< rms of electronic jitter */
      double m_ELefficiency;      /**< electronic efficiency */

      //! time transition spread (TTS) defined as a sum of Gaussian terms

      int m_NgaussTTS;                 /**< number of gaussian terms */
      double m_TTSfrac[MAXPTS_TTS];    /**< fractions */
      double m_TTSmean[MAXPTS_TTS];    /**< mean's */
      double m_TTSsigma[MAXPTS_TTS];   /**< sigma's */

      //! Quantum & collection efficiencies

      double m_ColEffi;           /**< collection efficiency */
      double m_LambdaFirst;       /**< wavelength [nm]: first QE data point */
      double m_LambdaStep;        /**< wavelength [nm]: step */
      int m_NpointsQE;            /**< number of QE data points */
      double m_QE[MAXPTS_QE];     /**< quantum efficiency times filter transmission*/

      //! Mirror parameters (spherical mirror)

      double m_Mirradius;     /**< radius  */
      double m_Mirposx;       /**< center of curvature in x */
      double m_Mirposy;       /**< center of curvature in y */
      double m_Mirthickness;  /**< reflective layer thickness */

      //! Support structure parameters

      double m_PannelThickness;   /**< thicness of QBB pannel */
      double m_PlateThickness;    /**< thickness of side pladte */
      double m_LowerGap;          /**< lower air gap */
      double m_UpperGap;          /**< upper air gap */
      double m_SideGap;           /**< side air gap */
      double m_ZForward;          /**< forward possition of support */
      double m_ZBackward;         /**< backward possition of support */

      //! Other

      static TOPGeometryPar* p_B4TOPGeometryParDB;  /**< Pointer to class instance */
      GearDir m_alignment;                          /**< GearDir of alignment */
      double m_unit;                                /**< conversion unit for length */

    };

  } //! end of namespace TOP
} //! end of namespace Belle2
#endif
