/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Makoto Uchida                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <vector>
#include <map>
#include <string>
#include <cmath>

#include <TObject.h>

namespace Belle2 {

  class GearDir;

  /**
   * The Class for CDC geometry.
   */

  class CDCGeometry: public TObject {

  public:


    /**
     * Rib structure geometry parameters.
     */
    class Rib : public TObject {

    public:

      /**
       * Constructor.
       */
      Rib() {}

      /**
       * Constructor.
       */
      Rib(int id, double length, double width,
          double thick, double x, double y, double z,
          int ndiv):
        m_id(id), m_length(length), m_width(width), m_thick(thick),
        m_x(x), m_y(y), m_z(z), m_ndiv(ndiv)
      {}


      /**
       * Destructor.
       */
      ~Rib() {}

      /**
       * Get the rib Id.
       */
      int getId() const {return m_id;}


      /**
       * Get the length of the rib.
       */
      double getLength() const { return m_length;}

      /**
       * Get the width of the rib.
       */
      double getWidth() const { return m_width;}

      /**
       * Get the thickness of the rib.
       */
      double getThick() const { return m_thick;}

      /**
       * Get the x-position of the rib.
       */
      double getX() const { return m_x;}
      /**
       * Get the y-position of the rib.
       */
      double getY() const { return m_y;}
      /**
       * Get the z-position of the rib.
       */
      double getZ() const { return m_z;}

      /**
       * Get number of division of the rib.
       */
      int getNDiv() const { return m_ndiv;}


    private:
      int m_id; /**< rib id. */
      double m_length; /**< length of the rib. */
      double m_width; /**< width of the rib. */
      double m_thick; /**< thickness of the rib. */
      double m_x; /**< the y-position of the rib. */
      double m_y;/**< the y-position of the rib. */
      double m_z;/**< the z-position of the rib. */
      int m_ndiv; /**< Number of division. */


      /** Makes objects storable. */
      ClassDef(Rib, 1);
    };

    /**
     * Cover structure geometry parameters.
     */
    class Cover : public TObject {

    public:

      /**
       * Constructor.
       */
      Cover() {}

      /**
       * Constructor.
       */
      Cover(int id, double rmin1, double rmin2,
            double rmax1, double rmax2,
            double thick, double z):
        m_id(id), m_rmin1(rmin1), m_rmin2(rmin2), m_rmax1(rmax1),
        m_rmax2(rmax2), m_thick(thick), m_z(z)
      {}


      /**
       * Destructor.
       */
      ~Cover() {}

      /**
       * Get the cover Id.
       */
      int getId() const {return m_id;}
      /**
       * Get the minimum radius 1 of the cover.
       */
      double getRmin1() const { return m_rmin1;}

      /**
       * Get the minimum radius 2 of the cover.
       */
      double getRmin2() const { return m_rmin2;}

      /**
       * Get the maximum radius 1 of the cover.
       */
      double getRmax1() const { return m_rmax1;}

      /**
       * Get the maximum radius 2 of the cover.
       */
      double getRmax2() const { return m_rmax2;}

      /**
       * Get the thickness of the cover.
       */
      double getThick() const { return m_thick;}

      /**
       * Get the z-position of the cover.
       */
      double getZ() const { return m_z;}



    private:
      int m_id; /**< cover id. */
      double m_rmin1; /**< the mimnum radius 1 of the cover. */
      double m_rmin2; /**< the mimnum radius 2 of the cover. */
      double m_rmax1; /**< the maximum radius 1 of the cover. */
      double m_rmax2; /**< the maximum radius 2 of the cover. */
      double m_thick;/**< the z-position of the cover. */
      double m_z;/**< the thickness of the cover. */


      /** Makes objects storable. */
      ClassDef(Cover, 1);
    };

    /**
     * Neutron shield geometry parameters.
     */
    class NeutronShield : public TObject {

    public:

      /**
       * Constructor.
       */
      NeutronShield() {}

      /**
       * Constructor.
       */
      NeutronShield(int id, double rmin1, double rmin2,
                    double rmax1, double rmax2,
                    double thick, double z):
        m_id(id), m_rmin1(rmin1), m_rmin2(rmin2), m_rmax1(rmax1),
        m_rmax2(rmax2), m_thick(thick), m_z(z)
      {}


      /**
       * Destructor.
       */
      ~NeutronShield() {}

      /**
       * Get the shield Id.
       */
      int getId() const {return m_id;}
      /**
       * Get the minimum radius 1 of the shield.
       */
      double getRmin1() const { return m_rmin1;}

      /**
       * Get the minimum radius 2 of the shield.
       */
      double getRmin2() const { return m_rmin2;}

      /**
       * Get the maximum radius 1 of the shield.
       */
      double getRmax1() const { return m_rmax1;}

      /**
       * Get the maximum radius 2 of the shield.
       */
      double getRmax2() const { return m_rmax2;}

      /**
       * Get the thickness of the shield.
       */
      double getThick() const { return m_thick;}

      /**
       * Get the z-position of the shield.
       */
      double getZ() const { return m_z;}



    private:
      int m_id; /**< shield id. */
      double m_rmin1; /**< the mimnum radius 1 of the shield. */
      double m_rmin2; /**< the mimnum radius 2 of the shield. */
      double m_rmax1; /**< the maximum radius 1 of the shield. */
      double m_rmax2; /**< the maximum radius 2 of the shield. */
      double m_thick;/**< the z-position of the shield. */
      double m_z;/**< the thickness of the shield. */


      /** Makes objects storable. */
      ClassDef(NeutronShield, 1);
    };

    /**
     * Frontend layer geometry parameters.
     */
    class Frontend : public TObject {

    public:

      /**
       * Constructor.
       */
      Frontend() {}

      /**
       * Constructor.
       */
      Frontend(int id, double rmin, double rmax, double zfwd, double zbwd):
        m_id(id), m_rmin(rmin), m_rmax(rmax), m_zfwd(zfwd), m_zbwd(zbwd)
      {}


      /**
       * Destructor.
       */
      ~Frontend() {}
      /**
       * Get the frontend layer Id.
       */
      int getId() const {return m_id;}
      /**
       * Get the minimum radius of the frontend layer.
       */
      double getRmin() const { return m_rmin;}
      /**
       * Get the maximum radius of the frontend layer.
       */
      double getRmax() const { return m_rmax;}

      /**
       * Get the forward z-position of the frontend layer.
       */
      double getZfwd() const { return m_zfwd;}

      /**
       * Get the backward z-position of the frontend layer.
       */
      double getZbwd() const { return m_zbwd;}


    private:
      int m_id; /**< Frontend layer id. */
      double m_rmin; /**< the mimnum radius of the frontend layer. */
      double m_rmax; /**< the maximum radius of the frontend layer. */
      double m_zfwd;/**< the forawrd z-position of the frontend layer. */
      double m_zbwd;/**< the forawrd z-position of the frontend layer. */

      /** Makes objects storable. */
      ClassDef(Frontend, 1);
    };


    /**
     * Mother volume geometry parameters.
     */
    class MotherVolume : public TObject {

    public:

      /**
       * Constructor.
       */
      MotherVolume() {}

      /**
       * Destructor.
       */
      ~MotherVolume() {}

      /**
       * Append a new node.
       */
      void appendNode(double rmin, double rmax , double z);

      /**
       * Get the number of the mother volume nodes.
       */
      int getNNodes() const { return m_rmin.size();}
      /**
       * Get the list of the Rmin corrdinates.
       */
      std::vector<double> getRmin() const { return m_rmin;}
      /**
       * Get the list of the Rmax corrdinates.
       */
      std::vector<double> getRmax() const { return m_rmax;}
      /**
       * Get the list of the z corrdinates.
       */
      std::vector<double> getZ() const { return m_z;}


    private:
      std::vector<double> m_rmin; /**< Rmin list of the mother volume. */
      std::vector<double> m_rmax; /**< Rmax list of the mother volume. */
      std::vector<double> m_z; /**< Z-cordinates list of the mother volume. */

      /** Makes objects storable. */
      ClassDef(MotherVolume, 1);
    };

    /**
     * Sense layer geometry parameters.
     */
    class SenseLayer : public TObject {

    public:

      /**
       * Default constructor.
       */
      SenseLayer()
      {}

      /**
       * Constructor.
       */
      SenseLayer(int id,
                 double r, double zfwd, double zbwd,
                 double dZfwd, double dZbwd,
                 double offset, int nWires, double nShifts) :
        m_id(id), m_r(r),
        m_zfwd(zfwd), m_zbwd(zbwd),
        m_dZfwd(dZfwd), m_dZbwd(dZbwd),
        m_offset(offset),
        m_nWires(nWires),
        m_nShifts(nShifts)
      {
      }

      /**
       * Destructor.
       */
      ~SenseLayer() {}

      /**
       * Get ID.
       */
      int  getId() const { return m_id;}

      /**
       * Get Radius.
       */
      double getR() const {return m_r;}

      /**
       * Get fwd z-position.
       */
      double getZfwd() const { return m_zfwd;}

      /**
       * Get bwd z-position.
       */
      double getZbwd() const { return m_zbwd;}

      /**
       * Set fwd dz.
       */
      void setDZfwd(double dz) {m_dZfwd = dz;}

      /**
       * Get fwd dz.
       */
      double getDZfwd() const {return m_dZfwd;}

      /**
       * Set bwd dz.
       */
      void setDZbwd(double dz) {m_dZbwd = dz;}

      /**
       * Get bwd dz.
       */
      double getDZbwd() const {return m_dZbwd;}

      /**
       * Set offset.
       */
      void setOffset(double offset) {m_offset = offset;}

      /**
       * Get offset.
       */
      double getOffset() const {return m_offset;}

      /**
       * Set Nshifts.
       */
      void setNShifts(int n) {m_nShifts = n;}

      /**
       * Get Nshifts.
       */
      int getNShifts() const {return m_nShifts;}

      /**
       * Set number of wires.
       */
      void setNWires(int n) {m_nWires = n;}

      /**
       * Get number of wires.
       */
      int  getNWires() const { return m_nWires;}


    private:

      int m_id; /**< ID. */
      double m_r;     /**< r min. */
      double m_zfwd;     /**< z fwd. */
      double m_zbwd;     /**< z bwd. */
      double m_dZfwd;   /**< dz fwd. */
      double m_dZbwd;   /**< dz fwd. */
      double m_offset;  /**< offset. */
      int m_nWires; /**< Number of wires. */
      double m_nShifts; /**< shift. */

      /** Makes objects storable. */
      ClassDef(SenseLayer, 1);
    };

    /**
     * Field layer geometry parameters.
     */
    class FieldLayer : public TObject {

    public:

      /**
       * Constructor.
       */
      FieldLayer() {}

      /**
       * Constructor.
       */
      FieldLayer(int id,
                 double r, double zfwd, double zbwd):
        m_id(id), m_r(r),
        m_zfwd(zfwd), m_zbwd(zbwd)
      {
      }

      /**
       * Destructor.
       */
      ~FieldLayer() {}

      /**
       * Get ID.
       */
      int  getId() const { return m_id;}

      /**
       * Get Radius.
       */
      double getR() const {return m_r;}

      /**
       * Get fwd z-position.
       */
      double getZfwd() const { return m_zfwd;}

      /**
       * Get bwd z-position.
       */
      double getZbwd() const { return m_zbwd;}

    private:

      int m_id; /**< ID. */
      double m_r;     /**< r min. */
      double m_zfwd;     /**< z fwd. */
      double m_zbwd;     /**< z bwd. */

      /** Makes objects storable. */
      ClassDef(FieldLayer, 1);
    };

    /**
     * Outer wall geometry parameters.
     */
    class OuterWall : public TObject {

    public:

      /**
       * Constructor.
       */
      OuterWall() {}

      /**
       * Constructor.
       */
      OuterWall(const std::string& name, int id,
                double rmin, double rmax, double zfwd, double zbwd):
        m_name(name), m_id(id), m_rmin(rmin),
        m_rmax(rmax), m_zfwd(zfwd), m_zbwd(zbwd)
      {
      }

      /**
       * Destructor.
       */
      ~OuterWall() {}

      /**
       * Get name.
       */
      std::string getName() const { return m_name;}

      /**
       * Get ID.
       */
      int  getId() const { return m_id;}

      /**
       * Get Rmin.
       */
      double getRmin() const { return m_rmin;}

      /**
       * Get Rmax.
       */
      double getRmax() const {return m_rmax;}

      /**
       * Get fwd z-position.
       */
      double getZfwd() const { return m_zfwd;}

      /**
       * Get bwd z-position.
       */
      double getZbwd() const { return m_zbwd;}

    private:

      std::string m_name; /**< Name. */
      int m_id; /**< ID. */
      double m_rmin;     /**< r min. */
      double m_rmax;     /**< r max. */
      double m_zfwd;     /**< z fwd. */
      double m_zbwd;     /**< z bwd. */


      /** Makes objects storable. */
      ClassDef(OuterWall, 1);
    };

    /**
     * Inner wall geometry parameters.
     */
    class InnerWall : public TObject {

    public:

      /**
       * Constructor.
       */
      InnerWall() {}

      /**
       * Constructor.
       */
      InnerWall(const std::string& name, int id,
                double rmin, double rmax, double zfwd, double zbwd):
        m_name(name), m_id(id), m_rmin(rmin),
        m_rmax(rmax), m_zfwd(zfwd), m_zbwd(zbwd)
      {
      }

      /**
       * Destructor.
       */
      ~InnerWall() {}

      /**
       * Get name.
       */
      std::string getName() const { return m_name;}

      /**
       * Get ID.
       */
      int  getId() const { return m_id;}

      /**
       * Get Rmin.
       */
      double getRmin() const { return m_rmin;}

      /**
       * Get Rmax.
       */
      double getRmax() const {return m_rmax;}

      /**
       * Get fwd z-position.
       */
      double getZfwd() const { return m_zfwd;}

      /**
       * Get bwd z-position.
       */
      double getZbwd() const { return m_zbwd;}

    private:

      std::string m_name; /**< Name. */
      int m_id; /**< ID. */
      double m_rmin;     /**< r min. */
      double m_rmax;     /**< r max. */

      double m_zfwd;     /**< z fwd. */
      double m_zbwd;     /**< z bwd. */


      /** Makes objects storable. */
      ClassDef(InnerWall, 1);
    };

    /**
     * Endplate layer geometry parameters.
     */
    class EndPlateLayer : public TObject {

    public:

      /**
       * Constructor.
       */
      EndPlateLayer() {}

      /**
       * Constructor.
       */
      EndPlateLayer(const std::string& name, int ilay,
                    double rmin, double rmax, double zfwd, double zbwd):
        m_name(name), m_layer(ilay), m_rmin(rmin),
        m_rmax(rmax), m_zfwd(zfwd), m_zbwd(zbwd)
      {
      }

      /**
       * Destructor.
       */
      ~EndPlateLayer() {}

      /**
       * Get name.
       */
      std::string getName() const { return m_name;}

      /**
       * Get ILayer.
       */
      int getILayer() const { return m_layer;}

      /**
       * Get Rmin.
       */
      double getRmin() const { return m_rmin;}

      /**
       * Get Rmax.
       */
      double getRmax() const {return m_rmax;}

      /**
       * Get fwd z-position.
       */
      double getZfwd() const { return m_zfwd;}

      /**
       * Get bwd z-position.
       */
      double getZbwd() const { return m_zbwd;}

    private:

      std::string m_name; /**< Name. */
      int m_layer; /**< Layer. */
      double m_rmin;     /**< r min. */
      double m_rmax;     /**< r max. */
      double m_zfwd;     /**< z fwd. */
      double m_zbwd;     /**< z bwd. */

      /** Makes objects storable. */
      ClassDef(EndPlateLayer, 1);
    };

    /**
     * Endplate geometry parameters.
     */
    class EndPlate : public TObject {

    public:
      /**
       * Constructor.
       */
      EndPlate() {}

      /**
       * Constructor.
       */
      explicit EndPlate(int i) :  m_id(i) {};

      /**
       * Destructor.
       */
      ~EndPlate() {}

      /**
       * Add new endplate layer.
       */
      void appendNew(const std::string& name, int ilay,
                     double rmin, double rmax, double zfwd, double zbwd)
      {
        EndPlateLayer eplayer(name, ilay, rmin, rmax, zfwd, zbwd);
        m_endPlateLayers.push_back(eplayer);
      }

      /**
       * Get the number of endplate layers.
       */
      int getNEndPlateLayers() const {return m_endPlateLayers.size();}

      /**
       * Get ID.
       */
      int getId() const {return m_id;}

      /**
       * Get i-th endplate layer.
       */
      EndPlateLayer getEndPlateLayer(int i) const {return m_endPlateLayers.at(i);}
      /**
       * Get the list of endplate layers.
       */
      std::vector<EndPlateLayer> getEndPlateLayers() const {return m_endPlateLayers;}
    private:
      int m_id; /**< Layer ID. */
      std::vector<EndPlateLayer> m_endPlateLayers; /**< Endplate layers. */

      /** Makes objects storable. */
      ClassDef(EndPlate, 1);
    };

    /**
     * Default constructor.
     */
    CDCGeometry() {};

    /**
     * Constructor.
     */
    explicit CDCGeometry(const GearDir&);

    /**
     * Destructor.
     */
    ~CDCGeometry();

    /**
     * Get geometry parameters from Gearbox.
     */
    void read(const GearDir&);

    /**
     * Get the global phi rotation of CDC.
     */
    double getGlobalPhiRotation() const { return m_globalPhiRotation;}

    /**
     * Get the global x offset of CDC wrt Belle2 coord. system.
     */
    double getGlobalOffsetX() const { return m_globalOffsetX;}

    /**
     * Get the global y offset of CDC wrt Belle2 coord. system.
     */
    double getGlobalOffsetY() const { return m_globalOffsetY;}

    /**
     * Get the global z offset of CDC wrt Belle2 coord. system.
     */
    double getGlobalOffsetZ() const { return m_globalOffsetZ;}

    /**
     * Get the fiducial Rmin of CDC sensitive volume.
     */
    double getFiducialRmin() const { return m_fiducialRmin;}

    /**
     * Get the fiducial Rmax of CDC sensitive volume.
     */
    double getFiducialRmax() const { return m_fiducialRmax;}

    /**
     * Get the mother volume geometry of CDC.
     */
    MotherVolume getMotherVolume() const { return m_mother;}

    /**
     * Get the list of outer walls.
     */
    std::vector<OuterWall> getOuterWalls() const { return m_outerWalls;}

    /**
     * Get the i-th outer wall.
     */
    OuterWall getOuterWall(int i) const { return m_outerWalls.at(i);}

    /**
     * Get the list of inner walls.
     */
    std::vector<InnerWall> getInnerWalls() const { return m_innerWalls;}

    /**
     * Get the i-th inner wall.
     */
    InnerWall getInnerWall(int i) const { return m_innerWalls.at(i);}

    /**
     * Get the number of endplates.
     */
    int getNEndplates() const { return m_endplates.size();}

    /**
     * Get the i-th endplate.
     */
    EndPlate getEndPlate(int i) const {return m_endplates.at(i);};

    /**
     * Get the list of endplates.
     */
    std::vector<EndPlate> getEndPlates() const { return m_endplates;}

    /**
     * Get the number of frontend layers.
     */
    int getNFrontends() const {return m_frontends.size();}

    /**
     * Get the list of frontend layers.
     */
    std::vector<Frontend> getFrontends() const {return m_frontends;}

    /**
     * Get i-th sense layer.
     */
    SenseLayer getSenseLayer(int i) const { return m_senseLayers.at(i);}

    /**
     * Get the list of sense layers.
     */
    std::vector<SenseLayer> getSenseLayers() const { return m_senseLayers;}

    /**
     * Get the number of sense layers.
     */
    int getNSenseLayers() const { return m_senseLayers.size();}

    /**
     * Get the diameter of sense wire.
     */
    double getSenseDiameter() const { return m_senseWireDiameter;}

    /**
     * Get the tension applied to sense wire.
     */
    double getSenseTension() const { return m_senseWireTension;}

    /**
     * Get the number of sense wires.
     */
    int getNSenseWires() const { return m_senseWireNumbers;}

    /**
     * Get the i-th field layer.
     */
    FieldLayer getFieldLayer(int i) const { return m_fieldLayers.at(i);}

    /**
     * Get the list of field layers.
     */
    std::vector<FieldLayer> getFieldLayers() const { return m_fieldLayers;}

    /**
     * Get the number of field layers.
     */
    int getNFieldLayers() const { return m_fieldLayers.size();}


    /**
     * Get the list of neutron shields.
     */
    std::vector<NeutronShield> getNeutronShields() const { return m_neutronShields;}

    /**
     * Get the list of covers.
     */
    std::vector<Cover> getCovers() const { return m_covers;}

    /**
     * Get the list of ribs.
     */
    std::vector<Rib> getRibs() const { return m_ribs;}


    /**
     * Get the diameter of field wire.
     */
    double getFieldDiameter() const { return m_fieldWireDiameter;}

    /**
     * Get the number of field wires.
     */
    int getNFieldWires() const { return m_fieldWireNumbers;}

    /**
     * Get the length of feedthrough.
     */
    double getFeedthroughLength() const {return m_feedThroughLength;}

    /**
     * Get the debug mode.
     */
    bool getDebugMode() const {return m_debug;}

    /**
     * Get the material definition mode.
     */
    int getMaterialDefinitionMode() const {return m_materialDefinitionMode;}


    /**
     * Get the definition mode of sense z-position.
     */
    int getSenseWireZposMode() const {return m_senseWireZposMode;}

    /**
     * Get the clock frequency.
     */
    double getClockFrequency() const { return m_clockFrequency;}

    /**
     * Get the nominal space resolution.
     */
    double getNominalSpaceResolution() const { return m_nominalSpaceResolution;}

    /**
     * Get the displacement mode.
     */
    bool getDisplacement() const { return m_displacement;}

    /**
     * Get the misalignment mode.
     */
    bool getMisalignment() const { return m_misalignment;}

    /**
     * Get the alignment mode.
     */
    bool getAlignment() const { return m_alignment;}

    /**
     * Get the file name for the displacement.
     */
    std::string getDisplacementFile() const { return m_displacementFile;}

    /**
     * Get the file name for the misalignment.
     */
    std::string getMisalignmentFile() const { return m_misalignmentFile;}

    /**
     * Get the file name for the salignment.
     */
    std::string getAlignmentFile() const { return m_alignmentFile;}

    /**
     * Get the threshold of energy deposition in geant4.
     */
    double getEnergyDepositThreshold() const
    {
      return m_thresholdEnergyDeposit;
    }

    /**
     * Get the minimum track length.
     */
    double getMinimumTrackLength() const
    {
      return m_minTrackLength;
    }

    /**
     * Get the wire sag mode.
     */
    bool getWireSagMode() const
    {
      return m_wireSag;
    }

    /**
     * Get the modified left/right definition mode.
     */
    bool getModifiedLeftRightFlag() const
    {
      return m_modLeftRightFlag;
    }


  private:

    double m_globalPhiRotation = 0.0; /**< Global rotation in phi (degrees). */
    double m_globalOffsetX = 0.0; /**< The offset of the whole cdc in x with respect to the IP (cm). */
    double m_globalOffsetY = 0.0; /**< The offset of the whole cdc in y with respect to the IP (cm). */
    double m_globalOffsetZ = 0.0; /**< The offset of the whole cdc in z with respect to the IP (cm). */
    double m_fiducialRmin; /**< Minimum radius of the CDC fiducial volume. */
    double m_fiducialRmax; /**< Maximum radius of the CDC fiducial volume. */
    MotherVolume m_mother;    /**< CDC mother volume. */
    std::vector<SenseLayer> m_senseLayers; /**< Sense layer. */
    std::vector<FieldLayer> m_fieldLayers; /**< Field layer. */
    std::vector<InnerWall> m_innerWalls;/**< Inner wall. */
    std::vector<OuterWall> m_outerWalls;/**< Outer wall. */
    std::vector<EndPlate> m_endplates;/**< Endplate. */
    std::vector<Frontend> m_frontends;/**< Electronics board. */
    std::vector<NeutronShield> m_neutronShields;/**< Neutron shields. */
    std::vector<Cover> m_covers;/**< Covers. */
    std::vector<Rib> m_ribs;/**< Ribs. */

    double m_senseWireDiameter = 0.030; /**< Sense wire diameter (mm). */
    int m_senseWireNumbers = 14336;/**< Number of sense wires. */
    double m_senseWireTension = 50.0; /**< Sense wire tension. */
    //    double m_senseSpaceResolution = 0.0130; /**< Nominal space resolution (cm). */
    double m_fieldWireDiameter = 0.126; /**< Field wire diameter (mm). */
    int m_fieldWireNumbers = 42240; /**< Number of field wires. */
    double m_feedThroughLength = 15.0; /**< Feedthrough length (mm). */
    bool m_debug;/**< Debug mode. */
    int m_materialDefinitionMode;/**< Material definition mode. */
    int m_senseWireZposMode; /**< Sense wire z-position definition mode. */
    double m_clockFrequency; /**< Clock frequency. */
    double m_nominalSpaceResolution;/**< Nominal space resolution. */

    bool m_displacement; /**< Displacement mode. */
    bool m_misalignment; /**< Misalignment mode. */
    bool m_alignment; /**< Alignment mode. */
    std::string m_displacementFile; /**< Displacement file. */
    std::string m_misalignmentFile; /**< Misalignment file. */
    std::string m_alignmentFile; /**< Alignment file. */

    double m_thresholdEnergyDeposit; /*!< Energy thresh. for G4 step */
    double m_minTrackLength;         /*!< Minimum track length for G4 step */
    bool m_wireSag;        /*!< Switch for sense wire sag */
    bool m_modLeftRightFlag; /*!< Switch for modified left/right flag */

    ClassDef(CDCGeometry, 2);  /**< ClassDef, must be the last term before the closing {}*/

  };
} // end of namespace Belle2

