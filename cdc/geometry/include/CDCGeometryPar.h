/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/gearbox/GearDir.h>
#include <framework/database/DBArray.h>
#include <framework/database/DBObjPtr.h>
#include <framework/dbobjects/HardwareClockSettings.h>
#include <framework/geometry/B2Vector3.h>

#include <cdc/dataobjects/WireID.h>
#include <cdc/dbobjects/CDCTimeZeros.h>
#include <cdc/dbobjects/CDCBadWires.h>
#include <cdc/dbobjects/CDCPropSpeeds.h>
#include <cdc/dbobjects/CDCTimeWalks.h>
#include <cdc/dbobjects/CDCXtRelations.h>
#include <cdc/dbobjects/CDCSpaceResols.h>
#include <cdc/dbobjects/CDCFudgeFactorsForSigma.h>
#include <cdc/dbobjects/CDCChannelMap.h>
#include <cdc/dbobjects/CDCDisplacement.h>
#include <cdc/dbobjects/CDCAlignment.h>
#include <cdc/dbobjects/CDCMisalignment.h>
#include <cdc/dbobjects/CDCGeometry.h>
#include <cdc/dbobjects/CDCEDepToADCConversions.h>

#include <cdc/geometry/CDCGeometryParConstants.h>

#include <vector>
#include <string>
#include <map>
//#include <fstream>


namespace Belle2 {
  namespace CDC {

    //! The Class for CDC Geometry Parameters
    /*! This class provides CDC gemetry paramters for simulation, reconstruction and so on.
        These parameters are gotten from gearbox.
    */
    class CDCGeometryPar {

    public:

      //! Wire position set
      enum EWirePosition {c_Base = 0, c_Misaligned, c_Aligned};

      //! Destructor
      virtual ~CDCGeometryPar();

      //! Static method to get a reference to the CDCGeometryPar instance.
      /*!
      \return A reference to an instance of this class.
      */
      static CDCGeometryPar& Instance(const CDCGeometry* = nullptr);

      //! Clears
      void clear();

      //! Print some debug information
      void Print() const;

      //! Gets geometry parameters from database.
      void readFromDB(const CDCGeometry&);

      //      /**
      //       * Read z-corrections.
      //       * @param GearDir Gear Dir.
      //       */
      //      void readDeltaz(const GearDir);
      //
      //      /**
      //       * Read z-corrections from DB.
      //       *
      //       */
      //      void readDeltaz(const CDCGeometry&);

      //      /**
      //       * Open a file
      //       * @param[in] ifs input file-stream
      //       * @param[in] fileName0 file-name on cdc/data directory
      //       */
      //      void openFile(std::ifstream& ifs, const std::string& fileName0) const;

      /**
       * Read displacement or (mis)alignment params from text file.
       * @param[in] set Wire position set, i.e. c_Base, c_Misaliged or c_Aligned.
       * @param[in] geom Pointer to DB CDCGeometry db object.
       */
      //      void readWirePositionParams(EWirePosition set, const CDCGeometry*,  const GearDir);
      void readWirePositionParams(EWirePosition set, const CDCGeometry* geom);

      /**
       * Set wire alignment params. from DB.
       */
      void setWirPosAlignParams();

      /**
       * Set wire misalignment params. from DB.
       */
      void setWirPosMisalignParams();

      /**
       * Read XT-relation table.
       * @param[in] gbxParams Gear Dir.
       * @param[in] mode 0: read simulation file, 1: read reconstruction file.
       */
      void readXT(const GearDir& gbxParams, int mode = 0);

      /**
       * Read XT-relation table in new format.
       * @param[in] gbxParams Gear Dir.
       * @param[in] mode 0: read simulation file, 1: read reconstruction file.
       */
      void newReadXT(const GearDir& gbxParams, int mode = 0);

      /**
       * Set XT-relation table (from DB).
       */
      void setXT();

      /**
       * Set XT-relation table (from DB) (new).
       */
      void setXtRel();

      /**
       * Read spatial resolution table.
       * @param gbxParams Gear Dir.
       * @param mode 0: read simulation file, 1: read reconstruction file.
       */
      void readSigma(const GearDir& gbxParams, int mode = 0);

      /**
       * Read spatial resolution table in new format.
       * @param gbxParams Gear Dir.
       * @param mode 0: read simulation file, 1: read reconstruction file.
       */
      void newReadSigma(const GearDir& gbxParams, int mode = 0);

      /**
       * Read fudge factors
       * @param gbxParams Gear Dir.
       * @param mode dummy now.
       */
      void readFFactor(const GearDir& gbxParams, int mode = 0);

      /**
       * Set spatial resolution (from DB).
       */
      void setSResol();

      /**
       * Set fudge factors (from DB).
       */
      void setFFactor();

      /**
       * Read the propagation speed along the sense wire.
       * @param gbxParams Gear Dir.
       * @param mode 0: read simulation file, 1: read reconstruction file.
       */
      void readPropSpeed(const GearDir& gbxParams, int mode = 0);

      /**
       * Set prop.-speeds (from DB).
       */
      void setPropSpeed();

      /**
       * Read t0 parameters (from a file).
       * @param gbxParams Gear Dir.
       * @param mode 0: read simulation file, 1: read reconstruction file.
       */
      void readT0(const GearDir& gbxParams, int mode = 0);

      /**
       * Set t0 parameters (from DB)
       */
      void setT0();

      /**
       * Calculate mean t0 in ns (over all good wires)
       * @param minT0 min. of t0 window (ns)
       * @param maxT0 max. of t0 window (ns)
       * @param maxIt max. no. of iterations
       * @param nStdv standard-deviation cut applied for next iteration
       * @param epsi  criterion for iteration stop (ns)
       */
      void calcMeanT0(double minT0 = 3800, double maxT0 = 5800, int maxIt = 10, double nStdv = 3, double epsi = 0.1);

      //      /**
      //       * Read bad-wires (from a file).
      //       * @param GearDir Gear Dir.
      //       * @param mode 0: read simulation file, 1: read reconstruction file.
      //       */
      //      void readBadWire(const GearDir, int mode = 0);

      /**
       * Set bad-wires (from DB)
       */
      void setBadWire();

      /**
       * Read channel map between wire-id and electronics-id.
       */
      //      void readChMap(const GearDir);
      void readChMap();

      /**
       * Set channel map (from DB)
       */
      void setChMap();

      /**
       * Read time-walk parameter.
       * @param gbxParams Gear Dir.
       * @param mode 0: read simulation file, 1: read reconstruction file.
       */
      void readTW(const GearDir& gbxParams, int mode = 0);

      /**
       * Read spatial edep-to-adc conv. factors.
       * @param gbxParams Gear Dir.
       * @param mode dummy now..
       */
      void readEDepToADC(const GearDir& gbxParams, int mode = 0);

      /**
       * Set time-walk parameters.
       */
      void setTW();

      /** Set edep-to-ADC conversion params. (from DB) */
      void setEDepToADCConversions();

      /** Return edep-to-ADC conversion factor.
       * @param layer no. (0-55)
       * @param cell  no. (0-)
       * @param edep  energy-deposit in the cell(keV).
       * @param dx    path length (cm) of the track in the cell.
       * @param costh cos(theta) of track.
       */
      double getEDepToADCConvFactor(unsigned short layer, unsigned short cell, double edep, double dx, double costh);


      /** Return edep-to-ADC conversion main factor (in count/keV)
       * @param layer no. (0-55)
       * @param cell  no. (0-)
       * @param costh cosine of incident angle (theta) of particle
       */
      double getEDepToADCMainFactor(unsigned short layer, unsigned short cell, double costh = 0)
      {
        return m_eDepToADCParams[layer][cell][0] + m_eDepToADCParams[layer][cell][4] * (costh - m_eDepToADCParams[layer][cell][5]);
      };

      /** Return sigma for extra smearing of edep to ADC conversion
       * @param layer no. (0-55)
       * @param cell  no. (0-)
       */
      double getEDepToADCSigma(unsigned short layer, unsigned short cell)
      {
        return m_eDepToADCParams[layer][cell][6];
      };

      //! Generate an xml file used in gearbox
      /*!
          \param of The file name.
      */
      void generateXML(const std::string& of);

      //! Returns the version of cdc geometry parameters
      /*!
         \return The version of the cdc geometry parameters.
      */
      std::string version() const;

      //! The method to get cdc mother volume inner R
      /*!
          \return The inner radius of the cdc mother volume.
      */
      double motherInnerR() const;

      //! The method to get cdc mother volume outer R
      /*!
          \return The outer radius of the cdc mother volume.
      */
      double motherOuterR() const;

      //! The method to get cdc mother volume length
      /*!
          \return The length of the cdc mother volume.
      */
      double motherLength() const;

      //! to get the number of boundary position of the CDC mother volume
      /*!
          \return The number of boundary position of the CDC mother volume.
      */
      int momBound() const;

      //! Returns boundary position in Z axis of the CDC mother volume
      /*!
          \param iBound : The boundary id.
          \return The z component of the specified baoundary position in the CDC mother volume
      */
      double momZ(int iBound) const;

      //! Returns inner radius of the CDC mother volume
      /*!
          \param iBound : The boundary id.
          \return The inner radius of the specified baoundary position in the CDC mother volume
      */
      double momRmin(int iBound) const;

      //! The method to get cell id based on given layer id and the position.
      /*!
          \param layerId The given layer id.
          \param position The given position to calculate cell id.
          \return Cell id.
      */
      unsigned cellId(unsigned layerId, const B2Vector3D& position) const;

      //! Returns the inner radius of the outer wall.
      /*!
          \return The inner radius of the outer wall.
      */
      double innerRadiusOuterWall() const;

      //! Returns the outer radius of the outer wall.
      /*!
          \return The outer radius of the outer wall.
      */
      double outerRadiusOuterWall() const;

      //! Returns the length of the outer wall in Z.
      /*!
          \return The length of the outer wall.
      */
      double zOuterWall() const;

      //! Returns the offset of the outer wall in z direction.
      /*!
          \return The z offset of the outer wall.
      */
      double zOffsetOuterWall() const;

      //! Returns the inner radius of the inner wall.
      /*!
          \return The inner radius of the inner wall.
      */
      double innerRadiusInnerWall() const;

      //! Returns the outer radius of the inner wall.
      /*!
          \return The outer radius of the inner wall.
      */
      double outerRadiusInnerWall() const;

      //! Returns the length of the inner wall in Z.
      /*!
          \return The length of the inner wall.
      */
      double zInnerWall() const;

      //! Returns the offset of the outer wall in z direction.
      /*!
          \return The z offset of the outer wall.
      */
      double zOffsetInnerWall() const;

      //! Returns diameter of the sense wire.
      /*!
          \return Diameter of the sense wire.
      */
      double senseWireDiameter() const;

      //! Returns diameter of the field wire.
      /*!
          \return Diameter of the field wire.
      */
      double fieldWireDiameter() const;

      //! Returns a number of wire layers.
      /*!
          \return The number of wire layers.
      */
      unsigned nWireLayers() const;

      //! Returns wire numbers in a layer
      /*!
          \param layerId The layer id.
          \return Wire number in layer layerId.
      */
      unsigned nWiresInLayer(int layerId) const;

      //! Returns an array of inner radius of wire layers.
      /*!
          \return An array of inner radius of wire layers.
      */
      const double* innerRadiusWireLayer() const;

      //! Returns an array of outer radius of wire layers.
      /*!
          \return A array of outer radius of wire layers.
      */
      const double* outerRadiusWireLayer() const;

      //! Returns an array of forward z of wire layers.
      /*!
          \return An array of forward z.
      */
      const double* zForwardWireLayer() const;

      //! Returns an array of backward z of wire layers.
      /*!
          \return An array of backward z.
      */
      const double* zBackwardWireLayer() const;

      //! Returns the offset of z of the wire layer i.
      /*!
          \param i The layer id.
          \return The z offset of wire layer i.
      */
      double zOffsetWireLayer(unsigned i) const;

      //! Returns the mean t0 over all wires.
      /*!
          \return mean t0.
      */
      double getMeanT0() const;

      //! Returns the forward position of the input sense wire.
      /*!
          \param layerId The layer id. of the wire
          \param cellId  The wire id. of the wire
          \param set     Wire position set; =c_Base, c_Misaligned or c_Aligned
          \return The forward position of the wire.
      */
      const B2Vector3D wireForwardPosition(uint layerId, int cellId, EWirePosition set = c_Base) const;

      /** The same function but in a different input format. */
      const B2Vector3D wireForwardPosition(const WireID& wireID, EWirePosition set = c_Base) const
      {
        return wireForwardPosition(wireID.getICLayer(), wireID.getIWire(), set);
      }

      //! Returns a virtual forward position corresp. to a tangent to the wire at the input z-position.
      /*!
      \param layerId The layer id. of the wire
      \param cellId  The wire id. of the wire
      \param z       z-position
      \param set     Wire position set; =c_Base, c_Misaligned or c_Aligned
      \return The virtual forward position of the wire.
      */
      const B2Vector3D wireForwardPosition(uint layerId, int cellId, double z, EWirePosition set = c_Base) const;
      /** The same function but in a different input format. */
      const B2Vector3D wireForwardPosition(const WireID& wireID, double z,
                                           EWirePosition set = c_Base) const
      {
        return wireForwardPosition(wireID.getICLayer(), wireID.getIWire(), z, set);
      }

      //! Returns the backward position of the input sense wire.
      /*!
          \param layerId The layer id. of the wire
          \param cellId  The wire id. of the wire
          \param set     Wire position set; =c_Base, c_Misaligned or c_Aligned
          \return The backward position of the wire.
      */
      const B2Vector3D wireBackwardPosition(uint layerId, int cellId, EWirePosition set = c_Base) const;

      /** The same function but in a different input format. */
      const B2Vector3D wireBackwardPosition(const WireID& wireID, EWirePosition set = c_Base) const
      {
        return wireBackwardPosition(wireID.getICLayer(), wireID.getIWire(), set);
      }

      //! Returns a virtual backward position corresp. to a tangent to the wire at the input z-position
      /*!
      \param layerId The layer id. of the wire
      \param cellId  The wire id. of the wire
      \param z       z-position
      \param set     Wire position set; =c_Base, c_Misaligned or c_Aligned
      \return The virtual backward position of the wire.
      */
      const B2Vector3D wireBackwardPosition(uint layerId, int cellId, double z, EWirePosition set = c_Base) const;
      /** The same function but in a different input format. */
      const B2Vector3D wireBackwardPosition(const WireID& wireID, double z, EWirePosition set = c_Base) const
      {
        return wireBackwardPosition(wireID.getICLayer(), wireID.getIWire(), z, set);
      }

      //! Returns coefficient for the sense wire sag.
      /*!
          \param set     Wire position set; =c_Base, c_Misaligned or c_Aligned
          \param layerId The layer id.
          \param cellId  The cell  id.
          \return Coefficient for the sense wire sag.
      */
      double getWireSagCoef(EWirePosition set, uint layerId, int cellId) const;


      //! Returns threshold for energy deposit in one G4 step
      /*!
      \return threshold (GeV)
      */
      double getThresholdEnerguDeposit() const
      {
        return m_thresholdEnergyDeposit;
      }

      //! Returns the minimum track length required in one G4 step (only secondary particles which pass this criterion are to be saved in MCParticle)
      /*!
      \return length (cm)
      */
      double getMinTrackLength() const
      {
        return m_minTrackLength;
      }

      //! Returns on/off for sense wire sag in FullSim
      /*!
      */
      bool isWireSagOn() const
      {
        return m_wireSag;
      }

      //! Returns on/off for modified left/right calculation in FullSim
      /*!
      */
      bool isModifiedLeftRightFlagOn() const
      {
        return  m_modLeftRightFlag;
      }


      //! Returns t0 parameter of the specified sense wire
      /*!
      \param wireID Wire id.
      \return       t0.
      */
      float getT0(const WireID& wireID) const
      {
        //  std::cout << wireID.getICLayer() <<" "<< wireID.getIWire() << std::endl;
        return m_t0[wireID.getICLayer()][wireID.getIWire()];
      }

      //! Returns frontend board id. corresponding to the wire id.
      /*!
      \param wID   wire  id.
      \return      board id.
      */
      unsigned short getBoardID(const WireID& wID) const
      {
        std::map<WireID, unsigned short>::const_iterator it = m_wireToBoard.find(wID);
        unsigned short iret = (it != m_wireToBoard.end()) ? it->second : -999;
        return iret;
      }

      //! Returns frontend channel id. corresponding to the wire id.
      /*!
      \param wID   wire  id.
      \return      channel id. (0-47)
      */
      unsigned short getChannelID(const WireID& wID) const
      {
        std::map<WireID, unsigned short>::const_iterator it = m_wireToChannel.find(wID);
        unsigned short iret = (it != m_wireToChannel.end()) ? it->second : -999;
        return iret;
      }

      //! Returns wire id. corresponding to the board-and-cannel ids.
      /*!
      \param bd board   id. (1-300)
      \param ch channel id. (0-47)
      \return   wire    id.
      */
      const WireID getWireID(unsigned short bd, unsigned short ch) const
      {
        return WireID(m_boardAndChannelToWire[bd][ch]);
      }

      //! Returns time-walk
      /*!
      \param wID      wire id
      \param adcCount ADC count
      \return         time-walk (in ns)
      */
      double getTimeWalk(const WireID& wID, unsigned short adcCount) const
      {
        std::map<WireID, unsigned short>::const_iterator it = m_wireToBoard.find(wID);
        //  std::cout <<"SL,L,W, bd#= " << wID.getISuperLayer() <<" "<< wID.getILayer() <<" "<< wID.getIWire() <<" "<< it->second << std::endl;
        double tw = 0.;
        if (it != m_wireToBoard.end() && adcCount > 0) {
          if (m_twParamMode == 0) {
            tw = m_timeWalkCoef[it->second][0] / sqrt(adcCount);
          } else if (m_twParamMode == 1) {
            double p0 = m_timeWalkCoef[it->second][0];
            double p1 = m_timeWalkCoef[it->second][1];
            tw = p0 * exp(-p1 * adcCount);
          }
        }
        //  std::cout <<"bd#,coef,adc,tw= " << it->second <<" "<< m_timeWalkCoef[it->second] <<" "<< adcCount <<" "<< tw << std::endl;
        return tw;
      }

      //! Calculates and saves shifts in super-layers (to be used in searching hits in neighboring cells)
      /*!
      */
      void setShiftInSuperLayer();

      //! Returns shift in the super-layer
      /*!
      \param iSuperLayer The super-layer id.
      \param iLayer      The layer id. in the super-layer
      */
      signed short getShiftInSuperLayer(unsigned short iSuperLayer, unsigned short iLayer) const;

      //! Returns radius of sense wire in each layer.
      /*!
          \param layerId The layer id.
          \return The radius of sense wire in layer layerId.
      */
      double senseWireR(int layerId) const;

      //! Returns forward z position of sense wire in each layer.
      /*!
          \param layerId The layer id.
          \return The forward z position of sense wire in layer layerId.
      */
      double senseWireFZ(int layerId) const;

      //! Returns backward z position of sense wire in each layer.
      /*!
          \param layerId The layer id.
          \return The backward z position of sense wire in layer layerId.
      */
      double senseWireBZ(int layerId) const;

      //! Returns radius of field wire in each layer.
      /*!
          \param layerId The layer id of field wires.
          \return The radius of field wire in layer layerId.
      */
      double fieldWireR(int layerId) const;

      //! Returns forward z position of field wire in each layer.
      /*!
          \param layerId The layer id of field wires.
          \return The forward z position of field wire in layer layerId.
      */
      double fieldWireFZ(int layerId) const;

      //! Returns backward z position of field wire in each layer.
      /*!
          \param layerId The layer id of field wires.
          \return The backward z position of field wire in layer layerId.
      */
      double fieldWireBZ(int layerId) const;

      //! Returns number shift
      /*!
          \param layerId The layer id of sense wires.
          \return The shifted cell number in layer layerId.
      */
      int nShifts(int layerId) const;

      /**
       * Return wire offset in phi direction at endplate.
       * @param layerID layerID(0-55).
       * @return offset value.
       */
      double offset(int layerID) const;

      //! Set radius of sense wire in each layer.
      /*!
          \param layerId The layer id of sense wires.
          \param r The radius of sense wires in layer layerId.
      */
      void setSenseWireR(int layerId, double r);

      //! Set forward z position of sense wires.
      /*!
          \param layerId The layer id of sense wire.
          \param fz The forward position of sense wires in layer layerId.
      */
      void setSenseWireFZ(int layerId, double fz);

      //! set backward z position of sense wires.
      /*!
          \param layerId The layer id of sense wire.
          \param bz The backward position of sense wires in layer layerId.
      */
      void setSenseWireBZ(int layerId, double bz);

      /**
       * Return TDC offset value (default = 0 ch).
       */

      inline unsigned short getTdcOffset() const
      {
        return m_tdcOffset;
      }

      /**
       * Return TDC bin width (nsec).
       */

      inline double getTdcBinWidth() const
      {
        return m_tdcBinWidth;
      }

      /**
       * Return the nominal drift velocity of He-ethane gas
       * (default: 4.0x10^-3 cm/nsec).
       */

      inline double getNominalDriftV() const
      {
        return m_nominalDriftV;
      }

      /**
       * Return the nominal propagation speed of the sense wire
       * (default: 27.25 cm/nsec).
       */

      inline double getNominalPropSpeed() const
      {
        return m_nominalPropSpeed;
      }

      /**
       * Return the nominal spacial resolution.
       * (default: 130 um defined in CDC.xml).
       */

      inline double getNominalSpaceResol() const
      {
        return m_nominalSpaceResol;
      }

      /**
       * Return mode for material definition.
       * Return value = 0: define a mixture of gases and wires
       * in the entire tracking volume;
       * =1: define two different mixtures:
       * one for the inner volume (small cell),
       * and  the other for the outer volume (normal cell).
       *
       */

      inline int getMaterialDefinitionMode() const
      {
        return m_materialDefinitionMode;
      }

      /**
       * Return mode for sense wire z position.
       * Return value =0: define at the end-plate (gas side) with
       * old design;
       * =1: define at the bush inside the feedthrough with correction for
       * the diff. betw. final- and old-design.
       *
       */
      inline int getSenseWireZposMode() const
      {
        return m_senseWireZposMode;
      }

      /**
       * Return backward 'deltaZ'
       * @param[in] layerID (0-55)
       *
       */
      inline double getBwdDeltaZ(unsigned short layerID) const
      {
        return m_dzSBackwardLayer[layerID];
      }

      /**
       * Return forward 'deltaZ'
       * @param[in] layerID (0-55)
       *
       */
      inline double getFwdDeltaZ(unsigned short layerID) const
      {
        return m_dzSForwardLayer[layerID];
      }

      /**
       * Set the nominal spacial resolution in the unit of um.
       * @param[in] resol spacial resolution (um)
       *
       */

      inline void setNominalSpaceResol(double resol)
      {
        m_nominalSpaceResol = resol;
      }

      /**
       * Get the inversel of propagation speed in the sense wire.
       * @param[in]  layerID  layer ID (0-55)
       *
       */

      inline double getPropSpeedInv(const unsigned int layerID) const
      {
        return m_propSpeedInv[layerID];
      }

      /**
       * Inquire if the wire is totally-dead
       */
      inline bool isBadWire(const WireID& wid)
      {
        //        std::map<unsigned short, float>::iterator it = m_badWire.find(wid.getEWire());
        //        bool torf = (it != m_badWire.end()) ? true : false;
        //        return torf;
        bool torf = *m_badWireFromDB ? (*m_badWireFromDB)->isBadWire(wid) : false;
        return torf;

      }

      /**
       * Inquire if the wire is dead
       */
      inline bool isDeadWire(const WireID& wid, double& eff)
      {
        bool torf = *m_badWireFromDB ? (*m_badWireFromDB)->isDeadWire(wid, eff) : false;
        return torf;
      }

      /**
       * Inquire if the wire is hot
       */
      inline bool isHotWire(const WireID& wid)
      {
        bool torf = *m_badWireFromDB ? (*m_badWireFromDB)->isHotWire(wid) : false;
        return torf;
      }

      /**
       * Compute effects of the sense wire sag.
       * @param[in] set     Wire position set; =c_Base, c_Misaligned or c_Aligned
       * @param[in] layerID Layer ID
       * @param[in] cellID  Cell ID in the layer
       * @param[in] zw  Z-coord. (cm) at which the sense wire sag is computed
       * @param[out] ywb_sag Y-corrd. (cm) of intersection between a tangent and the backward endplate.
       * @param[out] ywf_sag Y-corrd. (cm) of intersection between a tangent and the forward endplate.
       * @attention The tangent is computed from the first derivative of a paraboric wire (due to gravity) defined at Z.
       * @todo It may be replaced with a bit more accurate formula.
       * @todo The electrostatic force effect should be included.
       */

      void getWireSagEffect(EWirePosition set, unsigned layerID, unsigned cellID, double zw, double& ywb_sag, double& ywf_sag) const;

      /**
       * Get the realistic drift velocity.
       * @param[in] dt Drift time (ns).
       * @param[in] layer Layer ID.
       * @param[in] lr Left/Right
       * @param[in] alpha incident angle (in rphi plane) w.r.t. the cell (rad).
       * @param[in] theta incident angle (polar angle) (rad).
       *
       */

      double getDriftV(double dt, unsigned short layer, unsigned short lr, double alpha = 0., double theta = 0.5 * M_PI) const;

      /**
       * Return the drift dength to the sense wire.
       * @param[in] dt Drift time (ns).
       * @param[in] layer Layer ID.
       * @param[in] lr Left/Right
       * @param[in] alpha incident angle (in rphi plane) w.r.t. the cell (rad).
       * @param[in] theta incident angle (polar angle) (rad).
       * @param calculateMinTime calculate min. drift time inside this function (=true) or feed as input (=false).
       * @param minTime input min. drift time when calculateMinTime=false.
       */

      double getDriftLength(double dt, unsigned short layer, unsigned short lr, double alpha = 0., double theta = 0.5 * M_PI,
                            bool calculateMinTime = true, double minTime = 0.) const;

      /**
       * Return the drift dength to the sense wire; tentative ver.
       * @param[in] dt Drift time (ns).
       * @param[in] layer Layer ID.
       * @param[in] lr Left/Right
       * @param[in] alpha incident angle (in rphi plane) w.r.t. the cell (rad).
       * @param[in] theta incident angle (polar angle) (rad).
       */
      double getDriftLength0(double dt, unsigned short layer, unsigned short lr, double alpha = 0., double theta = 0.5 * M_PI) const;

      /**
       * Return the min. drift time (ns).
       * @param[in] layer Layer ID.
       * @param[in] lr Left/Right
       * @param[in] alpha incident angle (in rphi plane) w.r.t. the cell (rad).
       * @param[in] theta incident angle (polar angle) (rad).
       */
      double getMinDriftTime(unsigned short layer, unsigned short lr, double alpha = 0., double theta = 0.5 * M_PI) const;

      /**
       * Return the drift time to the sense wire.
       * @param dist Drift length (cm).
       * @param layer Layer ID.
       * @param lr Left/Right
       * @param alpha incident angle (in rphi plane) w.r.t. the cell (rad).
       * @param theta incident angle (polar angle) (rad).
       */

      double getDriftTime(double dist, unsigned short layer, unsigned short lr, double alpha, double theta) const;

      /**
       * Return the basic resolution of drift length (cm). N.B. A fudge factor may be multiplied at the place where this is called; be careful.
       * @param dist Drift length (cm); negative dist is treated as |dist|.
       * @param layer Layer id.
       * @param lr Left/Right.
       * @param alpha incident angle (in rphi plane) w.r.t. the cell (rad).
       * @param theta incident angle (polar angle) (rad).
       */
      double getSigma(double dist, unsigned short layer, unsigned short lr, double alpha = 0., double theta = 0.5 * M_PI) const;

      /**
       * Return the fuge factor for space resol.
       * @param target target sigma: =0: for sigma in data reconstruction; =1: for sigma in MC recon.; =2: for sigma in digitization in MC
       */
      double getFudgeFactorForSigma(unsigned short target) const
      {
        return m_fudgeFactorForSigma[target];
      }

      /**
       * Returns old left/right.
       * @param posOnWire  Position on the wire  at the closest point.
       * @param posOnTrack Position on the track at the closest point.
       * @param momentum   Track 3-momentum.
       */
      unsigned short getOldLeftRight(const B2Vector3D& posOnWire, const B2Vector3D& posOnTrack, const B2Vector3D& momentum) const;

      /**
       * Returns new left/right_raw.
       * @param posOnWire  Position on the wire  at the closest point.
       * @param posOnTrack Position on the track at the closest point.
       * @param momentum   Track 3-momentum.
       */

      unsigned short getNewLeftRightRaw(const B2Vector3D& posOnWire, const B2Vector3D& posOnTrack, const B2Vector3D& momentum) const;

      /**
       * Returns track incident angle in rphi plane (alpha in rad.).
       * @param posOnWire Position on the wire  at the closest point.
       * @param momentum  Track momentum at the closest point.
       */

      double getAlpha(const B2Vector3D& posOnWire, const B2Vector3D& momentum) const;

      /**
       * Returns track incident angle (theta in rad.).
       * @param momentum Track momentum at the closest point.
       */
      double getTheta(const B2Vector3D& momentum) const;


      /**
       * Converts incoming-lr to outgoing-lr.
       * @param lr    Left/Right flag.
       * @param alpha Track incident angle in rphi-plane (rad).
       */
      unsigned short getOutgoingLR(const unsigned short lr, const double alpha) const;


      /**
       * Converts incoming-  to outgoing-alpha.
       * @param alpha in rad.
       */
      double getOutgoingAlpha(const double alpha) const;


      /**
       * Converts incoming-  to outgoing-theta.
       * @param alpha in rad.
       * @param theta in rad.
       */
      double getOutgoingTheta(const double alpha, const double theta) const;


      /**
       * Returns the two closest alpha points for the input track incident angle (alpha).
       */
      void getClosestAlphaPoints(const double alpha, double& wal, unsigned short points[2], unsigned short lrs[2]) const;

      /**
       * Returns the two closest alpha points for sigma for the input track incident angle (alpha). TODO: unify the two getClosestAlphaPoints().
       */
      void getClosestAlphaPoints4Sgm(const double alpha, double& wal, unsigned short points[2], unsigned short lrs[2]) const;

      /**
       * Returns the two closest theta points for the input track incident angle (theta).
       */
      void getClosestThetaPoints(const double alpha, const double theta, double& wth, unsigned short points[2]) const;

      /**
       * Returns the two closest theta points for sigma for the input track incident angle (theta).
       */
      void getClosestThetaPoints4Sgm(const double alpha, const double theta, double& wth, unsigned short points[2]) const;

      /**
       * Set the desizend wire parameters.
       * @param[in] layerID Layer ID
       * @param[in] cellID Cell ID
       */
      void setDesignWirParam(unsigned layerID, unsigned cellID);

      /**
       * Write the designed wire parameters to the alignment\.dat (default).
       * @param[in] layerID Layer ID
       * @param[in] cellID Cell ID
       */
      void outputDesignWirParam(unsigned layerID, unsigned cellID) const;

      /**
       * Set displacement of sense wire.
       */
      void setDisplacement();


      /**
      * Get the number of sense wires.
      */
      ushort getNumberOfSenseWires() const { return m_nSenseWires;}

      /**
      * Get the number of field wires.
      */
      ushort getNumberOfFieldWires() const { return m_nFieldWires;}

      /**
      * Get the number of sense layers.
      */
      ushort getNumberOfSenseLayers() const { return m_maxNSenseLayers;}

      /**
      * Get the number of field layers.
      */
      ushort getNumberOfFieldLayers() const { return m_maxNFieldLayers;}

      /**
      * Get the maximum number of super layers.
      */
      ushort getMaxNumberOfSuperLayers() const { return m_maxNSuperLayers;}

      /**
      * Get the offset of the first layer.
      */
      ushort getOffsetOfFirstLayer() const { return m_firstLayerOffset;}

      /**
      * Get the offset of the first super layer.
      */
      ushort getOffsetOfFirstSuperLayer() const { return m_firstSuperLayerOffset;}

      /**
      * Get the maximum number of cells in one layer.
      */
      ushort getMaxNumberOfCellsPerLayer() const { return m_maxNCellsPerLayer;}

    private:
      /** Singleton class */
      CDCGeometryPar(const CDCGeometry* = nullptr);
      /** Singleton class */
      CDCGeometryPar(const CDCGeometryPar&);
      /** Singleton class */
      CDCGeometryPar& operator=(const CDCGeometryPar&);

      bool m_debug;          /*!< Switch for debug printing. */
      bool m_linearInterpolationOfXT;  /*!< Switch for linear interpolation of xt */
      bool m_linearInterpolationOfSgm; /*!< Switch for linear interpolation of sigma */
      bool m_XTetc;          /*!< Switch for reading x-t etc. params.. */
      bool m_displacement;   /*!< Switch for displacement. */
      bool m_misalignment;   /*!< Switch for misalignment. */
      bool m_alignment;      /*!< Switch for alignment. */
      bool m_XTetc4Recon;    /*!< Switch for selecting xt etc. */

      bool m_wireSag;        /*!< Switch for sense wire sag */
      bool m_modLeftRightFlag; /*!< Switch for modified left/right flag */

      std::string m_version; /*!< The version of geometry parameters. */
      int m_materialDefinitionMode; /*!< Control switch for gas and wire material definition. */
      int m_senseWireZposMode; /*!< Mode for sense wire z position corr. */
      int m_xtFileFormat;      /*!< Format of xt input file */
      int m_xtParamMode;       /*!< Mode for xt parameterization */
      int m_sigmaFileFormat;   /*!< Format of sigma input file */
      int m_sigmaParamMode;    /*!< Mode for sigma parameterization */
      int m_twParamMode;       /*!< Mode for tw parameterization */
      int m_nSLayer;         /*!< The number of sense wire layer. */
      int m_nFLayer;         /*!< The number of field wire layer. */
      unsigned short m_nAlphaPoints;  /*!< No. of alpha points for xt. */
      unsigned short m_nThetaPoints;  /*!< No. of theta points for xt. */
      unsigned short m_nAlphaPoints4Sgm;  /*!< No. of alpha points for sigma. */
      unsigned short m_nThetaPoints4Sgm;  /*!< No. of theta points for sigma. */
      signed short m_shiftInSuperLayer[c_nSuperLayers][8]; /*!< shift in phi-direction wrt the 1st layer in each super layer*/

      double m_rWall[4];     /*!< The array to store radius of inner wall and outer wall. */
      double m_zWall[4][2];  /*!< The array to store z position of inner wall and outer wall. */

      double m_rSLayer[c_maxNSenseLayers];          /*!< The array to store radius of sense wire layers. */
      double m_zSForwardLayer[c_maxNSenseLayers];   /*!< The array to store forward z position of sense wire layers. */
      double m_dzSForwardLayer[c_maxNSenseLayers];   /*!< Corrections for forward z position of sense wire layers. */
      double m_zSBackwardLayer[c_maxNSenseLayers];  /*!< The array to store backward z position of sense wire layers. */
      double m_dzSBackwardLayer[c_maxNSenseLayers]; /*!< Corrections for backward z position of sense wire layers. */
      double m_rFLayer[c_maxNFieldLayers];          /*!< The array to store radius of field wire layers. */
      double m_zFForwardLayer[c_maxNFieldLayers];   /*!< The array to store forward z position of field wire layers. */
      double m_zFBackwardLayer[c_maxNFieldLayers];  /*!< The array to store backward z position of field wire layers. */
      double m_offSet[c_maxNSenseLayers];           /*!< The array to store z offset of sense wire layers. */
      double m_cellSize[c_maxNSenseLayers];         /*!< The array to store cell size in each sense wire layer. */
      int m_nShifts[c_maxNSenseLayers];             /*!< The array to store shifted cell number in each sense wire layer. */
      unsigned m_nWires[c_maxNSenseLayers];         /*!< The array to store the wire number in each sense wire layre. */

      double m_senseWireDiameter;                   /*!< The diameter of sense wires. */
      double m_senseWireTension;                    /*!< The tension of sense wires. */
      double m_senseWireDensity;                    /*!< The density of sense wires. */
      double m_fieldWireDiameter;                   /*!< The diameter of field wires. */

      double m_globalPhiRotation;  /*!< Global ratation in phi (rad.); only for sence wires now. */

      double m_momZ[7];      /*!< Z-cordinates of the cdc mother volume (7 segments). */
      double m_momRmin[7];   /*!< R_min of the cdc mother volume  (7 segments).       */

      //      double m_bwdDz[c_maxNSenseLayers];  /*!< Tentative backward z-corrections.*/
      //      double m_fwdDz[c_maxNSenseLayers];  /*!< Tentative forward  z-corrections.*/

      double m_thresholdEnergyDeposit; /*!< Energy thresh. for G4 step */
      double m_minTrackLength;         /*!< Minimum track length for G4 step */

      float m_FWirPos[c_maxNSenseLayers][c_maxNDriftCells][3]; /*!< Wire position incl. displacement at the forward endplate for each cell; to be implemented in a smarter way. */
      float m_BWirPos[c_maxNSenseLayers][c_maxNDriftCells][3]; /*!< Wire position incl. displacement at the backward endplate for each cell; ibid. */
      float m_WireSagCoef[c_maxNSenseLayers][c_maxNDriftCells]; /*!< Wire sag coefficient for each cell; ibid. */

      float m_FWirPosMisalign[c_maxNSenseLayers][c_maxNDriftCells][3]; /*!< Wire position incl. misalignment at the forward endplate for each cell; ibid. */
      float m_BWirPosMisalign[c_maxNSenseLayers][c_maxNDriftCells][3]; /*!< Wire position incl. misalignment at the backward endplate for each cell; ibid. */
      float m_WireSagCoefMisalign[c_maxNSenseLayers][c_maxNDriftCells]; /*!< Wire sag coefficient incl. misalignment for each cell; ibid. */

      float m_FWirPosAlign[c_maxNSenseLayers][c_maxNDriftCells][3]; /*!< Wire position incl. alignment at the forward endplate for each cell; ibid. */
      float m_BWirPosAlign[c_maxNSenseLayers][c_maxNDriftCells][3]; /*!< Wire position incl. alignment at the backward endplate for each cell; ibid. */
      float m_WireSagCoefAlign[c_maxNSenseLayers][c_maxNDriftCells]; /*!< Wire sag coefficient incl. alignment for each cell; ibid. */
      float m_eDepToADCParams[c_maxNSenseLayers][c_maxNDriftCells][7] = {}; /*!< edep-to-ADC conv. params. */

      float m_alphaPoints[c_maxNAlphaPoints]; /*!< alpha sampling points for xt (rad) */
      float m_thetaPoints[c_maxNThetaPoints]; /*!< theta sampling points for xt (rad) */
      float m_alphaPoints4Sgm[c_maxNAlphaPoints]; /*!< alpha sampling points for sigma (rad) */
      float m_thetaPoints4Sgm[c_maxNThetaPoints]; /*!< theta sampling points for sigma (rad) */

      float m_XT[c_maxNSenseLayers][2][c_maxNAlphaPoints][c_maxNThetaPoints][c_nXTParams];  /*!< XT-relation coefficients for each layer, Left/Right, entrance angle and polar angle.  */
      float m_Sigma[c_maxNSenseLayers][2][c_maxNAlphaPoints][c_maxNThetaPoints][c_nSigmaParams];      /*!< position resulution for each layer. */
      float m_propSpeedInv[c_maxNSenseLayers];  /*!< Inverse of propagation speed of the sense wire. */
      float m_t0[c_maxNSenseLayers][c_maxNDriftCells] = {};  /*!< t0 for each sense-wire (in nsec). */
      float m_timeWalkCoef[c_nBoards][2];  /*!< coefficients for time walk. */

      //      float m_meanT0;  /*!< mean t0 over all wires. */
      double m_meanT0;  /*!< mean t0 over all wires; should be double. */

      std::map<WireID, unsigned short> m_wireToBoard;  /*!< map relating wire-id and board-id. */
      std::map<WireID, unsigned short> m_wireToChannel; /*!< map relating wire-id and channel-id. */
      unsigned short m_boardAndChannelToWire[c_nBoards][48]; /*!< array relating board-channel-id and wire-id. */

      //      std::map<unsigned short, float> m_badWire;  /*!< list of bad-wires. */

      unsigned short m_tdcOffset;  /*!< Not used; to be removed later. */
      double m_clockFreq4TDC;      /*!< Clock frequency used for TDC (GHz). */
      double m_tdcBinWidth;        /*!< TDC bin width (nsec/bin). */
      double m_nominalDriftV;      /*!< Nominal drift velocity (4.0x10^-3 cm/nsec). */
      double m_nominalDriftVInv;   /*!< Inverse of the nominal drift velocity. */
      double m_nominalPropSpeed;   /*!< Nominal propagation speed of the sense wire (27.25 cm/nsec). */
      double m_nominalSpaceResol;  /*!< Nominal spacial resolution (0.0130 cm). */
      double m_maxSpaceResol;      /*!< max space resolution allowed (cm). */
      double m_fudgeFactorForSigma[3];     /*!< Fuge factor for space resol. */

      DBObjPtr<CDCTimeZeros>* m_t0FromDB; /*!< t0s retrieved from DB. */
      DBObjPtr<CDCBadWires>* m_badWireFromDB; /*!< bad-wires retrieved from DB. */
      DBObjPtr<CDCPropSpeeds>* m_propSpeedFromDB; /*!< prop.-speeds retrieved from DB. */
      DBObjPtr<CDCTimeWalks>* m_timeWalkFromDB; /*!< time-walk coeffs. retrieved from DB. */
      DBObjPtr<CDCXtRelations>* m_xtRelFromDB; /*!< xt params. retrieved from DB (new). */
      DBObjPtr<CDCSpaceResols>* m_sResolFromDB; /*!< sigma params. retrieved from DB. */
      DBObjPtr<CDCFudgeFactorsForSigma>* m_fFactorFromDB; /*!< fudge factors retrieved from DB. */
      DBArray<CDCChannelMap>* m_chMapFromDB; /*!< channel map retrieved from DB. */
      DBArray<CDCDisplacement>* m_displacementFromDB; /*!< displacement params. retrieved from DB. */
      DBObjPtr<CDCAlignment>* m_alignmentFromDB; /*!< alignment params. retrieved from DB. */
      DBObjPtr<CDCMisalignment>* m_misalignmentFromDB; /*!< misalignment params. retrieved from DB. */
      DBObjPtr<CDCEDepToADCConversions>* m_eDepToADCConversionsFromDB; /*!< Pointer to edep-to-ADC conv. params. from DB. */

      DBObjPtr<HardwareClockSettings> m_clockSettings; /*!< hardware clock settings */

      static CDCGeometryPar* m_B4CDCGeometryParDB; /*!< Pointer that saves the instance of this class. */

      ushort m_nSenseWires            = c_nSenseWires;      /*!< Maximum number of Sense Wires */
      ushort m_nFieldWires            = c_nFieldWires;      /*!< Maximum number of Field Wires */
      ushort m_maxNSenseLayers        = c_maxNSenseLayers;  /*!< Maximum number of Sense Wire Layers */
      ushort m_maxNFieldLayers        = c_maxNFieldLayers;  /*!< Maximum number of Field Wire Layers */
      ushort m_maxNSuperLayers        = c_nSuperLayers;     /*!< Maximum number of Super Layers */
      ushort m_firstLayerOffset       = 0;                  /*!< Offset of the first layer (for reduced CDC studies) */
      ushort m_firstSuperLayerOffset  = 0;                  /*!< Offset of the first super layer (for reduced CDC studies) */
      ushort m_maxNCellsPerLayer      = c_maxNDriftCells;   /*!< Maximum number wires within a layer */

    };

//-----------------------------------------------------------------------------
//  Inline functions
//-----------------------------------------------------------------------------
    inline std::string CDCGeometryPar::version() const
    {
      return m_version;
    }

    inline double CDCGeometryPar::momZ(int iBound) const
    {
      return m_momZ[iBound];
    }

    inline double CDCGeometryPar::momRmin(int iBound) const
    {
      return m_momRmin[iBound];
    }

    inline int CDCGeometryPar::nShifts(int layerID) const
    {
      return m_nShifts[layerID];
    }

    inline double CDCGeometryPar::offset(int layerID) const
    {
      return m_offSet[layerID];
    }

    inline unsigned CDCGeometryPar::nWiresInLayer(int layerID) const
    {
      return m_nWires[layerID];
    }

    inline void CDCGeometryPar::setSenseWireR(int layerId, double r)
    {
      m_rSLayer[layerId] = r;
    }

    inline void CDCGeometryPar::setSenseWireFZ(int layerId, double fz)
    {
      m_zSForwardLayer[layerId] = fz;
    }

    inline void CDCGeometryPar::setSenseWireBZ(int layerId, double bz)
    {
      m_zSBackwardLayer[layerId] = bz;
    }

    inline double CDCGeometryPar::senseWireR(int layerID) const
    {
      return m_rSLayer[layerID];
    }

    inline double CDCGeometryPar::senseWireFZ(int layerID) const
    {
      return m_zSForwardLayer[layerID];
    }

    inline double CDCGeometryPar::senseWireBZ(int layerID) const
    {
      return m_zSBackwardLayer[layerID];
    }

    inline double CDCGeometryPar::fieldWireR(int layerID) const
    {
      return m_rFLayer[layerID];
    }

    inline double CDCGeometryPar::fieldWireFZ(int layerID) const
    {
      return m_zFForwardLayer[layerID];
    }

    inline double CDCGeometryPar::fieldWireBZ(int layerID) const
    {
      return m_zFBackwardLayer[layerID];
    }

    inline double CDCGeometryPar::innerRadiusOuterWall() const
    {
      return m_rWall[2];
    }

    inline double CDCGeometryPar::outerRadiusOuterWall() const
    {
      return m_rWall[3];
    }

    inline double CDCGeometryPar::zOuterWall() const
    {
      return (m_zWall[2][1] - m_zWall[2][0]);
    }

    inline double CDCGeometryPar::innerRadiusInnerWall() const
    {
      return m_rWall[0];
    }

    inline double CDCGeometryPar::outerRadiusInnerWall() const
    {
      return m_rWall[1];
    }

    inline double CDCGeometryPar::zInnerWall() const
    {
      return (m_zWall[0][1] - m_zWall[0][0]);
    }

    inline double CDCGeometryPar::zOffsetOuterWall() const
    {
      return (m_zWall[2][0] + zOuterWall() / 2);
    }

    inline double CDCGeometryPar::zOffsetInnerWall() const
    {
      return (m_zWall[0][0] + zInnerWall() / 2);
    }

    inline double CDCGeometryPar::senseWireDiameter() const
    {
      return m_senseWireDiameter;
    }

    inline double CDCGeometryPar::fieldWireDiameter() const
    {
      return m_fieldWireDiameter;
    }

    inline unsigned CDCGeometryPar::nWireLayers() const
    {
      return c_maxNSenseLayers;
    }

    inline const double* CDCGeometryPar::zForwardWireLayer() const
    {
      return m_zSForwardLayer;
    }

    inline const double* CDCGeometryPar::zBackwardWireLayer() const
    {
      return m_zSBackwardLayer;
    }

    inline double CDCGeometryPar::zOffsetWireLayer(unsigned i) const
    {
      return (m_zSBackwardLayer[i] + (m_zSForwardLayer[i] - m_zSBackwardLayer[i]) / 2);
    }

    inline double CDCGeometryPar::getMeanT0() const
    {
      return m_meanT0;
    }

  } // end of namespace CDC
} // end of namespace Belle2
