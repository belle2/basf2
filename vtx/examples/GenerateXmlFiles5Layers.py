#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# create 5 layers staggered with last obelix design
# using super modules 1x4, 1x3 sensors

OptL3Mod = True
Opt2Xi = False

fileName = ""
fileNameC = ""

if OptL3Mod:
    if Opt2Xi:
        fileName = 'VTX-CMOS5-staggered-L3mod-2Xi.xml'
        fileNameC = 'VTX-Components-CMOS5-staggered-L3mod-2Xi.xml'
    else:
        fileName = 'VTX-CMOS5-staggered-L3mod.xml'
        fileNameC = 'VTX-Components-CMOS5-staggered-L3mod.xml'
else:
    if Opt2Xi:
        fileName = 'VTX-CMOS5-staggered-2Xi.xml'
        fileNameC = 'VTX-Components-CMOS5-staggered-2Xi.xml'
    else:
        fileName = 'VTX-CMOS5-staggered.xml'
        fileNameC = 'VTX-Components-CMOS5-staggered.xml'

f = open('../data/' + fileNameC, 'w')

Material = "Si"
X0Si = 0.0937
X = 0.00439
Xi = 0.001  # 0.002
sensorL = 30.168
sensorW = 18.812
sensorH = 0.400  # str(X0Si*X*1e3)
sensorHi = 0.95  # str(X0Si*Xi*1e3)
offsetU = 3.5
offsetV = 0.3

activeL = 29.568
activeW = 15.312
activeH = 0.030

pixelsU = 464
pixelsV = 896


BulkDoping = 10.0
BackVoltage = -30
TopVoltage = -5.0
BorderU = 6.0
BorderV = 4.0
ChargeThreshold = 1.25
NoiseFraction = 0.00000000
ApplyElectronicEffects = 'true'
ElectronicNoise = 20.0
ApplyBinaryReadout = 'false'
BinaryHitThreshold = 200
ElectronToADU = 120
MaxADUCode = 127
ApplyPoissonSmearing = 'true'
ApplyIntegrationWindow = 'true'
SegmentLength = 0.005
ElectronGroupSize = 100
ElectronStepTime = 1.0
ElectronMaxSteps = 200
HardwareDelay = 0.0
ADCunit = 120
CloudSize = 0.000085
Gq = 1.0
X0average = 0.001
TanLorentzAngle = 0.24
ResolutionCoefficientU = 0.6
ResolutionCoefficientV = 0.6
IntegrationStart = -50
IntegrationEnd = 50
unitThinU = 2.0
unitThinV = 0.0

nlayer = 5
nsensor = 0
type = ""
start = 0
shift = ""
shiftR0 = "0.0"
shiftR1 = "1.5"
shift1 = 0.
gap = 0
shiftL = 0
radius = 0

f.write('<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n')
f.write('<Components>\n')
f.write('   <!-- VTX layout with 5 layers in a windmill structure\n')
f.write('      -->\n')

for layer in range(1, nlayer+1):

    if layer == 1:
        type = 'layer14i'
        nsensor = 1
        start = 0
        shift = str(2.9)
        gap = 0.1
        shiftL = 2.9
        radius = 14.0

    if layer == 2:
        type = 'layer14i'
        nsensor = 1
        start = 0
        shift = str(6.0)
        gap = 0.1
        shiftL = 6.0
        radius = 22.0

    if layer == 3:
        type = 'layer14'
        if not OptL3Mod:
            nsensor = 3
            start = -112.186 + 2*sensorL
            shift = str(8.0)
            gap = 0.05
            shiftL = 8.635
            radius = 69.0
        else:
            nsensor = 2
            start = -60.150 + 2*sensorL
            shift = str(8.0)
            gap = 0.05
            shiftL = 8.635
            radius = 39.0

    if layer == 4:
        type = 'layer13'
        nsensor = 5
        start = -141.874 + 1.5*sensorL
        shift = str(12.042)
        gap = 0.05
        shiftL = 12.084
        radius = 89.0

    if layer == 5:
        type = 'layer14'
        nsensor = 6
        start = -237.859 + 2*sensorL
        shift = str(5.818)
        shift1 = str(23.518)
        gap = 0.05
        shiftL = 0
        radius = 140.0

    f.write('  <Ladder layer=\"' + str(layer) + '\">\n')
    if (layer == 1):
        f.write('    <!-- shift defines the shift of the sensor center along the rphi direction\n')
        f.write('         of the ladder to create the windmill structure -->\n')
    f.write('    <shift  unit=\"mm\">' + str(shiftL) + '</shift>\n')
    if (layer == 1):
        f.write('    <!-- radius (together with the phi angle specified in VTX.xml for each\n')
        f.write('         ladder) specifies the center of the local coordinate system for the\n')
        f.write('         ladder: x along rphi, 0 at specified phi, y going outward 0 at\n')
        f.write('         specified radius, z = global z, 0 at global 0 -->\n')
        f.write('\n')
        f.write('    <!-- The nominal radius of the inner layer is 14 mm, but this is the inner\n')
        f.write('         surface of the sensor and here we give the center of the sensor, so\n')
        f.write('         let\'s add half of the thickness of supporting balcony (0.2mm) -->\n')
    f.write('    <radius unit=\"mm\">' + str(radius) + '</radius>\n')

    if (layer == 1):
        f.write('    <!-- The value of Sensors is the global z position -->\n')

    for id in range(0, nsensor):

        if (layer == 4):
            pos = start + (3*sensorL+gap)*id
        else:
            pos = start + (4*sensorL+gap)*id

        if not OptL3Mod:
            if layer == 3 and id == 1:
                pos = start + (3*sensorL+gap)*id

        posZ = str(pos)

        cmd1 = '    <Sensor id=\"' + str(id+1) + '\"' + ' type=\"' + type + '\" flipV=\"true\"><z unit=\"mm\">' + \
            posZ[0:7] + '</z><shift unit=\"mm\">' + shift[0:6] + '</shift><shiftR unit=\"mm\">' + shiftR0 + '</shiftR></Sensor>\n'

        f.write(cmd1)

    if (layer == 5):
        for id in range(0, nsensor):

            pos = start + (4*sensorL+gap)*id
            posZ = str(pos)

            cmd2 = '    <Sensor id=\"' + str(id+nsensor+1) + '\"' + ' type=\"' + type + '\" flipV=\"true\"><z unit=\"mm\">' + \
                posZ[0:7] + '</z><shift unit=\"mm\">' + shift1 + '</shift><shiftR unit=\"mm\">' + shiftR1 + '</shiftR></Sensor>\n'

            f.write(cmd2)

    f.write('  </Ladder>\n')

    f.write('\n')

f.write('<Sensor type=\"layer14\">\n')
f.write('  <Material>' + Material + '</Material>\n')
f.write('  <width unit=\"mm\">' + str(sensorW) + '</width>\n')
f.write('  <length unit=\"mm\">' + str(sensorL*4) + '</length>\n')
f.write('  <height unit=\"mm\">' + str(sensorH) + '</height>\n')
f.write('  <!-- definition of the active area of the module which is a part of the\n')
f.write('       whole thing. The position is given relative to the lower left edge of\n')
f.write('       the module (the point with the most negative local coordinates). -->\n')
f.write('  <Active>\n')
f.write('    <u unit=\"mm\">' + str(offsetU) + '</u>\n')
f.write('    <v unit=\"mm\">' + str(offsetV) + '</v>\n')
f.write('    <w>top</w>\n')
f.write('    <width unit=\"mm\">' + str(activeW) + '</width>\n')
f.write('    <length unit=\"mm\">' + str(activeL*4) + '</length>\n')
f.write('    <height unit=\"mm\">' + str(activeH)+'</height>\n')
f.write('    <!-- define the number of pixels in rphi direction -->\n')
f.write('    <pixelsU>' + str(pixelsU) + '</pixelsU>\n')
f.write('    <pixelsV>' + str(pixelsV*4) + '</pixelsV>\n')
f.write('     <!--\n')
f.write('           Maybe all those parameters don\'t make sense for Generic Pixel\n')
f.write('\n')
f.write('           Parameters describing the physical attributes of the sensor needed\n')
f.write('           for digitization. The voltages might be time dependent and than\n')
f.write('           should be placed somewhere else -->\n')
f.write('      <BulkDoping unit=\"1/um^3\">' + str(BulkDoping) + '</BulkDoping>\n')
f.write('      <BackVoltage unit=\"V\">' + str(BackVoltage) + '</BackVoltage>\n')
f.write('      <TopVoltage unit=\"V\">' + str(TopVoltage) + '</TopVoltage>\n')
f.write('      <BorderU unit=\"um\">' + str(BorderU) + '</BorderU>\n')
f.write('      <BorderV unit=\"um\">' + str(BorderV) + '</BorderV>\n')
f.write('      <ChargeThreshold unit=\"ADU\">' + str(ChargeThreshold) + '</ChargeThreshold>\n')
f.write('      <NoiseFraction>' + str(NoiseFraction) + '</NoiseFraction>\n')
f.write('      <!-- Parameters that were in the digitizer -->\n')
f.write('      <!-- Apply electronic effects? -->\n')
f.write('      <ApplyElectronicEffects>' + ApplyElectronicEffects + '</ApplyElectronicEffects>\n')
f.write('      <!-- Noise added by the electronics, set in ENC -->\n')
f.write('      <ElectronicNoise unit=\"ENC\">' + str(ElectronicNoise) + '</ElectronicNoise>\n')
f.write('      <!-- Apply binary readout? -->\n')
f.write('      <ApplyBinaryReadout>' + ApplyBinaryReadout + '</ApplyBinaryReadout>\n')
f.write('      <!-- Binary hit treshold, set in ENC -->\n')
f.write('      <BinaryHitThreshold unit=\"ENC\">' + str(BinaryHitThreshold) + '</BinaryHitThreshold>\n')
f.write('      <!-- ENC equivalent of 1 ADU -->\n')
f.write('      <ElectronToADU unit=\"ENC\">' + str(ElectronToADU) + '</ElectronToADU>\n')
f.write('      <!-- Maximum code for analog to digital converter (clamping) -->\n')
f.write('      <MaxADUCode>' + str(MaxADUCode) + '</MaxADUCode>\n')
f.write('      <!-- Apply Poisson smearing of electrons collected on pixels -->\n')
f.write('      <ApplyPoissonSmearing>' + str(ApplyPoissonSmearing) + '</ApplyPoissonSmearing>\n')
f.write('      <!-- Use integration window? -->\n')
f.write('      <ApplyIntegrationWindow>' + str(ApplyIntegrationWindow) + '</ApplyIntegrationWindow>\n')
f.write('      <!-- Maximum segment length (in mm) -->\n')
f.write('      <SegmentLength unit=\"mm\">' + str(0.005) + '</SegmentLength>\n')
f.write('      <!-- Split Signalpoints in smaller groups of N electrons (in e) -->\n')
f.write('      <ElectronGroupSize>' + str(ElectronGroupSize) + '</ElectronGroupSize>\n')
f.write('      <!-- Time step for tracking electron groups in readout plane (in ns) -->\n')
f.write('      <ElectronStepTime unit=\"ns\">' + str(ElectronStepTime) + '</ElectronStepTime>\n')
f.write('      <!-- Maximum number of steps when propagating electrons -->\n')
f.write('      <ElectronMaxSteps>' + str(ElectronMaxSteps) + '</ElectronMaxSteps>\n')
f.write('      <!-- Constant time delay between bunch crossing and switching on triggergate (in ns) -->\n')
f.write('      <HardwareDelay unit=\"ns\">' + str(HardwareDelay) + '</HardwareDelay>\n')
f.write('      <!-- ADC conversion factor -->\n')
f.write('      <ADCunit>' + str(ADCunit) + '</ADCunit>\n')
f.write('      <!-- Diffusion coefficient TODO add unit? is it diffusion coeff or cloud size??-->\n')
f.write('      <CloudSize>' + str(CloudSize) + '</CloudSize>\n')
f.write('      <!-- ADC gain factor -->\n')
f.write('      <Gq>' + str(Gq) + '</Gq>\n')
f.write('      <!-- Averaged sensors X0 (for perpendicular incidence) -->\n')
f.write('      <X0average>' + str(X0average) + '</X0average>\n')
f.write('      <!-- Tangent of Lorentz angle -->\n')
f.write('      <TanLorentzAngle>' + str(TanLorentzAngle) + '</TanLorentzAngle>\n')
f.write('      <!-- Spatial resolution coefficient for u axis -->\n')
f.write('      <ResolutionCoefficientU>' + str(ResolutionCoefficientU) + '</ResolutionCoefficientU>\n')
f.write('      <!-- Spatial resolution coefficient for v axis -->\n')
f.write('      <ResolutionCoefficientV>' + str(ResolutionCoefficientV) + '</ResolutionCoefficientV>\n')
f.write('      <!-- Integration time for Generic Pixel -->\n')
f.write('      <IntegrationStart unit=\"ns\">' + str(IntegrationStart) + '</IntegrationStart>\n')
f.write('      <IntegrationEnd   unit=\"ns\">' + str(IntegrationEnd) + '</IntegrationEnd>\n')
f.write('    </Active>\n')
f.write('    <!-- Used for thinning in the VXD sensitive area\n')
f.write('\n')
f.write('         now we place some components on the sensor by giving the name of the\n')
f.write('         component defined further down and the positions of all placements on\n')
f.write('         the sensor in local coordinates, starting at the lower left edge\n')
f.write('\n')
f.write('         possible placements for the w-coordinate are:\n')
f.write('          - below: place below the sensor surface\n')
f.write('          - bottom: place in the sensor but at the bottom\n')
f.write('          - center: place in the sensor center\n')
f.write('          - top: place in the sensor touching the top surface\n')
f.write('          - above place on top of the sensor surface -->\n')
f.write('\n')
f.write('     <!-- Thinning of the sensor down to the sensitive thickness by cutting out\n')
f.write('         a piece of the silicon -->\n')
f.write('    <!--Component type=\"ThinningLayer_iVTX_A\">\n')
f.write('      <u unit=\"mm\">' + str(unitThinU) + '</u><v unit=\"mm\">' + str(unitThinV) + '</v>\n')
f.write('    </Component-->\n')
f.write('  </Sensor>\n')

f.write('\n')

f.write('<Sensor type=\"layer14i'+'\">\n')
f.write('  <Material>' + Material + '</Material>\n')
f.write('  <width unit=\"mm\">' + str(sensorW) + '</width>\n')
f.write('  <length unit=\"mm\">' + str(sensorL*4) + '</length>\n')
f.write('  <height unit=\"mm\">' + str(sensorHi) + '</height>\n')
f.write('  <!-- definition of the active area of the module which is a part of the\n')
f.write('       whole thing. The position is given relative to the lower left edge of\n')
f.write('       the module (the point with the most negative local coordinates). -->\n')
f.write('  <Active>\n')
f.write('    <u unit=\"mm\">' + str(offsetU) + '</u>\n')
f.write('    <v unit=\"mm\">' + str(offsetV) + '</v>\n')
f.write('    <w>top</w>\n')
f.write('    <width unit=\"mm\">' + str(activeW) + '</width>\n')
f.write('    <length unit=\"mm\">' + str(activeL*4) + '</length>\n')
f.write('    <height unit=\"mm\">' + str(activeH)+'</height>\n')
f.write('    <!-- define the number of pixels in rphi direction -->\n')
f.write('    <pixelsU>' + str(pixelsU) + '</pixelsU>\n')
f.write('    <pixelsV>' + str(pixelsV*4) + '</pixelsV>\n')
f.write('     <!--\n')
f.write('           Maybe all those parameters don\'t make sense for Generic Pixel\n')
f.write('\n')
f.write('           Parameters describing the physical attributes of the sensor needed\n')
f.write('           for digitization. The voltages might be time dependent and than\n')
f.write('           should be placed somewhere else -->\n')
f.write('      <BulkDoping unit=\"1/um^3\">' + str(BulkDoping) + '</BulkDoping>\n')
f.write('      <BackVoltage unit=\"V\">' + str(BackVoltage) + '</BackVoltage>\n')
f.write('      <TopVoltage unit=\"V\">' + str(TopVoltage) + '</TopVoltage>\n')
f.write('      <BorderU unit=\"um\">' + str(BorderU) + '</BorderU>\n')
f.write('      <BorderV unit=\"um\">' + str(BorderV) + '</BorderV>\n')
f.write('      <ChargeThreshold unit=\"ADU\">' + str(ChargeThreshold) + '</ChargeThreshold>\n')
f.write('      <NoiseFraction>' + str(NoiseFraction) + '</NoiseFraction>\n')
f.write('      <!-- Parameters that were in the digitizer -->\n')
f.write('      <!-- Apply electronic effects? -->\n')
f.write('      <ApplyElectronicEffects>' + ApplyElectronicEffects + '</ApplyElectronicEffects>\n')
f.write('      <!-- Noise added by the electronics, set in ENC -->\n')
f.write('      <ElectronicNoise unit=\"ENC\">' + str(ElectronicNoise) + '</ElectronicNoise>\n')
f.write('      <!-- Apply binary readout? -->\n')
f.write('      <ApplyBinaryReadout>' + ApplyBinaryReadout + '</ApplyBinaryReadout>\n')
f.write('      <!-- Binary hit treshold, set in ENC -->\n')
f.write('      <BinaryHitThreshold unit=\"ENC\">' + str(BinaryHitThreshold) + '</BinaryHitThreshold>\n')
f.write('      <!-- ENC equivalent of 1 ADU -->\n')
f.write('      <ElectronToADU unit=\"ENC\">' + str(ElectronToADU) + '</ElectronToADU>\n')
f.write('      <!-- Maximum code for analog to digital converter (clamping) -->\n')
f.write('      <MaxADUCode>' + str(MaxADUCode) + '</MaxADUCode>\n')
f.write('      <!-- Apply Poisson smearing of electrons collected on pixels -->\n')
f.write('      <ApplyPoissonSmearing>' + str(ApplyPoissonSmearing) + '</ApplyPoissonSmearing>\n')
f.write('      <!-- Use integration window? -->\n')
f.write('      <ApplyIntegrationWindow>' + str(ApplyIntegrationWindow) + '</ApplyIntegrationWindow>\n')
f.write('      <!-- Maximum segment length (in mm) -->\n')
f.write('      <SegmentLength unit=\"mm\">' + str(0.005) + '</SegmentLength>\n')
f.write('      <!-- Split Signalpoints in smaller groups of N electrons (in e) -->\n')
f.write('      <ElectronGroupSize>' + str(ElectronGroupSize) + '</ElectronGroupSize>\n')
f.write('      <!-- Time step for tracking electron groups in readout plane (in ns) -->\n')
f.write('      <ElectronStepTime unit=\"ns\">' + str(ElectronStepTime) + '</ElectronStepTime>\n')
f.write('      <!-- Maximum number of steps when propagating electrons -->\n')
f.write('      <ElectronMaxSteps>' + str(ElectronMaxSteps) + '</ElectronMaxSteps>\n')
f.write('      <!-- Constant time delay between bunch crossing and switching on triggergate (in ns) -->\n')
f.write('      <HardwareDelay unit=\"ns\">' + str(HardwareDelay) + '</HardwareDelay>\n')
f.write('      <!-- ADC conversion factor -->\n')
f.write('      <ADCunit>' + str(ADCunit) + '</ADCunit>\n')
f.write('      <!-- Diffusion coefficient TODO add unit? is it diffusion coeff or cloud size??-->\n')
f.write('      <CloudSize>' + str(CloudSize) + '</CloudSize>\n')
f.write('      <!-- ADC gain factor -->\n')
f.write('      <Gq>' + str(Gq) + '</Gq>\n')
f.write('      <!-- Averaged sensors X0 (for perpendicular incidence) -->\n')
f.write('      <X0average>' + str(X0average) + '</X0average>\n')
f.write('      <!-- Tangent of Lorentz angle -->\n')
f.write('      <TanLorentzAngle>' + str(TanLorentzAngle) + '</TanLorentzAngle>\n')
f.write('      <!-- Spatial resolution coefficient for u axis -->\n')
f.write('      <ResolutionCoefficientU>' + str(ResolutionCoefficientU) + '</ResolutionCoefficientU>\n')
f.write('      <!-- Spatial resolution coefficient for v axis -->\n')
f.write('      <ResolutionCoefficientV>' + str(ResolutionCoefficientV) + '</ResolutionCoefficientV>\n')
f.write('      <!-- Integration time for Generic Pixel -->\n')
f.write('      <IntegrationStart unit=\"ns\">' + str(IntegrationStart) + '</IntegrationStart>\n')
f.write('      <IntegrationEnd   unit=\"ns\">' + str(IntegrationEnd) + '</IntegrationEnd>\n')
f.write('    </Active>\n')
f.write('    <!-- Used for thinning in the VXD sensitive area\n')
f.write('\n')
f.write('         now we place some components on the sensor by giving the name of the\n')
f.write('         component defined further down and the positions of all placements on\n')
f.write('         the sensor in local coordinates, starting at the lower left edge\n')
f.write('\n')
f.write('         possible placements for the w-coordinate are:\n')
f.write('          - below: place below the sensor surface\n')
f.write('          - bottom: place in the sensor but at the bottom\n')
f.write('          - center: place in the sensor center\n')
f.write('          - top: place in the sensor touching the top surface\n')
f.write('          - above place on top of the sensor surface -->\n')
f.write('\n')
f.write('     <!-- Thinning of the sensor down to the sensitive thickness by cutting out\n')
f.write('         a piece of the silicon -->\n')
f.write('    <!--Component type=\"ThinningLayer14\">\n')
f.write('      <u unit=\"mm\">' + str(unitThinU) + '</u><v unit=\"mm\">' + str(unitThinV) + '</v>\n')
f.write('    </Component-->\n')
f.write('  </Sensor>\n')

f.write('\n')

f.write('<Sensor type=\"layer13\">\n')
f.write('  <Material>' + Material + '</Material>\n')
f.write('  <width unit=\"mm\">' + str(sensorW) + '</width>\n')
f.write('  <length unit=\"mm\">' + str(sensorL*3) + '</length>\n')
f.write('  <height unit=\"mm\">' + str(sensorH) + '</height>\n')
f.write('  <!-- definition of the active area of the module which is a part of the\n')
f.write('       whole thing. The position is given relative to the lower left edge of\n')
f.write('       the module (the point with the most negative local coordinates). -->\n')
f.write('  <Active>\n')
f.write('    <u unit=\"mm\">' + str(offsetU) + '</u>\n')
f.write('    <v unit=\"mm\">' + str(offsetV) + '</v>\n')
f.write('    <w>top</w>\n')
f.write('    <width unit=\"mm\">' + str(activeW) + '</width>\n')
f.write('    <length unit=\"mm\">' + str(activeL*3) + '</length>\n')
f.write('    <height unit=\"mm\">' + str(activeH)+'</height>\n')
f.write('    <!-- define the number of pixels in rphi direction -->\n')
f.write('    <pixelsU>' + str(pixelsU) + '</pixelsU>\n')
f.write('    <pixelsV>' + str(pixelsV*3) + '</pixelsV>\n')
f.write('     <!--\n')
f.write('           Maybe all those parameters don\'t make sense for Generic Pixel\n')
f.write('\n')
f.write('           Parameters describing the physical attributes of the sensor needed\n')
f.write('           for digitization. The voltages might be time dependent and than\n')
f.write('           should be placed somewhere else -->\n')
f.write('      <BulkDoping unit=\"1/um^3\">' + str(BulkDoping) + '</BulkDoping>\n')
f.write('      <BackVoltage unit=\"V\">' + str(BackVoltage) + '</BackVoltage>\n')
f.write('      <TopVoltage unit=\"V\">' + str(TopVoltage) + '</TopVoltage>\n')
f.write('      <BorderU unit=\"um\">' + str(BorderU) + '</BorderU>\n')
f.write('      <BorderV unit=\"um\">' + str(BorderV) + '</BorderV>\n')
f.write('      <ChargeThreshold unit=\"ADU\">' + str(ChargeThreshold) + '</ChargeThreshold>\n')
f.write('      <NoiseFraction>' + str(NoiseFraction) + '</NoiseFraction>\n')
f.write('      <!-- Parameters that were in the digitizer -->\n')
f.write('      <!-- Apply electronic effects? -->\n')
f.write('      <ApplyElectronicEffects>' + ApplyElectronicEffects + '</ApplyElectronicEffects>\n')
f.write('      <!-- Noise added by the electronics, set in ENC -->\n')
f.write('      <ElectronicNoise unit=\"ENC\">' + str(ElectronicNoise) + '</ElectronicNoise>\n')
f.write('      <!-- Apply binary readout? -->\n')
f.write('      <ApplyBinaryReadout>' + ApplyBinaryReadout + '</ApplyBinaryReadout>\n')
f.write('      <!-- Binary hit treshold, set in ENC -->\n')
f.write('      <BinaryHitThreshold unit=\"ENC\">' + str(BinaryHitThreshold) + '</BinaryHitThreshold>\n')
f.write('      <!-- ENC equivalent of 1 ADU -->\n')
f.write('      <ElectronToADU unit=\"ENC\">' + str(ElectronToADU) + '</ElectronToADU>\n')
f.write('      <!-- Maximum code for analog to digital converter (clamping) -->\n')
f.write('      <MaxADUCode>' + str(MaxADUCode) + '</MaxADUCode>\n')
f.write('      <!-- Apply Poisson smearing of electrons collected on pixels -->\n')
f.write('      <ApplyPoissonSmearing>' + str(ApplyPoissonSmearing) + '</ApplyPoissonSmearing>\n')
f.write('      <!-- Use integration window? -->\n')
f.write('      <ApplyIntegrationWindow>' + str(ApplyIntegrationWindow) + '</ApplyIntegrationWindow>\n')
f.write('      <!-- Maximum segment length (in mm) -->\n')
f.write('      <SegmentLength unit=\"mm\">' + str(0.005) + '</SegmentLength>\n')
f.write('      <!-- Split Signalpoints in smaller groups of N electrons (in e) -->\n')
f.write('      <ElectronGroupSize>' + str(ElectronGroupSize) + '</ElectronGroupSize>\n')
f.write('      <!-- Time step for tracking electron groups in readout plane (in ns) -->\n')
f.write('      <ElectronStepTime unit=\"ns\">' + str(ElectronStepTime) + '</ElectronStepTime>\n')
f.write('      <!-- Maximum number of steps when propagating electrons -->\n')
f.write('      <ElectronMaxSteps>' + str(ElectronMaxSteps) + '</ElectronMaxSteps>\n')
f.write('      <!-- Constant time delay between bunch crossing and switching on triggergate (in ns) -->\n')
f.write('      <HardwareDelay unit=\"ns\">' + str(HardwareDelay) + '</HardwareDelay>\n')
f.write('      <!-- ADC conversion factor -->\n')
f.write('      <ADCunit>' + str(ADCunit) + '</ADCunit>\n')
f.write('      <!-- Diffusion coefficient TODO add unit? is it diffusion coeff or cloud size??-->\n')
f.write('      <CloudSize>' + str(CloudSize) + '</CloudSize>\n')
f.write('      <!-- ADC gain factor -->\n')
f.write('      <Gq>' + str(Gq) + '</Gq>\n')
f.write('      <!-- Averaged sensors X0 (for perpendicular incidence) -->\n')
f.write('      <X0average>' + str(X0average) + '</X0average>\n')
f.write('      <!-- Tangent of Lorentz angle -->\n')
f.write('      <TanLorentzAngle>' + str(TanLorentzAngle) + '</TanLorentzAngle>\n')
f.write('      <!-- Spatial resolution coefficient for u axis -->\n')
f.write('      <ResolutionCoefficientU>' + str(ResolutionCoefficientU) + '</ResolutionCoefficientU>\n')
f.write('      <!-- Spatial resolution coefficient for v axis -->\n')
f.write('      <ResolutionCoefficientV>' + str(ResolutionCoefficientV) + '</ResolutionCoefficientV>\n')
f.write('      <!-- Integration time for Generic Pixel -->\n')
f.write('      <IntegrationStart unit=\"ns\">' + str(IntegrationStart) + '</IntegrationStart>\n')
f.write('      <IntegrationEnd   unit=\"ns\">' + str(IntegrationEnd) + '</IntegrationEnd>\n')
f.write('    </Active>\n')
f.write('    <!-- Used for thinning in the VXD sensitive area\n')
f.write('\n')
f.write('         now we place some components on the sensor by giving the name of the\n')
f.write('         component defined further down and the positions of all placements on\n')
f.write('         the sensor in local coordinates, starting at the lower left edge\n')
f.write('\n')
f.write('         possible placements for the w-coordinate are:\n')
f.write('          - below: place below the sensor surface\n')
f.write('          - bottom: place in the sensor but at the bottom\n')
f.write('          - center: place in the sensor center\n')
f.write('          - top: place in the sensor touching the top surface\n')
f.write('          - above place on top of the sensor surface -->\n')
f.write('\n')
f.write('     <!-- Thinning of the sensor down to the sensitive thickness by cutting out\n')
f.write('         a piece of the silicon -->\n')
f.write('    <!--Component type=\"ThinningLayer_iVTX_A\">\n')
f.write('      <u unit=\"mm\">' + str(unitThinU) + '</u><v unit=\"mm\">' + str(unitThinV) + '</v>\n')
f.write('    </Component-->\n')
f.write('  </Sensor>\n')

f.write('\n')

Material = 'ColdAir'
thinW = activeW
thinL = activeL*4
thinH = 0.360
thinAngle = 0

f.write('  <Component name=\"ThinningLayer14'+'\">\n')
f.write('    <Material>' + Material + '</Material>\n')
f.write('    <Color>#5599BB</Color>\n')
f.write('    <width  unit=\"mm\">'+str(thinW)+'</width>\n')
f.write('    <length unit=\"mm\">'+str(thinL)+'</length>\n')
f.write('    <height unit=\"mm\">'+str(thinH)+'</height>\n')
f.write('    <angle  unit=\"deg\">'+str(thinAngle)+'</angle>\n')
f.write('  </Component>\n')

f.write('\n')

thinL = activeL*3

f.write('  <Component name=\"ThinningLayer13'+'\">\n')
f.write('    <Material>' + Material + '</Material>\n')
f.write('    <Color>#5599BB</Color>\n')
f.write('    <width  unit=\"mm\">'+str(thinW)+'</width>\n')
f.write('    <length unit=\"mm\">'+str(thinL)+'</length>\n')
f.write('    <height unit=\"mm\">'+str(thinH)+'</height>\n')
f.write('    <angle  unit=\"deg\">'+str(thinAngle)+'</angle>\n')
f.write('  </Component>\n')

f.write('\n')

f.write('</Components>\n')

f = open('../data/' + fileName, 'w')

DefaultMaterial = 'Air'
OnlyActiveMaterial = 'false'
ActiveStepSize = 5
OnlyPrimaryTrueHits = 'false'
ActiveChips = 'false'
SeeNeutrons = 'false'
SensitiveThreshold = 0.1
AlignmentFile = 'VTX-Alignment-CMOS5.xml'
EnvelopeFile = 'VTX-Envelope.xml'
SupportFile = 'VTX-Support.xml'
ComponentsFile = fileNameC
RadiationSensorsFile = 'VTX-RadiationSensors.xml'
shellAngleYing = 0
shellAngleYang = 180

f.write('<?xml version=\"1.0\" encoding="UTF-8\"?>\n')
f.write('<DetectorComponent name="VTX\" xmlns:xi="http://www.w3.org/2001/XInclude\">\n')
f.write('  <Creator library=\"vtx\">VTXCreator</Creator>\n')
f.write('  <Content>\n')
f.write('    <!-- Set the default Material for the Envelope and Components which do not have a Material assigned -->\n')
f.write('    <DefaultMaterial>' + DefaultMaterial + '</DefaultMaterial>\n')
f.write('    <!--If this is true, all dead Material will be ignored and only the active\n')
f.write('        Material will be placed and the rest of the Detector will be filled\n')
f.write('        with DefaultMaterial -->\n')
f.write('    <OnlyActiveMaterial>' + OnlyActiveMaterial + '</OnlyActiveMaterial>\n')
f.write('    <!-- Set the  stepsize to be used in senistive volumes -->\n')
f.write('    <ActiveStepSize unit=\"um\">' + str(ActiveStepSize) + '</ActiveStepSize>\n')
f.write('    <!-- If this is true, TrueHits will only be created for primary particles,\n')
f.write('         secondaries produced by Geant4 will not create TrueHits -->\n')
f.write('    <OnlyPrimaryTrueHits>' + OnlyPrimaryTrueHits + '</OnlyPrimaryTrueHits>\n')
f.write('    <!--Settings for background and exposure studies -->\n')
f.write('    <ActiveChips desc=\"Set to true to make chips on sensors sensitive\">' + ActiveChips + '</ActiveChips>\n')
f.write('    <SeeNeutrons desc=\"Set to true to make sensitive detectors see neutrons\">' + SeeNeutrons + '</SeeNeutrons>\n')
f.write('    <SensitiveThreshold desc=\"Minimum deposited energy per geant4 step\" unit=\"eV\">' +
        str(SensitiveThreshold) + '</SensitiveThreshold>\n')
f.write('    <!-- Include Alignment information -->\n')
f.write('    <xi:include href=\"' + AlignmentFile + '\"/>\n')
f.write('    <!-- Include definition of logical volume encapsulating the VTX -->\n')
f.write('    <!--xi:include href=\"' + EnvelopeFile + '\"/-->\n')
f.write('    <!-- Include the definition of the mechanical support -->\n')
f.write('    <!--xi:include href=\"' + SupportFile + '\"/-->\n')
f.write('    <!-- Include the definition of all the components like ladders, sensors and\n')
f.write('         chips -->\n')
f.write('    <xi:include href=\"' + ComponentsFile + '\"/>\n')
f.write('    <!-- include radiation sensor definition -->\n')
f.write('    <!--xi:include href=\"' + RadiationSensorsFile + '\"/-->\n')
f.write('\n')
f.write('    <!-- VTX is build as two half shells which will be attached to the beampipe\n')
f.write('         from the sides. We follow this scheme in the simulation geometry for\n')
f.write('         alignment purposes. Each shell consists of a number of ladders for\n')
f.write('         each layer. -->\n')
f.write('    <HalfShell name=\"Ying\">\n')
f.write('      <!-- For this shell we place the support without any rotation -->\n')
f.write('      <shellAngle unit=\"deg\">' + str(shellAngleYing) + '</shellAngle>\n')
f.write('      <!-- Define the ladders in each layer with increasing phi angle -->\n')

nlayers = 5
nLaddersYing = 0
nLaddersYang = 0
nLadders = 0

# Ying
for layer in range(1, nlayers+1):

    if layer == 1:
        nLaddersYing = 3
        nLadders = 6

    if layer == 2:
        nLaddersYing = 5
        nLadders = 10

    if layer == 3:
        nLaddersYing = 7
        nLadders = 15

    if layer == 4:
        nLaddersYing = 17
        nLadders = 33

    if layer == 5:
        nLaddersYing = 13
        nLadders = 26

    f.write('      <Layer id=\"' + str(layer) + '\">\n')
    dPhi = 360/nLadders

    for id in range(1, nLaddersYing+1):

        ang = str(dPhi*(id-1))

        angt = ang[0:6]

        cmd = '        <Ladder id=\"' + str(id) + '\"' + '><phi unit=\"deg\">' + angt + '</phi></Ladder>\n'

        f.write(cmd)

    f.write('      </Layer>\n')
f.write('    </HalfShell>\n')

# Yang
f.write('    <HalfShell name="Yang">\n')
f.write('      <!-- Place the support with an rotation of 180 degree around the z-axis\n')
f.write('           compared to form second half -->\n')
f.write('      <shellAngle unit=\"deg\">' + str(shellAngleYang) + '</shellAngle>\n')
f.write('      <!-- Define the ladders in each layer with increasing phi angle. The\n')
f.write('           ladder ids are unique per layer and thus are continued from the\n')
f.write('           first half shell -->\n')

for layer in range(1, nlayers+1):

    if layer == 1:
        nLaddersYing = 3
        nLaddersYang = 6
        nLadders = 6

    if layer == 2:
        nLaddersYing = 5
        nLaddersYang = 10
        nLadders = 10

    if layer == 3:
        nLaddersYing = 7
        nLaddersYang = 15
        nLadders = 15

    if layer == 4:
        nLaddersYing = 17
        nLaddersYang = 33
        nLadders = 33

    if layer == 5:
        nLaddersYing = 13
        nLaddersYang = 26
        nLadders = 26

    f.write('      <Layer id=\"' + str(layer) + '\">\n')
    dPhi = 360/nLadders

    for id in range(nLaddersYing+1, nLaddersYang+1):

        ang = str(dPhi*(id-1))

        angt = ang[0:6]

        cmd = '        <Ladder id=\"' + str(id) + '\"' + '><phi unit=\"deg\">' + angt + '</phi></Ladder>\n'

        f.write(cmd)
    f.write('      </Layer>\n')

f.write('    </HalfShell>\n')
f.write('  </Content>\n')
f.write('</DetectorComponent>\n')
f.close()
