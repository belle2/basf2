/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Alexandre BEAULIEU                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <vector>

#include <TObject.h>
#include <string>

namespace Belle2 {

  /**
  * The Class for STR geometry
  */

  class STRGeometryPar: public TObject {

  public:

    //! Default constructor
    STRGeometryPar();

    //! Destructor
    ~STRGeometryPar();

    //
    // Setters and Getters
    //
    ///////////////////////

    //! Get STR geometry version
    int getVersion() const { return m_Version; }

    //! Set STR geometry version
    void setVersion(int version) { m_Version = version; }

    //
    // Shield parameters
    //


    //! Get the number of layers in the shield
    int getNLayers(int shield) const { return m_NLayers[shield] ; }

    //! Set the number of layers in the shield, resize std::vectors
    void setNLayers(int shield, int nLayers)
    {
      m_NLayers[shield] = nLayers ;

      // Resize std::vectors according to the number of layers
      m_LayerMaterial[shield].resize(nLayers);
      m_LayerNPlanes[shield].resize(nLayers);
      m_LayerPlaneZ[shield].resize(nLayers);
      m_LayerPlaneInnerRadius[shield].resize(nLayers);
      m_LayerPlaneOuterRadius[shield].resize(nLayers);

    }

    //! Get the material of a layer (as a std::string)
    std::string getLayerMaterial(int shield, int layer) const { return m_LayerMaterial[shield].at(layer);}

    //! Set the material of a layer (as a std::string)
    void setLayerMaterial(int shield, int layer, const std::string& material) { m_LayerMaterial[shield].at(layer) = material; }

    //! Get Number of polycone planes in each layer
    int getLayerNPlanes(int shield, int layer) const
    {
      return m_LayerNPlanes[shield].at(layer);
    }

    //! Set Number of polycone planes in each layer
    void setLayerNPlanes(int shield, int layer, int nplanes)
    {
      m_LayerNPlanes[shield].at(layer) = nplanes;

      // Resize std::vectors according to the number of planes in each layer
      m_LayerPlaneZ[shield].at(layer).resize(nplanes);
      m_LayerPlaneInnerRadius[shield].at(layer).resize(nplanes);
      m_LayerPlaneOuterRadius[shield].at(layer).resize(nplanes);
    }

    //! Get Z-position of one plane (w.r.t. IP)
    double getLayerPlaneZ(int shield, int layer, int plane) const
    {
      return m_LayerPlaneZ[shield].at(layer).at(plane);
    }

    //! Get Z-positions of all planes (w.r.t. IP)
    const double* getLayerPlaneZ(int shield, int layer) const
    {
      return &m_LayerPlaneZ[shield].at(layer).at(0);
    }


    //! Set Z-position of each plane (w.r.t. IP)
    void setLayerPlaneZ(int shield, int layer, int plane, double z)
    {
      m_LayerPlaneZ[shield].at(layer).at(plane) = z;
    }


    //! Get Inner radius of one plane
    double getLayerPlaneInnerRadius(int shield, int layer, int plane) const
    {
      return m_LayerPlaneInnerRadius[shield].at(layer).at(plane);
    }

    //! Get Inner radius of all planes
    const  double* getLayerPlaneInnerRadius(int shield, int layer) const
    {
      return &m_LayerPlaneInnerRadius[shield].at(layer).at(0);
    }

    //! Set Inner radii of the planes
    void setLayerPlaneInnerRadius(int shield, int layer, int plane, double r)
    {
      m_LayerPlaneInnerRadius[shield].at(layer).at(plane) = r;
    }

    //! Get Outer radii of one planes
    double getLayerPlaneOuterRadius(int shield, int layer, int plane) const
    {
      return m_LayerPlaneOuterRadius[shield].at(layer).at(plane);
    }

    //! Get Outer radii of all planes
    const double* getLayerPlaneOuterRadius(int shield, int layer) const
    {
      return &m_LayerPlaneOuterRadius[shield].at(layer).at(0);
    }

    //! Set Outer radii of the planes
    void setLayerPlaneOuterRadius(int shield, int layer, int plane, double r)
    {
      m_LayerPlaneOuterRadius[shield].at(layer).at(plane) = r;
    }

    //
    // POLE Pieces
    //
    //! Get the material of a layer (as a std::string)
    std::string getPoleMaterial(int pole) const
    {
      return m_PoleMaterial[pole];
    }

    //! Set the material of a layer (as a std::string)
    void setPoleMaterial(int pole, const std::string& material)
    {
      m_PoleMaterial[pole] = material;
    }

    //! Get Number of polycone planes in each layer
    int getPoleNPlanes(int pole) const
    {
      return m_PoleNPlanes[pole];
    }

    //! Set Number of polycone planes in each layer
    void setPoleNPlanes(int pole, int nplanes)
    {
      m_PoleNPlanes[pole] = nplanes;

      // Resize std::vectors according to the number of planes in each layer
      m_PolePlaneZ[pole].resize(nplanes);
      m_PolePlaneInnerRadius[pole].resize(nplanes);
      m_PolePlaneOuterRadius[pole].resize(nplanes);
    }

    //! Get Z-position of one plane (w.r.t. IP)
    double getPolePlaneZ(int pole, int plane) const
    {
      return m_PolePlaneZ[pole].at(plane);
    }

    //! Get Z-positions of all planes (w.r.t. IP)
    const double* getPolePlaneZ(int pole) const
    {
      return &m_PolePlaneZ[pole].at(0);
    }


    //! Set Z-position of each plane (w.r.t. IP)
    void setPolePlaneZ(int pole, int plane, double z)
    {
      m_PolePlaneZ[pole].at(plane) = z;
    }


    //! Get Inner radius of one plane
    double getPolePlaneInnerRadius(int pole, int plane) const
    {
      return m_PolePlaneInnerRadius[pole].at(plane);
    }

    //! Get Inner radius of all planes
    const  double* getPolePlaneInnerRadius(int pole) const
    {
      return &m_PolePlaneInnerRadius[pole].at(0);
    }

    //! Set Inner radii of the planes
    void setPolePlaneInnerRadius(int pole, int plane, double r)
    {
      m_PolePlaneInnerRadius[pole].at(plane) = r;
    }

    //! Get Outer radii of one planes
    double getPolePlaneOuterRadius(int pole, int plane) const
    {
      return m_PolePlaneOuterRadius[pole].at(plane);
    }

    //! Get Outer radii of all planes
    const double* getPolePlaneOuterRadius(int pole) const
    {
      return &m_PolePlaneOuterRadius[pole].at(0);
    }

    //! Set Outer radii of the planes
    void setPolePlaneOuterRadius(int pole, int plane, double r)
    {
      m_PolePlaneOuterRadius[pole].at(plane) = r;
    }



    //
    // Constants that will _always_ be true
    // These ate not design parameters but fundamental
    // characteristics of the system. e.g. we will always
    // have only one forward and one backward shield.
    //
    //! Number of ECL shields in Belle2
    static const int NECLSHIELDS = 2;
    //! Index corresponding to the forward shield
    static const int FWD_ECLSHIELD = 0;
    //! Index corresponding to the backward shield
    static const int BWD_ECLSHIELD = 1;

    //! Number of pole pieces in Belle2
    static const int NPOLEPIECES = 2;
    //! Index corresponding to the forward pole piece
    static const int FWD_POLEPIECE = 0;
    //! Index corresponding to the backward pole piece
    static const int BWD_POLEPIECE = 1;



  private:


    //! static pointer to the singleton instance of this class
    static STRGeometryPar* m_Instance;


    //
    //         Data members:
    // these are the geometry parameters
    //
    ////////////////////////////////////

    //! Geometry version
    int m_Version;

    //
    // ECL Shields
    //

    //! Number of layers in the shield
    int m_NLayers[NECLSHIELDS];

    //! Material of each of the layers
    std::vector<std::string> m_LayerMaterial[NECLSHIELDS];

    //! Number of polycone planes in each layer
    std::vector<int> m_LayerNPlanes[NECLSHIELDS];

    //! Z-position of each plane (w.r.t. IP)
    std::vector< std::vector<double> > m_LayerPlaneZ[NECLSHIELDS];

    //! Inner radii of the planes
    std::vector< std::vector<double> > m_LayerPlaneInnerRadius[NECLSHIELDS];

    //! Outer radii of the planes
    std::vector< std::vector<double> > m_LayerPlaneOuterRadius[NECLSHIELDS];


    //
    // POLE Pieces
    //

    //! Material of each of the pole pieces
    std::string m_PoleMaterial[NPOLEPIECES];

    //! Number of polycone planes in each pole piece
    int m_PoleNPlanes[NPOLEPIECES];

    //! Z-position of each plane (w.r.t. IP)
    std::vector<double> m_PolePlaneZ[NPOLEPIECES];

    //! Inner radii of the planes
    std::vector<double> m_PolePlaneInnerRadius[NPOLEPIECES];

    //! Outer radii of the planes
    std::vector<double> m_PolePlaneOuterRadius[NPOLEPIECES];



    //
    // For ROOT objects
    //

    ClassDef(STRGeometryPar, 1);  /**< ClassDef, must be the last term before the closing {}*/

  };
} // end of namespace Belle2


