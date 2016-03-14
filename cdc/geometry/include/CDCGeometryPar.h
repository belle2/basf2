/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Martin Heck                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCGEOMETRYPAR_H
#define CDCGEOMETRYPAR_H

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <cdc/dataobjects/WireID.h>

#include <vector>
#include <string>

#include "TVector3.h"

const unsigned MAX_N_SLAYERS =    56;
const unsigned MAX_N_SCELLS  =   384;
const unsigned MAX_N_FLAYERS =    55;
const unsigned nSenseWires   = 14336;
const unsigned nSuperLayers  =     9;

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

      //      //! Constructor
      //      CDCGeometryPar();

      //! Destructor
      virtual ~CDCGeometryPar();

      //! Static method to get a reference to the CDCGeometryPar instance.
      /*!
          \return A reference to an instance of this class.
      */
      static CDCGeometryPar& Instance();

      //! Clears
      void clear();

      //! Print some debug information
      void Print() const;

      //! Gets geometry parameters from gearbox.
      void read();


      /**
       * Read (mis)alignment params.
       * @param[in] GearDir Gear Dir.
       * @param[in] Wire position set =c_Misaliged: read misalignment file; =c_Aligned: read alignment file.
       */

      void readWirePositionParams(const GearDir, EWirePosition set);


      /**
       * Read XT-relation table.
       * @param[in] GearDir Gear Dir.
       * @param[in] mode 0: read simulation file, 1: read reconstruction file.
       */

      void readXT(const GearDir, int mode = 0);

      /**
       * Read spatial resolution of Drift length.
       * @param GearDir Gear Dir.
       * @param mode 0: read simulation file, 1: read reconstruction file.
       */

      void readSigma(const GearDir, int mode = 0);

      /**
       * Read the propagatio of speed on the sense wire.
       * @param GearDir Gear Dir.
       * @param mode 0: read simulation file, 1: read reconstruction file.
       */

      void readPropSpeed(const GearDir, int mode = 0);

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
      unsigned cellId(unsigned layerId, const TVector3& position) const;

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

      //! Returns the forward position of the input sense wire.
      /*!
          \param layerId The layer id. of the wire
          \param cellId  The wire id. of the wire
          \param set     Wire position set; =c_Base, c_Misaligned or c_Aligned
          \return The forward position of the wire.
      */
      const TVector3 wireForwardPosition(int layerId, int cellId, EWirePosition set = c_Base) const;

      /** The same function but in a different input format. */
      const TVector3 wireForwardPosition(const WireID& wireID, EWirePosition set = c_Base) const
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
      const TVector3 wireForwardPosition(int layerId, int cellId, double z, EWirePosition set = c_Base) const;
      /** The same function but in a different input format. */
      const TVector3 wireForwardPosition(const WireID& wireID, double z,
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
      const TVector3 wireBackwardPosition(int layerId, int cellId, EWirePosition set = c_Base) const;

      /** The same function but in a different input format. */
      const TVector3 wireBackwardPosition(const WireID& wireID, EWirePosition set = c_Base) const
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
      const TVector3 wireBackwardPosition(int layerId, int cellId, double z, EWirePosition set = c_Base) const;
      /** The same function but in a different input format. */
      const TVector3 wireBackwardPosition(const WireID& wireID, double z, EWirePosition set = c_Base) const
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
      double getWireSagCoef(EWirePosition set, int layerId, int cellId) const;

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
       * @parm layerID layerID(0-55).
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
       * Return TDC bin width (default: 1 nsec).
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
        return m_PropSpeedInv[layerID];
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
       * @param[in] dt Drift time.
       * @param[in] layer Layer ID.
       * @param[in] lr Left/Right
       * @param[in] alpha incident angle (in rphi plane) w.r.t. the cell.
       * @param[in] theta incident angle (polar angle).
       *
       */

      double getDriftV(double dt, unsigned short layer, unsigned short lr, double alpha = 0., double theta = 0.5 * M_PI) const;

      /**
       * Return the drift dength to the sense wire.
       * @param[in] dt Drift time.
       * @param[in] layer Layer ID.
       * @param[in] lr Left/Right
       * @param[in] alpha incident angle (in rphi plane) w.r.t. the cell.
       * @param[in] theta incident angle (polar angle).
       */

      double getDriftLength(double dt, unsigned short layer, unsigned short lr, double alpha = 0., double theta = 0.5 * M_PI) const;

      /**
       * Return the drift time to the sense wire.
       * @param dist Drift length.
       * @param layer Layer ID.
       * @param lr Left/Right
       * @param alpha incident angle (in rphi plane) w.r.t. the cell.
       * @param theta incident angle (polar angle).
       */

      double getDriftTime(double dist, unsigned short layer, unsigned short lr, double alpha, double theta) const;


      /**
       * Return the resolution of drift length.
       * @param dist Drift length..
       * @param layer Layer id
       */

      double getSigma(double dist, unsigned short layer) const;

      /**
       * Returns old left/right.
       * @param posOnWire  Position on the wire  at the closest point.
       * @param posOnTrack Position on the track at the closest point.
       * @param momentum   Track 3-momentum.
       */

      unsigned short getOldLeftRight(const TVector3& posOnWire, const TVector3& posOnTrack, const TVector3& momentum) const;

      /**
       * Returns new left/right_raw.
       * @param posOnWire  Position on the wire  at the closest point.
       * @param posOnTrack Position on the track at the closest point.
       * @param momentum   Track 3-momentum.
       */

      unsigned short getNewLeftRightRaw(const TVector3& posOnWire, const TVector3& posOnTrack, const TVector3& momentum) const;

      /**
       * Returns track incident angle in rphi plane (alpha in rad.).
       * @param posOnWire Position on the wire  at the closest point.
       * @param momentum  Track momentum at the closest point.
       */

      double getAlpha(const TVector3& posOnWire, const TVector3& momentum) const;

      /**
       * Returns track incident angle (theta in rad.).
       * @param momentum Track momentum at the closest point.
       */

      double getTheta(const TVector3& momentum) const;


      /**
       * Returns bin no. corresponding to track incident angle (alpha).
       * @param alpha in rad.
       */

      int getAlphaBin(const double alpha) const;


      /**
       * Returns bin no. corresponding to track incident angle (theta).
       * @param theta in rad.
       */

      int getThetaBin(const double theta) const;


      /**
       * Returns a closest distance between a track and a wire.
       * @param bwp[in] wire position at backward
       * @param fwp[in] wire position at forward
       * @param posIn[in] entrance position
       * @param posOut[in] exit position
       * @param hitPosition[out] track position corresp. to the closetst distance
       * @param wirePosition[out] wire position corresp. to the closetst distance
       */

      double ClosestApproach(const TVector3& bwp, const TVector3& fwp, const TVector3& posIn, const TVector3& posOut,
                             TVector3& hitPosition,
                             TVector3& wirePosition) const;

      /**
       * Check if neighboring cell in the same super-layer; essentially a copy from cdcLocalTracking/mclookup.
       * @param[in] wireId wire-id. in question (reference)
       * @param[in] otherWireId another wire-id. in question
       */

      unsigned short areNeighbors(const WireID& wireId, const WireID& otherWireId) const;

      /**
       * Check if neighboring cell in the same super-layer; essentially a copy from cdcLocalTracking/mclookup.
       * @param[in] iCLayer later-id (continuous) in question (reference)
       * @param[in] iSuperLayer super-later-id in question (reference)
       * @param[in] iLayer later-id in the super-layer in question (reference)
       * @param[in] iWire wire-id in the layer in question (reference)
       * @param[in] otherWireId another wire-id. in question
       */

      unsigned short areNeighbors(unsigned short iCLayer, unsigned short iSuperLayer, unsigned short iLayer, unsigned short iWire,
                                  const WireID& otherWireId) const;

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



    private:
      /** Singleton class */
      CDCGeometryPar();
      /** Singleton class */
      CDCGeometryPar(const CDCGeometryPar&);
      /** Singleton class */
      CDCGeometryPar& operator=(const CDCGeometryPar&);

      bool m_debug;          /*!< Switch for debug printing. */
      bool m_XTetc;          /*!< Switch for reading x-t etc. params.. */
      bool m_Misalignment;   /*!< Switch for misalignment. */
      bool m_Alignment;      /*!< Switch for alignment. */
      bool m_XTetc4Recon;    /*!< Switch for selecting xt etc. */
      std::string m_version; /*!< The version of geometry parameters. */
      int m_materialDefinitionMode; /*!< Control switch for gas and wire material definition. */
      signed short m_shiftInSuperLayer[nSuperLayers][8]; /*!< shift in phi-direction wrt the 1st layer in each super layer*/
      int m_nSLayer;         /*!< The number of sense wire layer. */
      int m_nFLayer;         /*!< The number of field wire layer. */
      double m_rWall[4];     /*!< The array to store radius of inner wall and outer wall. */
      double m_zWall[4][2];  /*!< The array to store z position of inner wall and outer wall. */

      double m_rSLayer[MAX_N_SLAYERS];          /*!< The array to store radius of sense wire layers. */
      double m_zSForwardLayer[MAX_N_SLAYERS];   /*!< The array to store forward z position of sense wire layers. */
      double m_zSBackwardLayer[MAX_N_SLAYERS];  /*!< The array to store backward z position of sense wire layers. */
      double m_rFLayer[MAX_N_FLAYERS];          /*!< The array to store radius of field wire layers. */
      double m_zFForwardLayer[MAX_N_FLAYERS];   /*!< The array to store forward z position of field wire layers. */
      double m_zFBackwardLayer[MAX_N_FLAYERS];  /*!< The array to store backward z position of field wire layers. */
      double m_offSet[MAX_N_SLAYERS];           /*!< The array to store z offset of sense wire layers. */
      double m_cellSize[MAX_N_SLAYERS];         /*!< The array to store cell size in each sense wire layer. */
      int m_nShifts[MAX_N_SLAYERS];             /*!< The array to store shifted cell number in each sense wire layer. */
      unsigned m_nWires[MAX_N_SLAYERS];         /*!< The array to store the wire number in each sense wire layre. */


      double m_senseWireDiameter;                   /*!< The diameter of sense wires. */
      double m_senseWireTension;                    /*!< The tension of sense wires. */
      double m_senseWireDensity;                    /*!< The density of sense wires. */
      double m_fieldWireDiameter;                   /*!< The diameter of field wires. */

      double m_motherInnerR;  /*!< The inner radius of cdc mother volume. */
      double m_motherOuterR;  /*!< The outer radius of cdc mother volume. */
      double m_motherLength;  /*!< The length of cdc mother volume. */

      double m_momZ[7];      /*!< Z-cordinates of the cdc mother volume (7 segments). */
      double m_momRmin[7];   /*!< R_min of the cdc mother volume  (7 segments).       */

      float m_FWirPos[MAX_N_SLAYERS][MAX_N_SCELLS][3]; /*!< Wire position at the forward endplate for each cell; to be implemented in a smarter way. */
      float m_BWirPos[MAX_N_SLAYERS][MAX_N_SCELLS][3]; /*!< Wire position at the backward endplate for each cell; ibid. */
      float m_WireSagCoef[MAX_N_SLAYERS][MAX_N_SCELLS]; /*!< Wire sag coefficient for each cell; ibid. */

      float m_FWirPosMisalign[MAX_N_SLAYERS][MAX_N_SCELLS][3]; /*!< Wire position incl. misalignment at the forward endplate for each cell; ibid. */
      float m_BWirPosMisalign[MAX_N_SLAYERS][MAX_N_SCELLS][3]; /*!< Wire position incl. misalignment at the backward endplate for each cell; ibid. */
      float m_WireSagCoefMisalign[MAX_N_SLAYERS][MAX_N_SCELLS]; /*!< Wire sag coefficient incl. misalignment for each cell; ibid. */

      float m_FWirPosAlign[MAX_N_SLAYERS][MAX_N_SCELLS][3]; /*!< Wire position incl. alignment at the forward endplate for each cell; ibid. */
      float m_BWirPosAlign[MAX_N_SLAYERS][MAX_N_SCELLS][3]; /*!< Wire position incl. alignment at the backward endplate for each cell; ibid. */
      float m_WireSagCoefAlign[MAX_N_SLAYERS][MAX_N_SCELLS]; /*!< Wire sag coefficient incl. alignment for each cell; ibid. */

      double m_XT[MAX_N_SLAYERS][2][19][7][9];  /*!< XT-relation coefficients for each layer, Left/Right, entrance angle and polar angle.  */
      double m_Sigma[MAX_N_SLAYERS][7];      /*!< position resulution for each layer. */
      double m_PropSpeedInv[MAX_N_SLAYERS];  /*!< Inverse of propagation speed of the sense wire. */

      unsigned short m_tdcOffset;  /*!< TDC off set value (default = 0).*/
      double m_tdcBinWidth;        /*!< TDC bin width (1 nsec/bin). */
      double m_nominalDriftV;      /*!< Nominal drift velocity (4.0x10^-3 cm/nsec). */
      double m_nominalDriftVInv;   /*!< Inverse of the nominal drift velocity. */
      double m_nominalPropSpeed;   /*!< Nominal propagation speed of the sense wire (27.25 cm/nsec). */
      double m_nominalSpaceResol;  /*!< Nominal spacial resolution (0.0130 cm). */
      double m_maxSpaceResol;      /*!< 10 times Nominal spacial resolution. */

      static CDCGeometryPar* m_B4CDCGeometryParDB; /*!< Pointer that saves the instance of this class. */


    };

//-----------------------------------------------------------------------------
//
//  Inline functions
//
//-----------------------------------------------------------------------------


    inline double CDCGeometryPar::motherInnerR() const
    {
      return m_motherInnerR;
    }

    inline double CDCGeometryPar::motherOuterR() const
    {
      return m_motherOuterR;
    }

    inline double CDCGeometryPar::motherLength() const
    {
      return m_motherLength;
    }

    inline double CDCGeometryPar::momZ(int iBound) const
    {
      return m_momZ[iBound];
    }

    inline double CDCGeometryPar::momRmin(int iBound) const
    {
      return m_momRmin[iBound];
    }

    inline std::string CDCGeometryPar::version() const
    {
      return m_version;
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
      return MAX_N_SLAYERS;
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
  } // end of namespace CDC
} // end of namespace Belle2

#endif
