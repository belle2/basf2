/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj, Rok Pestotnik                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHBTESTGEOMETRYPAR_H
#define ARICHBTESTGEOMETRYPAR_H

#include <utility>
#include <vector>
#include <string>
#include <map>
#include <framework/gearbox/GearDir.h>
#include "TRotation.h"
#include "TVector3.h"
#include "TVector2.h"


#include <G4ThreeVector.hh>
#include <G4MaterialPropertyVector.hh>
#include <cmath>
#include <boost/format.hpp>
#include <iomanip>


namespace Belle2 {
  //!  Beamtest ARICH Geometry Tracking Class
  /*! Contains the parameters of the 4 MWPCs used for tracking
  */
  class ARICHTracking {
  public:
    //! Default Constructor
    ARICHTracking():
      atdc(0)
    {
      for (int i = 0; i < 4; i++) tdc[i] = 0;
      for (int i = 0; i < 2; i++) {
        slp[i] = 0;
        offset[i] = 0;
        cutll[i] = 0;
        cutul[i] = 0;
        diff[i] = 0;
        sum[i] = 0;
        status[i] = 0;
      }
      for (int i = 0; i < 3; i++) {
        pos[i] = 0;
        reco[i] = 0;
      }
    };
    //! Default destructor
    ~ARICHTracking() {};
    //! TDC of the 4 cathode signals
    int tdc[4];
    //! TDC of the anode signal
    int atdc;
    //! Calibration constants of the MWPC (\delta x= slope \delta t + offset) - slopes for x an y direction
    float slp[2];
    //! Calibration constants of the MWPC - offsets for x an y direction
    float offset[2];
    //! Cuts on the tdc sums - lower levels
    int cutll[2];
    //! Cuts on the tdc sums - upper levels
    int cutul[2];
    //! MWPC chamber position
    float pos[3];

    //! TDC difference
    int diff[2];
    //! TDC sum
    int sum[2];
    //! Status of the position determination
    int status[2];
    //! Reconstructed position of the hit
    double reco[3];
    //!  Debug printouts
    void Print()
    {
      printf("----------------------------------------------------------------\n");
      for (int i = 0; i < 4; i++) printf("tdc%d= %d\t", i, tdc[i]);
      printf("\n");
      for (int i = 0; i < 1; i++) printf("atdc%d= %d\t", i, atdc);
      printf("\n");
      for (int i = 0; i < 2; i++) printf("slp%d= %3.3f\t", i, slp[i]);
      printf("\n");
      for (int i = 0; i < 2; i++) printf("off%d= %3.3f\t", i, offset[i]);
      printf("\n");
      for (int i = 0; i < 2; i++) printf("cutll%d= %d\t", i, cutll[i]);
      printf("\n");
      for (int i = 0; i < 2; i++) printf("cutul%d= %d\t", i, cutul[i]);
      printf("\n");
      for (int i = 0; i < 3; i++) printf("pos%d= %3.3f\t", i, pos[i]);
      printf("\n");
    };
  };

  //! The Class for ARICH Beamtest Geometry Parameters
  /*! This class provides ARICH gemetry paramters for simulation, reconstruction and so on.
    These parameters are gotten from gearbox.
  */


  class ARICHBtestGeometryPar {

  public:

    //! Constructor
    ARICHBtestGeometryPar();

    //! copy constructor
    ARICHBtestGeometryPar(const ARICHBtestGeometryPar& arichPar);

    //! Destructor
    virtual ~ARICHBtestGeometryPar();

    //! Static method to get a reference to the ARICHBtestGeometryPar instance.
    /*!
    \return A reference to an instance of this class.
    */
    static ARICHBtestGeometryPar* Instance();
    //! calculates detector parameters needed for geometry build and reconstruction.
    void Initialize(const GearDir& content);
    //! calculates detector parameters needed for geometry build and reconstruction.
    void Initialize(const GearDir& modulecontent, const GearDir& mirrorcontent);


    //! Clears
    void clear(void);

    //! Print some debug information
    void Print(void) const;

    //! gets geometry parameters from gearbox.
    void read(const GearDir& content);


    //! Set the tracking shift
    void setTrackingShift(TVector3&);
    //! Get the tracking shift
    TVector3 getTrackingShift();
    //! Set the rotation center of the Aerogel RICH frame
    void setRotationCenter(TVector3&);
    //! Get the rotation center of the Aerogel RICH frame
    TVector3 getRotationCenter();

    //! Set the rotation angle of the Aerogel RICH frame
    void setFrameRotation(double);
    //! Get the rotation matrix of the Aerogel RICH frame
    TRotation getFrameRotation();
    //! Set the flag for the reconstruction by using the average aerogel refractive index
    void setAverageAgel(bool);
    //! Get the flag for the reconstruction by using the average aerogel refractive index
    bool getAverageAgel();
    //! Set the pointer of the tracking MWPCs
    void setMwpc(ARICHTracking* m_mwpc);
    //! Get the pointer of the tracking MWPCs
    ARICHTracking* getMwpc();
    //! Get the position of the HAPD channel
    std::pair<double, double> GetHapdChannelPosition(int);
    //! Get the mapping of the electronic channel to the HAPD module nr and the channel number
    std::pair<int, int> GetHapdElectronicMap(int);
    //! Set the position of the HAPD channel
    int AddHapdChannelPositionPair(double , double);
    //! Set the mapping of the electronic channel to the HAPD module nr and the channel number
    int AddHapdElectronicMapPair(int, int);
    //! Set of the setup global offset
    void setOffset(TVector3&);
    //! Get the setup global offset
    TVector3 getOffset();


  private:
    //! Pointer to the tracking chambers
    ARICHTracking* m_mwpc;
    //! Global offset
    TVector3       m_offset;
    //! Tracking shift
    TVector3       m_trackingShift;
    //! rotation center of the Aerogel RICH frame
    TVector3       m_rotationCenter;
    //! rotation matrix of the Aerogel RICH frame
    TRotation      m_frameRotation;
    //! flag for the reconstruction by using the average aerogel refractive index
    bool           m_averageAgel;
    //! the position of the HAPD channels
    std::vector < std::pair<double, double> > m_hapdmap;
    //! mapping of the electronic channel to the HAPD module nr and the channel number
    std::vector < std::pair<int, int> > m_hapdeid;

    int m_nPads;                           /*!< Number of detector module pads */

    bool m_init;                          /*!< True if parametrization is already initialized */
    bool m_simple;                        /*!< True if parametrization initialized with simple geometry (beamtest) */



    static ARICHBtestGeometryPar* p_B4ARICHBtestGeometryParDB; /*!< Pointer that saves the instance of this class. */

    // vectors holding information on HAPDs and chips and pads positions.


  };

  //-----------------------------------------------------------------------------

} // end of namespace Belle2

#endif

