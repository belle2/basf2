/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric, Marko Staric                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <map>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <cmath>
#include <G4Transform3D.hh>
#include <G4ThreeVector.hh>
#include <G4RotationMatrix.hh>

#include <top/dbobjects/TOPGeometry.h>
#include <top/geometry/TOPQbar.h>
#include <top/geometry/FrontEndMapper.h>
#include <top/geometry/ChannelMapper.h>

#include <vector>

namespace Belle2 {
  namespace TOP {

    /**
     * Class for TOP Geometry Parameters.
     * To pass the parameters from .xml files to geometry builder and any other module.
     * Usage of this class asures that all parts of TOP obtain the same information.
     * All acces to .xml files should be made through usage of this class.
     */

    class TOPGeometryPar {

    public:

      /**
       * Various constants
       */
      enum {c_WindowSize = 64, /**< number of samples per ASIC window */
            c_syncWindows = 2  /**< number of windows corresponding to timeBase */
           };

      /**
       * Destructor
       */
      virtual ~TOPGeometryPar() {s_instance = 0;}

      /**
       * Static method to obtain a pointer to its instance.
       * @return pointer to the instance of this class.
       */
      static TOPGeometryPar* Instance();

      /**
       * Initialize from Gearbox
       * @param content GearBox directory where .xml are stored
       */
      void Initialize(const GearDir& content);

      /**
       * check if the parameters are read from Gearbox
       * @return true if done
       */
      bool isInitialized() const {return m_initialized;}

      /**
       * to return parameters in the default units
       */
      void setBasfUnits() {m_unit = Unit::cm;}

      /**
       * to return parameters in Gean units
       */
      void setGeanUnits() {m_unit = Unit::mm;}

      /**
       * Sets electronic time jitter
       * @param jitter rms of electronic time jitter [ns]
       */
      void setELjitter(double jitter) {m_ELjitter = jitter;}

      /**
       * Sets electronic efficiency
       * @param effi efficiency
       */
      void setELefficiency(double effi) {m_ELefficiency = effi;}

      /**
       * Returns number of modules
       * @return number of modules
       */
      int getNbars() const {return m_Nbars;}

      /**
       * Returns distance from the origin to the inner surface of the quartz bar
       * @return distance
       */
      double getRadius() const {return m_Radius / m_unit;}

      /**
       * Returns azimuthal angle of the first bar
       * @return azimuthal angle
       */
      double getPhi0() const {return m_phi0;}

      /**
       * Returns the width of the quartz bar
       *@return width of quartz bar
       */
      double getQwidth() const {return m_Qwidth / m_unit; }

      /**
       * Returns the thickness of the quartz bar
       * @return thickness of quartz bar
       */
      double getQthickness() const {return m_Qthickness / m_unit; }

      /**
       * Returns backward position of the quartz bar (prism-bar connection)
       * @return barward possition on quartz bar
       */
      double getBposition() const {return m_Bposition / m_unit; }

      /**
       * Returns length of first quartz bar segment.
       * @return length of first quartz bar segment
       */
      double getLength1() const {return m_Length1 / m_unit; }

      /**
       * Returns length of second quartz bar segment.
       * @return length of second quartz bar segment
       */
      double getLength2() const {return m_Length2 / m_unit; }

      /**
       * Returns length of third quartz bar segment.
       * @return length of third quartz bar segment
       */
      double getLength3() const {return m_Length3 / m_unit; }

      /**
       * Returns length of the wedge (prism)
       * @return length of wedge
       */
      double getWLength() const {return m_WLength / m_unit; }

      /**
       * Returns width of the wedge (prism)
       * @return width of wedge
       */
      double getWwidth() const {return m_Wwidth / m_unit; }

      /**
       * Returns wedge extension down measured from the inner bar surface
       * @return wedge extension down
       */
      double getWextdown() const {return m_Wextdown / m_unit; }

      /**
       * Returns length of the wedge flat part at exit window
       * @return length of the flat part
       */
      double getWflat() const {return m_Wflat / m_unit; }

      /**
       * Returns width of glue between wedge and bar segment 1
       * @return width of the glue joint 1
       */
      double getGwidth1() const {return m_Gwidth1 / m_unit; }

      /**
       * Returns width of glue between bar segment 1 and 2
       * @return width of the glue joint 2
       */
      double getGwidth2() const {return m_Gwidth2 / m_unit; }

      /**
       * Returns width of glue between bar segment 2 and segment 3
       * @return width of the glue joint 3
       */
      double getGwidth3() const {return m_Gwidth3 / m_unit; }

      /**
       * Returns sigma alpha parameter describing surface roughness of quartz
       * @return sigma alpha of quartz
       */
      double getSigmaAlpha() const {return m_SigmaAlpha; }

      /**
       * Returnss Q-bar geometry object
       * @param ID Q-bar ID
       * @return pointer to Q-bar or NULL
       */
      const TOPQbar* getQbar(int ID) const
      {
        if (ID < 1 || ID > (int) m_bars.size()) return NULL;
        return &m_bars[ID - 1];
      }

      /**
       * Returns gap between PMTs in x direction
       * @return gap between PMTs in x direction
       */
      double getXgap() const {return m_Xgap / m_unit; }

      /**
       * Returns gap between PMTs in y direction
       * @return gap between PMTs in y direction
       */
      double getYgap() const {return m_Ygap / m_unit; }

      /**
       * Returns number of PMT rows
       * @return number of PMT rows
       */
      int getNpmtx() const {return m_Npmtx; }

      /**
       * Returns number of PMT columns
       * @return number of PMT columns
       */
      int getNpmty() const {return m_Npmty; }

      /**
       * Returns PMT size in x direction
       * @return size in x direction
       */
      double getMsizex() const {return m_Msizex / m_unit; }

      /**
       * Returns PMT size in y direction
       * @return size y direction
       */
      double getMsizey() const {return m_Msizey / m_unit; }

      /**
       * Returns PMT wall size in in z direction
       * @return wall size in z direction
       */
      double getMsizez() const {return m_Msizez / m_unit; }

      /**
       * Returns PMT wall thickness
       * @return wall thickness
       */
      double getMWallThickness() const {return m_MWallThickness / m_unit; }

      /**
       * Returns PMT active area size in x direction
       * @return active area size in x direction
       */
      double getAsizex() const {return m_Asizex / m_unit; }

      /**
       * Returns PMT active area size in y direction
       * @return active area size in y direction
       */
      double getAsizey() const {return m_Asizey / m_unit; }

      /**
       * Returns PMT active area size in z direction
       * @return active area size in z direction
       */
      double getAsizez() const {return m_Asizez / m_unit; }

      /**
       * Returns thickness of the PMT window
       * @return  window thickness
       */
      double getWinthickness() const {return m_Winthickness / m_unit; }

      /**
       * Returns thickness of PMT bottom
       * @return bottom thickness
       */
      double getBotthickness() const {return m_Botthickness / m_unit; }

      /**
       * Returns number of PMT pads in x direction
       * @return number of pads in x direction
       */
      int getNpadx() const {return m_Npadx; }

      /**
       * Returns number of PMT pads in y direction
       * @return number of pads in y direction
       */
      int getNpady() const {return m_Npady; }

      /**
       * Returns PMT pad size in x direction
       * @return pad size in x
       */
      double getPadx() const {return m_padx; }

      /**
       * Returns PMT pad size in y direction
       * @return pad size in x
       */
      double getPady() const {return m_pady; }

      /**
       * Returns the thickness of the glue between the PMT stack and the wedge
       * @return the thickness of the glue between the PMT stack and the wedge
       */
      double getdGlue() const {return m_dGlue / m_unit; }

      /**
       * Returns offset of PMT array in x
       * @return offset in x
       */
      double getPMToffsetX() const {return m_pmtOffsetX / m_unit;}

      /**
       * Returns offset of PMT array in y
       * @return offset in y
       */
      double getPMToffsetY() const {return m_pmtOffsetY / m_unit;}

      /**
       * Returns number of TDC bits
       * @return number of TDC bits
       */
      int getTDCbits() const {return m_NTDC; }

      /**
       * Returns number of bits per ASIC sample
       * @return number of sub-bits
       */
      int getSubBits() const {return m_subBits;}

      /**
       * Returns number of ASIC windows per waveform
       * @return number of windows
       */
      int getNumWindows() const {return m_numWindows;}

      /**
       * Returns synchronization time with accelerator for IRSX readout
       * @return synchronization time
       */
      double getSyncTimeBase() const {return m_syncTimeBase;}

      /**
       * Returns width of TDC bin
       * @return width of TDC bin [ns]
       */
      double getTDCbitwidth() const {return m_TDCwidth; }

      /**
       * Returns TDC offset (see getTime(int TDC) for the definition)
       * @return offset [ns]
       */
      double getTDCoffset() const {return m_TDCoffset; }

      /**
       * Returns pile-up time relevant for CF discriminator
       * @return pile-up time
       */
      double getPileupTime() const {return m_pileupTime;}

      /**
       * Returns double-hit resolution relevant for CF discriminator
       * @return double-hit resolution time
       */
      double getDoubleHitResolution() const {return m_doubleHitResolution;}

      /**
       * Returns TDC overflow value
       * @return overflow value
       */
      int TDCoverflow() const {return 1 << m_NTDC;}

      /**
       * Convert TDC count to time
       * @param TDC TDC count
       * @return time [ns]
       */
      double getTime(int TDC) const {return (TDC + 0.5) * m_TDCwidth - m_TDCoffset;}

      /**
       * Convert time to TDC count.
       * For times being outside TDC range, TDC overflow value is returned.
       * @param time [ns]
       * @return TDC TDC count
       */
      int getTDCcount(double time) const
      {
        time += m_TDCoffset;
        if (time < 0) return TDCoverflow();
        if (time > TDCoverflow() * m_TDCwidth) return TDCoverflow();
        return int(time / m_TDCwidth);
      }

      /**
       * Returns electronic jitter
       * @return rms of electronic jitter in ns
       */
      double getELjitter() const {return m_ELjitter; }

      /**
       * Returns electronic efficiency
       * @return efficiency
       */
      double getELefficiency() const {return m_ELefficiency; }

      /**
       * Returns number of Gaussian terms for TTS distribution
       * @return number of Gaussian terms
       */
      int getNgaussTTS() const {return m_TTSfrac.size(); }

      /**
       * Returns fraction of Gaussian term i
       * @return fraction i
       */
      double getTTSfrac(int i) const {return m_TTSfrac[i]; }

      /**
       * Returns mean of Gaussian term i
       * @return mean i
       */
      double getTTSmean(int i) const {return m_TTSmean[i]; }

      /**
       * Returns sigma of Gaussian term i
       * @return sigma i
       */
      double getTTSsigma(int i) const {return m_TTSsigma[i]; }

      /**
       * Returns PMT collection efficiency
       * @return collection efficiency
       */
      double getColEffi() const {return m_ColEffi; }

      /**
       * Returns lambda of the first QE data point
       * @return lambda min [nm]
       */
      double getLambdaFirst() const {return m_LambdaFirst; }

      /**
       * Returns lambda step [nm]
       * @return step
       */
      double getLambdaStep() const {return m_LambdaStep; }

      /**
       * Returns number of QE data points
       * @return number of data points
       */
      int getNpointsQE() const {return m_QE.size(); }

      /**
       * Returns quantum efficiency point i
       * @return quantum efficiency point i
       */
      double getQE(int i) const {return m_QE[i]; }

      /**
       * Returns quantum efficiency for photon energy e [eV] (linear interpolation)
       * @return quantum efficiency at e
       */
      double QE(double e) const;

      /**
       * Returns radius of spherical mirror
       * @return radius of spherical mirror
       */
      double getMirradius() const {return m_Mirradius / m_unit; }

      /**
       * Returns the dispacement along quartz width for mirror axis
       * @return dispacement along quartz width for mirror axis
       */
      double getMirposx() const {return m_Mirposx / m_unit; }

      /**
       * Returns the dispacement along quartz thickness for mirror axis
       * @return dispacement along quartz thickness for mirror axis
       */
      double getMirposy() const {return m_Mirposy / m_unit; }

      /**
       * Returns mirror layer thickness
       * @return mirror layer thickness
       */
      double getMirthickness() const {return m_Mirthickness / m_unit; }

      /**
       * Returns thickness of the QBB pannel
       * @return thicness of QBB pannel
       */
      double getPannelThickness() const {return m_PannelThickness / m_unit; }

      /**
       * Returns thickness of the sinde plate
       * @return thickness of side pladte
       */
      double getPlateThickness() const {return m_PlateThickness / m_unit; }

      /**
       * Returns size of air gap between quartz and support at inner radious
       * @return air gar
       */
      double getLowerGap() const {return m_LowerGap / m_unit; }

      /**
       * Returns size of air gap between quartz and support at outer radious
       * @return air gar
       */
      double getUpperGap() const {return m_UpperGap / m_unit; }

      /**
       * Returns size of air gap between quartz and support (side plate)
       * @return air gar
       */
      double getSideGap() const {return m_SideGap / m_unit; }

      /**
       * Returns forward possition of support
       * @return forward possition of support
       */
      double getZForward() const {return m_ZForward / m_unit; }

      /**
       * Returns backward possition of support
       * @return backward possition of support
       */
      double getZBackward() const {return m_ZBackward / m_unit; }

      /**
       * Returns backwads possition
       *@return backward possition of the quartz bar
       */
      double getZ1() const {return (m_Bposition  - m_Gwidth1) / m_unit; }

      /**
       * Returns forward possition
       * @return forward possition of the bar (segmen 1+2+3 + glues)
       */
      double getZ2() const
      {
        return (m_Bposition + m_Length1 + m_Gwidth2 + m_Length2 + m_Gwidth3 + m_Length3)
               / m_unit;
      }

      /**
       * Returns alignment for given component from the database
       * @param  component Name of the component to align
       * @return Transformation matrix for component, idendity if component
       *         could not be found
       */
      G4Transform3D getAlignment(const std::string& component);

      /**
       * Calculate pixel ID from local possition on the PMT
       * @param x position x on the sensitive detector plane
       * @param y position y on the sensitive detector plane
       * @param pmtID  ID of the PMT
       * @return pixel ID (1-based)
       */
      int getPixelID(double x, double y, int pmtID) const;

      /**
       * Convert new numbering scheme for pixels to the old one
       * @param pixelID pixel ID in the new numbering scheme
       * @return pixel ID in the old numbering scheme
       */
      int getOldNumbering(int pixelID) const;

      /**
       * Convert old numbering scheme for pixels to the new one
       * @param pixelID pixel ID in the old numbering scheme
       * @return pixel ID in the new numbering scheme
       */
      int getNewNumbering(int pixelID) const;

      /**
       * Returns front-end mapper (mapping of SCROD's to positions within TOP modules)
       * @return  front-end mapper object
       */
      const FrontEndMapper& getFrontEndMapper() const {return m_frontEndMapper;}

      /**
       * Returns channel mapper (mapping of channels to pixels)
       * @return  channel mapper object
       */
      const ChannelMapper& getChannelMapper(ChannelMapper::EType type =
                                              ChannelMapper::c_IRSX) const
      {
        switch (type) {
          case ChannelMapper::c_IRS3B: return m_channelMapperIRS3B;
          case ChannelMapper::c_IRSX:  return m_channelMapperIRSX;
          default: return m_channelMapperIRSX;
        }
      }

      /**
       * sets a pointer to geometry object
       * @param geo pointer to geometry object
       */
      void setGeometry(const TOPGeometry* geo) {m_geo = geo;}

      /**
       * Returns pointer to geometry object
       * @return pointer to geometry object
       */
      const TOPGeometry* getGeometry() const {return m_geo;}

    private:

      /**
       * Hidden constructor since it is a singleton class
       */
      TOPGeometryPar()
      {}

      /**
       * Reads geometry parameters from gearbox
       * @param content GearBox directory where .xml are stored
       */
      void read(const GearDir& content);

      // Quartz bar parameters

      int m_Nbars = 0;          /**< number of bars */
      double m_Radius = 0;      /**< distance to inner surface */
      double m_phi0 = 0;        /**< azimuthal angle of first bar */
      double m_Qwidth = 0;      /**< bar width */
      double m_Qthickness = 0;  /**< bar thickness */
      double m_Bposition = 0;   /**< bar backward position */
      double m_Length1 = 0;     /**< length of the first bar segment (backward) */
      double m_Length2 = 0;     /**< length of the second bar segment (middle) */
      double m_Length3 = 0;     /**< length of the third bar segment (forward) */
      double m_WLength = 0;     /**< wedge length */
      double m_Wwidth = 0;      /**< wedge width */
      double m_Wextdown = 0;    /**< wedge extension down */
      double m_Wflat = 0;       /**< length of flat portion of wedge at exit window */
      double m_Gwidth1 = 0;     /**< glue thickness between wedge and segment 1 */
      double m_Gwidth2 = 0;     /**< glue thickness between segments 1 and 2 */
      double m_Gwidth3 = 0;     /**< glue thickness between segment 3 and mirror */
      double m_SigmaAlpha = 0;  /**< surface roughness of quartz */
      std::vector<TOPQbar> m_bars; /**< geometry of the bars */

      // PMT parameters

      int m_Npmtx = 0;                /**< number of PMT columns  */
      int m_Npmty = 0;                /**< number of PMT rows */
      double m_Msizex = 0;            /**< PMT size in x */
      double m_Msizey = 0;            /**< PMT size in y */
      double m_Msizez = 0;            /**< PMT wall size in z */
      double m_MWallThickness = 0;    /**< PMT wall thickness */
      double m_Xgap = 0;              /**< gap between PMTs in x direction */
      double m_Ygap = 0;              /**< gap between PMTs in y direction */
      double m_Asizex = 0;            /**< active area in x */
      double m_Asizey = 0;            /**< active area in y */
      double m_Asizez = 0;            /**< active area thickness */
      double m_Winthickness = 0;      /**< window thickness */
      double m_Botthickness = 0;      /**< window thickness */
      int m_Npadx = 0;                /**< number of pads in x */
      int m_Npady = 0;                /**< number of pads in y */
      double m_padx = 0;              /**< pad size in x */
      double m_pady = 0;              /**< pad size in y */
      double m_AsizexHalf = 0;        /**< active area half size in x */
      double m_AsizeyHalf = 0;        /**< active area half size in y */
      double m_dGlue = 0;             /**< PMT wedge glue thickness */
      double m_pmtOffsetX = 0;        /**< PMT array offset in x */
      double m_pmtOffsetY = 0;        /**< PMT array offset in y */

      // TDC parameters

      int m_NTDC = 0;                 /**< number of TDC bits */
      int m_numWindows = 0;           /**< number of ASIC windows per waveform */
      int m_subBits = 0;              /**< number of bits per ASIC sample */
      double m_syncTimeBase = 0;      /**< sinchronization time base [ns] */
      double m_TDCwidth = 0;          /**< width of a bit in [ns] */
      double m_TDCoffset = 0;         /**< offset to be subtracted [ns] */
      double m_pileupTime = 0;        /**< pile-up time [ns] */
      double m_doubleHitResolution = 0; /**< double hit resolution time [ns] */

      // electronics jitter and efficiency

      double m_ELjitter = 0;          /**< rms of electronic jitter */
      double m_ELefficiency = 1.0;      /**< electronic efficiency */

      // time transition spread (TTS) defined as a sum of Gaussian terms

      std::vector<double> m_TTSfrac;    /**< fractions */
      std::vector<double> m_TTSmean;    /**< mean's */
      std::vector<double> m_TTSsigma;   /**< sigma's */

      // Quantum & collection efficiencies

      double m_ColEffi = 0;           /**< collection efficiency */
      double m_LambdaFirst = 0;       /**< wavelength [nm]: first QE data point */
      double m_LambdaStep = 0;        /**< wavelength [nm]: step */
      std::vector<double> m_QE;     /**< quantum efficiency times filter transmission*/

      // Mirror parameters (spherical mirror)

      double m_Mirradius = 0;     /**< radius  */
      double m_Mirposx = 0;       /**< center of curvature in x */
      double m_Mirposy = 0;       /**< center of curvature in y */
      double m_Mirthickness = 0;  /**< reflective layer thickness */

      // Support structure parameters

      double m_PannelThickness = 0;   /**< thicness of QBB pannel */
      double m_PlateThickness = 0;    /**< thickness of side pladte */
      double m_LowerGap = 0;          /**< lower air gap */
      double m_UpperGap = 0;          /**< upper air gap */
      double m_SideGap = 0;           /**< side air gap */
      double m_ZForward = 0;          /**< forward possition of support */
      double m_ZBackward = 0;         /**< backward possition of support */

      // Mappings

      FrontEndMapper m_frontEndMapper; /**< front end electronics mapper */
      ChannelMapper m_channelMapperIRS3B; /**< channel-pixel mapper */
      ChannelMapper m_channelMapperIRSX;  /**< channel-pixel mapper */

      // Other

      GearDir m_alignment;            /**< GearDir of alignment */
      double m_unit = Unit::cm;       /**< conversion unit for length */
      bool m_initialized = false;     /**< true if Initialize() called */
      const TOPGeometry* m_geo = 0;   /**< geometry parameters */

      static TOPGeometryPar* s_instance;  /**< Pointer to class instance */

    };

  } // end of namespace TOP
} // end of namespace Belle2
