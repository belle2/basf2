/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

//#include <vxd/dbobjects/VXDPolyConePar.h>
#include <TObject.h>
#include <string>
#include <vector>


namespace Belle2 {

  class GearDir;


  /*
  <Support>
    <HalfShell>
      <RotationSolid> "foward" </RotationSolid>  -> like VXDEnvelope
      <RotationSolid> "backward" </RotationSolid>  -> like VXDEnvelope
      <RotationSolid> "outer cover" </RotationSolid>  -> like VXDEnvelope
      <RotationSolid> "BackwardRing" </RotationSolid>  -> like VXDEnvelope
      <RotationSolid> "ForwardRing" </RotationSolid>  -> like VXDEnvelope
      <RotationSolid> "BackwardEnd" </RotationSolid>  -> like VXDEnvelope
      <RotationSolid> "ForwardEnd" </RotationSolid>  -> like VXDEnvelope
    </HalfShell>
    <Endrings>
        <Material>Fe</Material>
  <!-- Leave this amount of space to the YZ plane for alignment purposes -->
  <gapWidth unit="um">100</gapWidth>
  <!-- length in z direction of the endrings -->
  <length unit="mm"> 15.000</length>
  <!-- thickness of the base ring on the carbon cone -->
  <baseThickness unit="mm">6.000</baseThickness>
  <Layer id="4">
          <Endring name="Forward"> </Endring>
          <Endring name="Backward"> </Endring>
        </Layer>
        <Layer id="5">
          <Endring name="Forward"> </Endring>
          <Endring name="Backward"> </Endring>
        </Layer>
        <Layer id="6">
          <Endring name="Forward"> </Endring>
          <Endring name="Backward"> </Endring>
        </Layer>
    </Endrings>
    <SupportRibs>
        <Layer id="3">
          <Endmount name="Forward"> </Endmount>
          <Endmount name="Backward"> </Endmount>
          nx <box> </box>
          nx <tab> </tab>
        </Layer>
        //!same for other layers
    </SupportRibs>
    <CoolingPipes>
  <Material>Fe</Material>
  <outerDiameter unit="mm">1.500</outerDiameter>
  <wallThickness unit="mm">0.050</wallThickness>
  <Layer id="4">
      <!-- number of pipes for this layer/halfshell -->
      <nPipes>6</nPipes>
      <!-- phi angle of the center of the first pipe -->
      <startPhi unit="deg">-82.000</startPhi>
      <!-- phi angle between two pipes -->
      <deltaPhi unit="deg"> 36.000</deltaPhi>
      <!-- diameter difference of the last bend (optional) -->
      <deltaL unit="mm"> 14.500</deltaL>
      <!-- distance between IP and center of the pipes -->
      <radius   unit="mm">  87.000</radius>
      <!-- beginning of pipe in z -->
      <zstart   unit="mm">-163.780</zstart>
      <!-- end of pipe in z -->
      <zend     unit="mm">  97.380</zend>
  </Layer>
  //! similar for layer 5,6
    </CoolingPipes>
  </Support>
  */


  /**
  * The Class for VXD support
  */

  class SVDSupportPar: public TObject {

  public:

    //! Default constructor
    SVDSupportPar() {}
    //! Constructor using Gearbox
    explicit SVDSupportPar(const GearDir& content) { read(content); }
    //! Destructor
    ~SVDSupportPar() {}
    //! Get geometry parameters from Gearbox
    void read(const GearDir&);
    //! Get endflanges
    //std::vector<VXDPolyConePar> getEndflanges(void) const { return m_endflanges; }

  private:
    //! Endflanges
    //std::vector<VXDPolyConePar> m_endflanges;

    ClassDef(SVDSupportPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };
} // end of namespace Belle2

