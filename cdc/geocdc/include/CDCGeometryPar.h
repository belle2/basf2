/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCGEOMETRYPAR_H
#define CDCGEOMETRYPAR_H

#include <vector>
#include <string>

#include "TVector3.h"

#define MAX_N_SLAYERS 56
#define MAX_N_SCELLS 384
#define MAX_N_FLAYERS 55
#define MAX_N_KINK 30

namespace Belle2 {

//! The Class for CDC Geometry Parameters
  /*! This class provides CDC gemetry paramters for simulation, reconstruction and so on.
      These parameters are gotten from gearbox.
  */
  class CDCGeometryPar {

  public:

    //! Constructor
    CDCGeometryPar();

    //! Destructor
    virtual ~CDCGeometryPar();

    //! Static method to get a reference to the CDCGeometryPar instance.
    /*!
        \return A reference to an instance of this class.
    */
    static CDCGeometryPar* Instance(void);

    //! Clears
    void clear(void);

    //! Print some debug information
    void Print(void) const;

    //! Gets geometry parameters from gearbox.
    void read();

    //! Generate an xml file used in gearbox
    /*!
        \param of The file name.
    */
    void generateXML(const std::string & of);

    //! The method to get cell id based on given layer id and the position.
    /*!
        \param layerId The given layer id.
        \param position The given position to calculate cell id.
        \return Cell id.
    */
    unsigned cellId(unsigned layerId, const TVector3 & position) const;

    //! Returns the inner radius of the outer wall.
    /*!
        \return The inner radius of the outer wall.
    */
    double innerRadiusOuterWall(void) const;

    //! Returns the outer radius of the outer wall.
    /*!
        \return The outer radius of the outer wall.
    */
    double outerRadiusOuterWall(void) const;

    //! Returns the length of the outer wall in Z.
    /*!
        \return The length of the outer wall.
    */
    double zOuterWall(void) const;

    //! Returns the offset of the outer wall in z direction.
    /*!
        \return The z offset of the outer wall.
    */
    double zOffsetOuterWall(void) const;

    //! Returns the inner radius of the inner wall.
    /*!
        \return The inner radius of the inner wall.
    */
    double innerRadiusInnerWall(void) const;

    //! Returns the outer radius of the inner wall.
    /*!
        \return The outer radius of the inner wall.
    */
    double outerRadiusInnerWall(void) const;

    //! Returns the length of the inner wall in Z.
    /*!
        \return The length of the inner wall.
    */
    double zInnerWall(void) const;

    //! Returns the offset of the outer wall in z direction.
    /*!
        \return The z offset of the outer wall.
    */
    double zOffsetInnerWall(void) const;

    //! Returns diameter of the sense wire.
    /*!
        \return Diameter of the sense wire.
    */
    double senseWireDiameter(void) const;

    //! Returns diameter of the field wire.
    /*!
        \return Diameter of the field wire.
    */
    double fieldWireDiameter(void) const;

    //! Returns a number of wire layers.
    /*!
        \return The number of wire layers.
    */
    unsigned nWireLayers(void) const;

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
    const double * innerRadiusWireLayer(void) const;

    //! Returns an array of outer radius of wire layers.
    /*!
        \return A array of outer radius of wire layers.
    */
    const double * outerRadiusWireLayer(void) const;

    //! Returns an array of forward z of wire layers.
    /*!
        \return An array of forward z.
    */
    const double * zForwardWireLayer(void) const;

    //! Returns an array of backward z of wire layers.
    /*!
        \return An array of backward z.
    */
    const double * zBackwardWireLayer(void) const;

    //! Returns the offset of z of the wire layer i.
    /*!
        \param i The layer id.
        \return The z offset of wire layer i.
    */
    double zOffsetWireLayer(unsigned i) const;

    //! Returns forward position of sense wires in each layer.
    /*!
        \param layerId The layer id.
        \param cellId The wire id.
        \return The forward position of wire cellId in layer layerId.
    */
    TVector3 wireForwardPosition(int layerId, int cellId);

    //! Returns an array of backward position of sense wires in each layer.
    /*!
        \param layerId The layer id.
        \param cellId The wire id.
        \return The backward position of wire cellId in layer layerId.
    */
    TVector3 wireBackwardPosition(int layerId, int cellId);

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

  private:

    std::string _version; /*!< The version of geometry parameters. */
    int _nSLayer;         /*!< The number of sense wire layer. */
    int _nFLayer;         /*!< The number of field wire layer. */
    double _rWall[4];     /*!< The array to store radius of inner wall and outer wall. */
    double _zWall[4][2];  /*!< The array to store z position of inner wall and outer wall. */

    double _rSLayer[MAX_N_SLAYERS];          /*!< The array to store radius of sense wire layers. */
    double _zSForwardLayer[MAX_N_SLAYERS];   /*!< The array to store forward z position of sense wire layers. */
    double _zSBackwardLayer[MAX_N_SLAYERS];  /*!< The array to store backward z position of sense wire layers. */
    double _rFLayer[MAX_N_FLAYERS];          /*!< The array to store radius of field wire layers. */
    double _zFForwardLayer[MAX_N_FLAYERS];   /*!< The array to store forward z position of field wire layers. */
    double _zFBackwardLayer[MAX_N_FLAYERS];  /*!< The array to store backward z position of field wire layers. */
    double _offSet[MAX_N_SLAYERS];           /*!< The array to store z offset of sense wire layers. */
    double _cellSize[MAX_N_SLAYERS];         /*!< The array to store cell size in each sense wire layer. */
    int _nShifts[MAX_N_SLAYERS];             /*!< The array to store shifted cell number in each sense wire layer. */
    unsigned _nWires[MAX_N_SLAYERS];         /*!< The array to store the wire number in each sense wire layre. */

    TVector3 _wireForwardPosition[MAX_N_SLAYERS][MAX_N_SCELLS];  /*!< The forward position of each sense wire in each layer. */
    TVector3 _wireBackwardPosition[MAX_N_SLAYERS][MAX_N_SCELLS]; /*!< The backward position of each sense wire in each layer. */

    double _senseWireDiameter;                   /*!< The diameter of sense wires. */
    double _fieldWireDiameter;                   /*!< The diameter of field wires. */

    static CDCGeometryPar* p_B4CDCGeometryParDB; /*!< Pointer that saves the instance of this class. */
  };

//-----------------------------------------------------------------------------

  inline int CDCGeometryPar::nShifts(int layerID) const
  {
    return _nShifts[layerID];
  }

  inline unsigned CDCGeometryPar::nWiresInLayer(int layerID) const
  {
    return _nWires[layerID];
  }

  inline void CDCGeometryPar::setSenseWireR(int layerId, double r)
  {
    _rSLayer[layerId] = r;
  }

  inline void CDCGeometryPar::setSenseWireFZ(int layerId, double fz)
  {
    _zSForwardLayer[layerId] = fz;
  }

  inline void CDCGeometryPar::setSenseWireBZ(int layerId, double bz)
  {
    _zSBackwardLayer[layerId] = bz;
  }

  inline double CDCGeometryPar::senseWireR(int layerID) const
  {
    return _rSLayer[layerID];
  }

  inline double CDCGeometryPar::senseWireFZ(int layerID) const
  {
    return _zSForwardLayer[layerID];
  }

  inline double CDCGeometryPar::senseWireBZ(int layerID) const
  {
    return _zSBackwardLayer[layerID];
  }

  inline double CDCGeometryPar::fieldWireR(int layerID) const
  {
    return _rFLayer[layerID];
  }

  inline double CDCGeometryPar::fieldWireFZ(int layerID) const
  {
    return _zFForwardLayer[layerID];
  }

  inline double CDCGeometryPar::fieldWireBZ(int layerID) const
  {
    return _zFBackwardLayer[layerID];
  }

  inline
  double
  CDCGeometryPar::innerRadiusOuterWall(void) const
  {
    return _rWall[2];
  }

  inline
  double
  CDCGeometryPar::outerRadiusOuterWall(void) const
  {
    return _rWall[3];
  }

  inline
  double
  CDCGeometryPar::zOuterWall(void) const
  {
    return (_zWall[2][1] - _zWall[2][0]);
  }

  inline
  double
  CDCGeometryPar::innerRadiusInnerWall(void) const
  {
    return _rWall[0];
  }

  inline
  double
  CDCGeometryPar::outerRadiusInnerWall(void) const
  {
    return _rWall[1];
  }

  inline
  double
  CDCGeometryPar::zInnerWall(void) const
  {
    return (_zWall[0][1] - _zWall[0][0]);
  }

  inline
  double
  CDCGeometryPar::zOffsetOuterWall(void) const
  {
    return (_zWall[2][0] + zOuterWall() / 2);
  }

  inline
  double
  CDCGeometryPar::zOffsetInnerWall(void) const
  {
    return (_zWall[0][0] + zInnerWall() / 2);
  }

  inline
  double
  CDCGeometryPar::senseWireDiameter(void) const
  {
    return _senseWireDiameter;
  }

  inline
  double
  CDCGeometryPar::fieldWireDiameter(void) const
  {
    return _fieldWireDiameter;
  }

  inline
  unsigned
  CDCGeometryPar::nWireLayers(void) const
  {
    return MAX_N_SLAYERS;
  }

  inline
  const double *
  CDCGeometryPar::zForwardWireLayer(void) const
  {
    return _zSForwardLayer;
  }

  inline
  const double *
  CDCGeometryPar::zBackwardWireLayer(void) const
  {
    return _zSBackwardLayer;
  }

  inline
  double
  CDCGeometryPar::zOffsetWireLayer(unsigned i) const
  {
    return (_zSBackwardLayer[i] + (_zSForwardLayer[i] - _zSBackwardLayer[i]) / 2);
  }

} // end of namespace Belle2

#endif
