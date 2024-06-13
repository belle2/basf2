#!/usr/bin/env python3
# -*- coding: utf-8 -*-


import xml.etree.ElementTree as ET
import math


def indent(elem, level=0):
    i = ""

    if elem.tag == 'z' or elem.tag == 'shift' or elem.tag == 'shiftR':
        i = ""
    else:
        i = "\n" + level*'  '

    if len(elem):
        if (not elem.text or not elem.text.strip()) and (elem.tag != 'sensor'):
            elem.text = i + "  "
        elif (not elem.text or not elem.text.strip()) and (elem.tag == 'sensor'):
            elem.text = ''
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
        for elem in elem:
            indent(elem, level+1)
        if (not elem.tail or not elem.tail.strip()) and elem.tag != 'shiftR':
            elem.tail = i

    else:
        if (level and (not elem.tail or not elem.tail.strip())):
            elem.tail = i


def indentL(elem, level=0):
    i = ""
    if elem.tag == 'phi':
        i = ""
    else:
        i = "\n" + level*'  '

    if len(elem):
        if (not elem.text or not elem.text.strip()) and (elem.tag != 'Ladder'):
            elem.text = i + "  "
        elif (not elem.text or not elem.text.strip()) and (elem.tag == 'Ladder'):
            elem.text = ''
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
        for elem in elem:
            indentL(elem, level+1)
        if (not elem.tail or not elem.tail.strip()) and elem.tag != 'phi':
            elem.tail = i

    else:
        if (level and (not elem.tail or not elem.tail.strip())):
            elem.tail = i

# #################### main #####################


OptL3Mod = True

fileName = ""
fileNameC = ""

if OptL3Mod:
    fileName = 'VTX-CMOS6-staggered-L3mod-New.xml'
    fileNameC = 'VTX-Components-CMOS6-staggered-L3mod-New.xml'
else:
    fileName = 'VTX-CMOS6-staggered-New.xml'
    fileNameC = 'VTX-Components-CMOS6-staggered-New.xml'

Material = "Si"
X0Si = 0.0937
X = 0.00439
Xi = 0.001  # 0.002
sensorL = 30.168
sensorW = 18.812
sensorH = 0.400  # str(X0Si*X*1e3)
sensorHi = 0.095*2  # str(X0Si*Xi*1e3)
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

nlayer = 6
nsensor = 0
type = ""
start = 0
shift = ""
shift1 = ""
shiftR = 0.
shiftR1 = 6.5
gap = 0
shiftL = 0
radius = 0
radius1 = 0
phi = 0
phi1 = 0

root = ET.Element("Components")
root.append(ET.Comment('VTX layout with 6 layers in a windmill structure'))

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
        shiftL = 6
        radius = 22.0

    if layer == 3:
        type = 'layer14'
        nsensor = 3
        start = -111.474 + 2*sensorL
        shift = str(11.0)
        gap = 0.1
        shiftL = 11
        radius = 69.0

    if layer == 4:
        type = 'layer14'
        nsensor = 4
        start = -157.144 + 2*sensorL
        gap = 0.1
        shiftL = 0
        radius = 82.5
        radius1 = radius + shiftR1
        phi = 5.
        phi1 = 15.

    if layer == 5:
        type = 'layer12'
        nsensor = 10
        start = -207.546 + sensorL
        gap = 0.1
        shiftL = 0
        radius = 106.0
        radius1 = radius + shiftR1
        phi = 3.75
        phi1 = 11.25

    if layer == 6:
        type = 'layer14'
        nsensor = 6
        start = -249.082 + 2*sensorL
        gap = 0.1
        shiftL = 0
        radius = 133.5
        radius1 = radius + shiftR1
        phi = 3.
        phi1 = 9.

    shift = str(radius*math.sin(math.radians(phi)))
    shift1 = str(radius1*math.sin(math.radians(phi1)))

    L1 = ET.Element('Ladder')
    L1.set('layer', str(layer))
    root.append(L1)

    s1 = ET.SubElement(L1, "shift")
    s1.set('unit', 'mm')
    s1.text = str(shiftL)
    r1 = ET.SubElement(L1, "radius")
    r1.set('unit', 'mm')
    r1.text = str(radius)

    if (layer == 1):
        L1.insert(0, ET.Comment(' shift defines the shift of the sensor center along the rphi direction '))
        L1.insert(1, ET.Comment(' of the ladder to create the windmill structure '))
        L1.insert(3, ET.Comment(' radius (together with the phi angle specified in VTX.xml for each '))
        L1.insert(4, ET.Comment(' ladder) specifies the center of the local coordinate system for the '))
        L1.insert(5, ET.Comment(' ladder: x along rphi, 0 at specified phi, y going outward 0 at '))
        L1.insert(6, ET.Comment(' specified radius, z = global z, 0 at global 0 '))
        L1.insert(7, ET.Comment(''))
        L1.insert(8, ET.Comment(' The nominal radius of the inner layer is 14 mm, but this is the inner '))
        L1.insert(9, ET.Comment(' surface of the sensor and here we give the center of the sensor, so '))
        L1.insert(10, ET.Comment(' let s add half of the thickness of supporting balcony (0.2mm) '))

    for id in range(0, nsensor):

        pos = 0
        if (layer == 5):
            pos = start + (2*sensorL+gap)*id
        else:
            pos = start + (4*sensorL+gap)*id
        posZ = str(pos)

        o1 = ET.SubElement(L1, "sensor")
        o1.set('id', str(id+1))
        o1.set('type', type)
        o1.set('flipV', 'true')
        z1 = ET.SubElement(o1, "z")
        z1.set('unit', 'mm')
        z1.text = posZ[0:7]
        sf1 = ET.SubElement(o1, "shift")
        sf1.set('unit', 'mm')
        sf1.text = shift[0:6]
        sfR1 = ET.SubElement(o1, "shiftR")
        sfR1.set('unit', 'mm')
        sfR1.text = str(shiftR)

        if (id == 0 and layer == 1):
            L1.insert(12, ET.Comment('The value of Sensors is the global z position'))

    if (layer >= 4):
        for id in range(0, nsensor):
            pos = 0
            if (layer == 5):
                pos = start + (2*sensorL+gap)*id
            else:
                pos = start + (4*sensorL+gap)*id
            posZ = str(pos)

        o1 = ET.SubElement(L1, "sensor")
        o1.set('id', str(id+1))
        o1.set('type', type)
        o1.set('flipV', 'true')
        z1 = ET.SubElement(o1, "z")
        z1.set('unit', 'mm')
        z1.text = posZ[0:7]
        sf1 = ET.SubElement(o1, "shift")
        sf1.set('unit', 'mm')
        sf1.text = shift1[0:6]
        sfR1 = ET.SubElement(o1, "shiftR")
        sfR1.set('unit', 'mm')
        sfR1.text = str(shiftR1)

# sensor type layer14
S14 = ET.Element('Sensor')
S14.set('type', 'layer14')
root.append(S14)

el1 = ET.SubElement(S14, "Material")
el1.text = Material

el1 = ET.SubElement(S14, "width")
el1.set('unit', 'mm')
el1.text = str(sensorW)

el1 = ET.SubElement(S14, "length")
el1.set('unit', 'mm')
el1.text = str(sensorL*4)

el1 = ET.SubElement(S14, "height")
el1.set('unit', 'mm')
el1.text = str(sensorH)

S14.insert(5, ET.Comment(' definition of the active area of the module which is a part of the '))
S14.insert(6, ET.Comment(' whole thing. The position is given relative to the lower left edge of '))
S14.insert(7, ET.Comment(' the module (the point with the most negative local coordinates). '))

A14 = ET.SubElement(S14, "Active")
el1 = ET.SubElement(A14, "u")
el1.set('unit', 'mm')
el1.text = str(offsetU)

el1 = ET.SubElement(A14, "v")
el1.set('unit', 'mm')
el1.text = str(offsetV)

el1 = ET.SubElement(A14, "w")
el1.text = 'top'

el1 = ET.SubElement(A14, "width")
el1.set('unit', 'mm')
el1.text = str(activeW)

el1 = ET.SubElement(A14, "length")
el1.set('unit', 'mm')
el1.text = str(activeL*4)

el1 = ET.SubElement(A14, "height")
el1.set('unit', 'mm')
el1.text = str(activeH)
A14.insert(6, ET.Comment(' define the number of pixels in rphi direction '))

el1 = ET.SubElement(A14, "pixelsU")
el1.text = str(pixelsU)

el1 = ET.SubElement(A14, "pixelsU")
el1.text = str(pixelsV*4)

A14.insert(9, ET.Comment('  Maybe all those parameters don\'t make sense for Generic Pixel '))
A14.insert(10, ET.Comment('  Parameters describing the physical attributes of the sensor needed '))
A14.insert(11, ET.Comment('  for digitization. The voltages might be time dependent and than '))
A14.insert(12, ET.Comment('  should be placed somewhere else '))

el1 = ET.SubElement(A14, "BulkDoping")
el1.set('unit', '1/um^3')
el1.text = str(BulkDoping)

el1 = ET.SubElement(A14, "BackVoltage")
el1.set('unit', 'V')
el1.text = str(BackVoltage)

el1 = ET.SubElement(A14, "TopVoltage")
el1.set('unit', 'V')
el1.text = str(TopVoltage)

el1 = ET.SubElement(A14, "BorderU")
el1.set('unit', 'um')
el1.text = str(BorderU)

el1 = ET.SubElement(A14, "BorderV")
el1.set('unit', 'um')
el1.text = str(BorderV)

el1 = ET.SubElement(A14, "ChargeThreshold")
el1.set('unit', 'ADC')
el1.text = str(ChargeThreshold)

el1 = ET.SubElement(A14, "NoiseFraction")
el1.text = str(NoiseFraction)

A14.insert(20, ET.Comment(' Parameters that were in the digitizer '))
A14.insert(21, ET.Comment(' Apply electronic effects? '))

el1 = ET.SubElement(A14, "ApplyElectronicEffects")
el1.text = str(ApplyElectronicEffects)

A14.insert(23, ET.Comment(' Noise added by the electronics, set in ENC '))
el1 = ET.SubElement(A14, "ElectronicNoise")
el1.set('unit', 'ENC')
el1.text = str(ElectronicNoise)

A14.insert(25, ET.Comment(' Apply binary readout? '))
el1 = ET.SubElement(A14, "ApplyBinaryReadout")
el1.text = str(ApplyBinaryReadout)

A14.insert(27, ET.Comment(' Binary hit threshold, set in ENC '))
el1 = ET.SubElement(A14, "BinaryHitThreshold")
el1.set('unit', 'ENC')
el1.text = str(BinaryHitThreshold)

A14.insert(29, ET.Comment(' ENC equivalent of 1 ADU '))
el1 = ET.SubElement(A14, "ElectronToADU")
el1.set('unit', 'ENC')
el1.text = str(ElectronToADU)

A14.insert(31, ET.Comment(' Maximum code for analog to digital converter (clamping) '))
el1 = ET.SubElement(A14, "MaxADUCode")
el1.text = str(MaxADUCode)

A14.insert(33, ET.Comment(' Apply Poisson smearing of electrons collected on pixels '))
el1 = ET.SubElement(A14, "ApplyPoissonSmearing")
el1.text = str(ApplyPoissonSmearing)

A14.insert(35, ET.Comment(' Use integration window? '))
el1 = ET.SubElement(A14, "ApplyIntegrationWindow")
el1.text = str(ApplyIntegrationWindow)

A14.insert(37, ET.Comment(' Maximum segment length (in mm) '))
el1 = ET.SubElement(A14, "SegmentLength")
el1.set('unit', 'mm')
el1.text = str(0.005)

A14.insert(41, ET.Comment(' Split Signalpoints in smaller groups of N electrons (in e) '))
el1 = ET.SubElement(A14, "ElectronGroupSize")
el1.text = str(ElectronGroupSize)

A14.insert(43, ET.Comment(' Split Signalpoints in smaller groups of N electrons (in e) '))
el1 = ET.SubElement(A14, "ElectronStepTime")
el1.set('unit', 'ns')
el1.text = str(ElectronStepTime)

A14.insert(45, ET.Comment(' Maximum number of steps when propagating electrons '))
el1 = ET.SubElement(A14, "ElectronMaxSteps")
el1.text = str(ElectronMaxSteps)

A14.insert(47, ET.Comment(' Constant time delay between bunch crossing and switching on triggergate (in ns) '))
el1 = ET.SubElement(A14, "HardwareDelay")
el1.set('unit', 'ns')
el1.text = str(HardwareDelay)

A14.insert(49, ET.Comment(' ADC conversion factor '))
el1 = ET.SubElement(A14, "ADCunit")
el1.text = str(ADCunit)

A14.insert(51, ET.Comment(' Diffusion coefficient TODO add unit? is it diffusion coeff or cloud size? '))
el1 = ET.SubElement(A14, "CloudSize")
el1.text = str(CloudSize)

A14.insert(53, ET.Comment(' ADC gain factor '))
el1 = ET.SubElement(A14, "Gq")
el1.text = str(Gq)

A14.insert(55, ET.Comment(' Averaged sensors X0 (for perpendicular incidence) '))
el1 = ET.SubElement(A14, "X0average")
el1.text = str(X0average)

A14.insert(57, ET.Comment(' Tangent of Lorentz angle '))
el1 = ET.SubElement(A14, "TanLorentzAngle")
el1.text = str(TanLorentzAngle)

A14.insert(59, ET.Comment(' Spatial resolution coefficient for u axis '))
el1 = ET.SubElement(A14, "ResolutionCoefficientU")
el1.text = str(ResolutionCoefficientU)

A14.insert(61, ET.Comment(' Spatial resolution coefficient for v axis '))
el1 = ET.SubElement(A14, "ResolutionCoefficientV")
el1.text = str(ResolutionCoefficientV)

A14.insert(63, ET.Comment(' Integration time for Generic Pixel '))
el1 = ET.SubElement(A14, "IntegrationStart")
el1.set('unit', 'ns')
el1.text = str(IntegrationStart)

el1 = ET.SubElement(A14, "IntegrationEnd")
el1.set('unit', 'ns')
el1.text = str(IntegrationEnd)

S14.insert(9, ET.Comment(' Integration time for Generic Pixel '))
S14.insert(10, ET.Comment(' Used for thinning in the VXD sensitive area '))
S14.insert(11, ET.Comment(' now we place some components on the sensor by giving the name '))
S14.insert(12, ET.Comment(' component defined further down and the positions of all placem '))
S14.insert(13, ET.Comment(' the sensor in local coordinates, starting at the lower left ed '))
S14.insert(14, ET.Comment(' possible placements for the w-coordinate are: '))
S14.insert(15, ET.Comment('     - below: place below the sensor surface '))
S14.insert(16, ET.Comment('     - bottom: place in the sensor but at the bottom '))
S14.insert(17, ET.Comment('     - center: place in the sensor center '))
S14.insert(18, ET.Comment('     - top: place in the sensor touching the top surface '))
S14.insert(19, ET.Comment('     - above place on top of the sensor surface '))
S14.insert(20, ET.Comment(' Thinning of the sensor down to the sensitive thickness by cutting out a piece of the silicon'))

C14 = ET.SubElement(S14, "Component")
C14.set('type', 'ThinningLayer14')
el1 = ET.SubElement(C14, "u")
el1.set('unit', 'mm')
el1.text = str(unitThinU)
el1 = ET.SubElement(C14, "v")
el1.set('unit', 'mm')
el1.text = str(unitThinV)

# sensor type layer14i
S14 = ET.Element('Sensor')
S14.set('type', 'layer14i')
root.append(S14)

el1 = ET.SubElement(S14, "Material")
el1.text = Material

el1 = ET.SubElement(S14, "width")
el1.set('unit', 'mm')
el1.text = str(sensorW)

el1 = ET.SubElement(S14, "length")
el1.set('unit', 'mm')
el1.text = str(sensorL*4)

el1 = ET.SubElement(S14, "height")
el1.set('unit', 'mm')
el1.text = str(sensorHi)

S14.insert(5, ET.Comment(' definition of the active area of the module which is a part of the '))
S14.insert(6, ET.Comment(' whole thing. The position is given relative to the lower left edge of '))
S14.insert(7, ET.Comment(' the module (the point with the most negative local coordinates). '))

A14 = ET.SubElement(S14, "Active")
el1 = ET.SubElement(A14, "u")
el1.set('unit', 'mm')
el1.text = str(offsetU)

el1 = ET.SubElement(A14, "v")
el1.set('unit', 'mm')
el1.text = str(offsetV)

el1 = ET.SubElement(A14, "w")
el1.text = 'top'

el1 = ET.SubElement(A14, "width")
el1.set('unit', 'mm')
el1.text = str(activeW)

el1 = ET.SubElement(A14, "length")
el1.set('unit', 'mm')
el1.text = str(activeL*4)

el1 = ET.SubElement(A14, "height")
el1.set('unit', 'mm')
el1.text = str(activeH)
A14.insert(6, ET.Comment(' define the number of pixels in rphi direction '))

el1 = ET.SubElement(A14, "pixelsU")
el1.text = str(pixelsU)

el1 = ET.SubElement(A14, "pixelsU")
el1.text = str(pixelsV*4)

A14.insert(9, ET.Comment('  Maybe all those parameters don\'t make sense for Generic Pixel '))
A14.insert(10, ET.Comment('  Parameters describing the physical attributes of the sensor needed '))
A14.insert(11, ET.Comment('  for digitization. The voltages might be time dependent and than '))
A14.insert(12, ET.Comment('  should be placed somewhere else '))

el1 = ET.SubElement(A14, "BulkDoping")
el1.set('unit', '1/um^3')
el1.text = str(BulkDoping)

el1 = ET.SubElement(A14, "BackVoltage")
el1.set('unit', 'V')
el1.text = str(BackVoltage)

el1 = ET.SubElement(A14, "TopVoltage")
el1.set('unit', 'V')
el1.text = str(TopVoltage)

el1 = ET.SubElement(A14, "BorderU")
el1.set('unit', 'um')
el1.text = str(BorderU)

el1 = ET.SubElement(A14, "BorderV")
el1.set('unit', 'um')
el1.text = str(BorderV)

el1 = ET.SubElement(A14, "ChargeThreshold")
el1.set('unit', 'ADC')
el1.text = str(ChargeThreshold)

el1 = ET.SubElement(A14, "NoiseFraction")
el1.text = str(NoiseFraction)

A14.insert(20, ET.Comment(' Parameters that were in the digitizer '))
A14.insert(21, ET.Comment(' Apply electronic effects? '))

el1 = ET.SubElement(A14, "ApplyElectronicEffects")
el1.text = str(ApplyElectronicEffects)

A14.insert(23, ET.Comment(' Noise added by the electronics, set in ENC '))
el1 = ET.SubElement(A14, "ElectronicNoise")
el1.set('unit', 'ENC')
el1.text = str(ElectronicNoise)

A14.insert(25, ET.Comment(' Apply binary readout? '))
el1 = ET.SubElement(A14, "ApplyBinaryReadout")
el1.text = str(ApplyBinaryReadout)

A14.insert(27, ET.Comment(' Binary hit threshold, set in ENC '))
el1 = ET.SubElement(A14, "BinaryHitThreshold")
el1.set('unit', 'ENC')
el1.text = str(BinaryHitThreshold)

A14.insert(29, ET.Comment(' ENC equivalent of 1 ADU '))
el1 = ET.SubElement(A14, "ElectronToADU")
el1.set('unit', 'ENC')
el1.text = str(ElectronToADU)

A14.insert(31, ET.Comment(' Maximum code for analog to digital converter (clamping) '))
el1 = ET.SubElement(A14, "MaxADUCode")
el1.text = str(MaxADUCode)

A14.insert(33, ET.Comment(' Apply Poisson smearing of electrons collected on pixels '))
el1 = ET.SubElement(A14, "ApplyPoissonSmearing")
el1.text = str(ApplyPoissonSmearing)

A14.insert(35, ET.Comment(' Use integration window? '))
el1 = ET.SubElement(A14, "ApplyIntegrationWindow")
el1.text = str(ApplyIntegrationWindow)

A14.insert(37, ET.Comment(' Maximum segment length (in mm) '))
el1 = ET.SubElement(A14, "SegmentLength")
el1.set('unit', 'mm')
el1.text = str(0.005)

A14.insert(41, ET.Comment(' Split Signalpoints in smaller groups of N electrons (in e) '))
el1 = ET.SubElement(A14, "ElectronGroupSize")
el1.text = str(ElectronGroupSize)

A14.insert(43, ET.Comment(' Split Signalpoints in smaller groups of N electrons (in e) '))
el1 = ET.SubElement(A14, "ElectronStepTime")
el1.set('unit', 'ns')
el1.text = str(ElectronStepTime)

A14.insert(45, ET.Comment(' Maximum number of steps when propagating electrons '))
el1 = ET.SubElement(A14, "ElectronMaxSteps")
el1.text = str(ElectronMaxSteps)

A14.insert(47, ET.Comment(' Constant time delay between bunch crossing and switching on triggergate (in ns) '))
el1 = ET.SubElement(A14, "HardwareDelay")
el1.set('unit', 'ns')
el1.text = str(HardwareDelay)

A14.insert(49, ET.Comment(' ADC conversion factor '))
el1 = ET.SubElement(A14, "ADCunit")
el1.text = str(ADCunit)

A14.insert(51, ET.Comment(' Diffusion coefficient TODO add unit? is it diffusion coeff or cloud size? '))
el1 = ET.SubElement(A14, "CloudSize")
el1.text = str(CloudSize)

A14.insert(53, ET.Comment(' ADC gain factor '))
el1 = ET.SubElement(A14, "Gq")
el1.text = str(Gq)

A14.insert(55, ET.Comment(' Averaged sensors X0 (for perpendicular incidence) '))
el1 = ET.SubElement(A14, "X0average")
el1.text = str(X0average)

A14.insert(57, ET.Comment(' Tangent of Lorentz angle '))
el1 = ET.SubElement(A14, "TanLorentzAngle")
el1.text = str(TanLorentzAngle)

A14.insert(59, ET.Comment(' Spatial resolution coefficient for u axis '))
el1 = ET.SubElement(A14, "ResolutionCoefficientU")
el1.text = str(ResolutionCoefficientU)

A14.insert(61, ET.Comment(' Spatial resolution coefficient for v axis '))
el1 = ET.SubElement(A14, "ResolutionCoefficientV")
el1.text = str(ResolutionCoefficientV)

A14.insert(63, ET.Comment(' Integration time for Generic Pixel '))
el1 = ET.SubElement(A14, "IntegrationStart")
el1.set('unit', 'ns')
el1.text = str(IntegrationStart)

el1 = ET.SubElement(A14, "IntegrationEnd")
el1.set('unit', 'ns')
el1.text = str(IntegrationEnd)

S14.insert(9, ET.Comment(' Integration time for Generic Pixel '))
S14.insert(10, ET.Comment(' Used for thinning in the VXD sensitive area '))
S14.insert(11, ET.Comment(' now we place some components on the sensor by giving the name '))
S14.insert(12, ET.Comment(' component defined further down and the positions of all placem '))
S14.insert(13, ET.Comment(' the sensor in local coordinates, starting at the lower left ed '))
S14.insert(14, ET.Comment(' possible placements for the w-coordinate are: '))
S14.insert(15, ET.Comment('     - below: place below the sensor surface '))
S14.insert(16, ET.Comment('     - bottom: place in the sensor but at the bottom '))
S14.insert(17, ET.Comment('     - center: place in the sensor center '))
S14.insert(18, ET.Comment('     - top: place in the sensor touching the top surface '))
S14.insert(19, ET.Comment('     - above place on top of the sensor surface '))
S14.insert(20, ET.Comment(' Thinning of the sensor down to the sensitive thickness by cutting out a piece of the silicon'))

C14 = ET.SubElement(S14, "Component")
C14.set('type', 'ThinningLayer14i')
el1 = ET.SubElement(C14, "u")
el1.set('unit', 'mm')
el1.text = str(unitThinU)
el1 = ET.SubElement(C14, "v")
el1.set('unit', 'mm')
el1.text = str(unitThinV)


# sensor type layer12
S14 = ET.Element('Sensor')
S14.set('type', 'layer12')
root.append(S14)

el1 = ET.SubElement(S14, "Material")
el1.text = Material

el1 = ET.SubElement(S14, "width")
el1.set('unit', 'mm')
el1.text = str(sensorW)

el1 = ET.SubElement(S14, "length")
el1.set('unit', 'mm')
el1.text = str(sensorL*2)

el1 = ET.SubElement(S14, "height")
el1.set('unit', 'mm')
el1.text = str(sensorH)

S14.insert(5, ET.Comment(' definition of the active area of the module which is a part of the '))
S14.insert(6, ET.Comment(' whole thing. The position is given relative to the lower left edge of '))
S14.insert(7, ET.Comment(' the module (the point with the most negative local coordinates). '))

A14 = ET.SubElement(S14, "Active")
el1 = ET.SubElement(A14, "u")
el1.set('unit', 'mm')
el1.text = str(offsetU)

el1 = ET.SubElement(A14, "v")
el1.set('unit', 'mm')
el1.text = str(offsetV)

el1 = ET.SubElement(A14, "w")
el1.text = 'top'

el1 = ET.SubElement(A14, "width")
el1.set('unit', 'mm')
el1.text = str(activeW)

el1 = ET.SubElement(A14, "length")
el1.set('unit', 'mm')
el1.text = str(activeL*4)

el1 = ET.SubElement(A14, "height")
el1.set('unit', 'mm')
el1.text = str(activeH)
A14.insert(6, ET.Comment(' define the number of pixels in rphi direction '))

el1 = ET.SubElement(A14, "pixelsU")
el1.text = str(pixelsU)

el1 = ET.SubElement(A14, "pixelsU")
el1.text = str(pixelsV*2)

A14.insert(9, ET.Comment('  Maybe all those parameters don\'t make sense for Generic Pixel '))
A14.insert(10, ET.Comment('  Parameters describing the physical attributes of the sensor needed '))
A14.insert(11, ET.Comment('  for digitization. The voltages might be time dependent and than '))
A14.insert(12, ET.Comment('  should be placed somewhere else '))

el1 = ET.SubElement(A14, "BulkDoping")
el1.set('unit', '1/um^3')
el1.text = str(BulkDoping)

el1 = ET.SubElement(A14, "BackVoltage")
el1.set('unit', 'V')
el1.text = str(BackVoltage)

el1 = ET.SubElement(A14, "TopVoltage")
el1.set('unit', 'V')
el1.text = str(TopVoltage)

el1 = ET.SubElement(A14, "BorderU")
el1.set('unit', 'um')
el1.text = str(BorderU)

el1 = ET.SubElement(A14, "BorderV")
el1.set('unit', 'um')
el1.text = str(BorderV)

el1 = ET.SubElement(A14, "ChargeThreshold")
el1.set('unit', 'ADC')
el1.text = str(ChargeThreshold)

el1 = ET.SubElement(A14, "NoiseFraction")
el1.text = str(NoiseFraction)

A14.insert(20, ET.Comment(' Parameters that were in the digitizer '))
A14.insert(21, ET.Comment(' Apply electronic effects? '))

el1 = ET.SubElement(A14, "ApplyElectronicEffects")
el1.text = str(ApplyElectronicEffects)

A14.insert(23, ET.Comment(' Noise added by the electronics, set in ENC '))
el1 = ET.SubElement(A14, "ElectronicNoise")
el1.set('unit', 'ENC')
el1.text = str(ElectronicNoise)

A14.insert(25, ET.Comment(' Apply binary readout? '))
el1 = ET.SubElement(A14, "ApplyBinaryReadout")
el1.text = str(ApplyBinaryReadout)

A14.insert(27, ET.Comment(' Binary hit threshold, set in ENC '))
el1 = ET.SubElement(A14, "BinaryHitThreshold")
el1.set('unit', 'ENC')
el1.text = str(BinaryHitThreshold)

A14.insert(29, ET.Comment(' ENC equivalent of 1 ADU '))
el1 = ET.SubElement(A14, "ElectronToADU")
el1.set('unit', 'ENC')
el1.text = str(ElectronToADU)

A14.insert(31, ET.Comment(' Maximum code for analog to digital converter (clamping) '))
el1 = ET.SubElement(A14, "MaxADUCode")
el1.text = str(MaxADUCode)

A14.insert(33, ET.Comment(' Apply Poisson smearing of electrons collected on pixels '))
el1 = ET.SubElement(A14, "ApplyPoissonSmearing")
el1.text = str(ApplyPoissonSmearing)

A14.insert(35, ET.Comment(' Use integration window? '))
el1 = ET.SubElement(A14, "ApplyIntegrationWindow")
el1.text = str(ApplyIntegrationWindow)

A14.insert(37, ET.Comment(' Maximum segment length (in mm) '))
el1 = ET.SubElement(A14, "SegmentLength")
el1.set('unit', 'mm')
el1.text = str(0.005)

A14.insert(41, ET.Comment(' Split Signalpoints in smaller groups of N electrons (in e) '))
el1 = ET.SubElement(A14, "ElectronGroupSize")
el1.text = str(ElectronGroupSize)

A14.insert(43, ET.Comment(' Split Signalpoints in smaller groups of N electrons (in e) '))
el1 = ET.SubElement(A14, "ElectronStepTime")
el1.set('unit', 'ns')
el1.text = str(ElectronStepTime)

A14.insert(45, ET.Comment(' Maximum number of steps when propagating electrons '))
el1 = ET.SubElement(A14, "ElectronMaxSteps")
el1.text = str(ElectronMaxSteps)

A14.insert(47, ET.Comment(' Constant time delay between bunch crossing and switching on triggergate (in ns) '))
el1 = ET.SubElement(A14, "HardwareDelay")
el1.set('unit', 'ns')
el1.text = str(HardwareDelay)

A14.insert(49, ET.Comment(' ADC conversion factor '))
el1 = ET.SubElement(A14, "ADCunit")
el1.text = str(ADCunit)

A14.insert(51, ET.Comment(' Diffusion coefficient TODO add unit? is it diffusion coeff or cloud size? '))
el1 = ET.SubElement(A14, "CloudSize")
el1.text = str(CloudSize)

A14.insert(53, ET.Comment(' ADC gain factor '))
el1 = ET.SubElement(A14, "Gq")
el1.text = str(Gq)

A14.insert(55, ET.Comment(' Averaged sensors X0 (for perpendicular incidence) '))
el1 = ET.SubElement(A14, "X0average")
el1.text = str(X0average)

A14.insert(57, ET.Comment(' Tangent of Lorentz angle '))
el1 = ET.SubElement(A14, "TanLorentzAngle")
el1.text = str(TanLorentzAngle)

A14.insert(59, ET.Comment(' Spatial resolution coefficient for u axis '))
el1 = ET.SubElement(A14, "ResolutionCoefficientU")
el1.text = str(ResolutionCoefficientU)

A14.insert(61, ET.Comment(' Spatial resolution coefficient for v axis '))
el1 = ET.SubElement(A14, "ResolutionCoefficientV")
el1.text = str(ResolutionCoefficientV)

A14.insert(63, ET.Comment(' Integration time for Generic Pixel '))
el1 = ET.SubElement(A14, "IntegrationStart")
el1.set('unit', 'ns')
el1.text = str(IntegrationStart)

el1 = ET.SubElement(A14, "IntegrationEnd")
el1.set('unit', 'ns')
el1.text = str(IntegrationEnd)

S14.insert(9, ET.Comment(' Integration time for Generic Pixel '))
S14.insert(10, ET.Comment(' Used for thinning in the VXD sensitive area '))
S14.insert(11, ET.Comment(' now we place some components on the sensor by giving the name '))
S14.insert(12, ET.Comment(' component defined further down and the positions of all placem '))
S14.insert(13, ET.Comment(' the sensor in local coordinates, starting at the lower left ed '))
S14.insert(14, ET.Comment(' possible placements for the w-coordinate are: '))
S14.insert(15, ET.Comment('     - below: place below the sensor surface '))
S14.insert(16, ET.Comment('     - bottom: place in the sensor but at the bottom '))
S14.insert(17, ET.Comment('     - center: place in the sensor center '))
S14.insert(18, ET.Comment('     - top: place in the sensor touching the top surface '))
S14.insert(19, ET.Comment('     - above place on top of the sensor surface '))
S14.insert(20, ET.Comment(' Thinning of the sensor down to the sensitive thickness by cutting out a piece of the silicon'))

C14 = ET.SubElement(S14, "Component")
C14.set('type', 'ThinningLayer12')
el1 = ET.SubElement(C14, "u")
el1.set('unit', 'mm')
el1.text = str(unitThinU)
el1 = ET.SubElement(C14, "v")
el1.set('unit', 'mm')
el1.text = str(unitThinV)

Material = 'ColdAir'
thinW = activeW
thinL = activeL*4
thinH = 0.360
thinAngle = 0

# thin layer layer14
T14 = ET.Element('Component')
T14.set('name', 'ThinningLayer14')
root.append(T14)

el1 = ET.SubElement(T14, "Material")
el1.text = Material

el1 = ET.SubElement(T14, "Color")
el1.set('unit', 'mm')
el1.text = '#5599BB'

el1 = ET.SubElement(T14, "width")
el1.set('unit', 'mm')
el1.text = str(thinW)

el1 = ET.SubElement(T14, "length")
el1.set('unit', 'mm')
el1.text = str(thinL)

el1 = ET.SubElement(T14, "height")
el1.set('unit', 'mm')
el1.text = str(thinH)

el1 = ET.SubElement(T14, "angle")
el1.set('unit', 'mm')
el1.text = str(thinAngle)

# thin layer layer14
T14 = ET.Element('Component')
T14.set('name', 'ThinningLayer14i')
root.append(T14)

el1 = ET.SubElement(T14, "Material")
el1.text = Material

el1 = ET.SubElement(T14, "Color")
el1.set('unit', 'mm')
el1.text = '#5599BB'

el1 = ET.SubElement(T14, "width")
el1.set('unit', 'mm')
el1.text = str(thinW)

el1 = ET.SubElement(T14, "length")
el1.set('unit', 'mm')
el1.text = str(thinL)

el1 = ET.SubElement(T14, "height")
el1.set('unit', 'mm')
el1.text = str(thinH)

el1 = ET.SubElement(T14, "angle")
el1.set('unit', 'mm')
el1.text = str(thinAngle)

thinH = 0.360
thinL = activeL*2


# thin layer layer12
T14 = ET.Element('Component')
T14.set('name', 'ThinningLayer12')
root.append(T14)

el1 = ET.SubElement(T14, "Material")
el1.text = Material

el1 = ET.SubElement(T14, "Color")
el1.set('unit', 'mm')
el1.text = '#5599BB'

el1 = ET.SubElement(T14, "width")
el1.set('unit', 'mm')
el1.text = str(thinW)

el1 = ET.SubElement(T14, "length")
el1.set('unit', 'mm')
el1.text = str(thinL)

el1 = ET.SubElement(T14, "height")
el1.set('unit', 'mm')
el1.text = str(thinH)

el1 = ET.SubElement(T14, "angle")
el1.set('unit', 'mm')
el1.text = str(thinAngle)


indent(root)

tree = ET.ElementTree(root)

tree.write(fileNameC, encoding="UTF-8", xml_declaration=True)

######################

DefaultMaterial = 'Air'
OnlyActiveMaterial = 'false'
ActiveStepSize = 5
OnlyPrimaryTrueHits = 'false'
ActiveChips = 'false'
SeeNeutrons = 'false'
SensitiveThreshold = 0.1
AlignmentFile = 'VTX-Alignment-CMOS6.xml'
EnvelopeFile = 'VTX-Envelope.xml'
SupportFile = 'VTX-Support.xml'
ComponentsFile = fileNameC
RadiationSensorsFile = 'VTX-RadiationSensors.xml'
shellAngleYing = 0
shellAngleYang = 180

root = ET.Element("DetectorComponent")
root.set('name', 'VTX')
root.set('xmlns:xi', 'http://www.w3.org/2001/XInclude')

CR1 = ET.Element('Creator')
CR1.set('library', 'vtx')
CR1.text = 'VTXCreator'
root.append(CR1)

CO1 = ET.Element('Content')
root.append(CO1)

CO1.insert(0, ET.Comment(' Set the default Material for the Envelope and Components which do not have a Material assigned '))
el1 = ET.SubElement(CO1, "DefaultMaterial")
el1.text = str(DefaultMaterial)

CO1.insert(2, ET.Comment(' If this is true, all dead Material will be ignored and only the active '))
CO1.insert(3, ET.Comment(' Material will be placed and the rest of the Detector will be filled '))
CO1.insert(4, ET.Comment(' with DefaultMaterial '))
el1 = ET.SubElement(CO1, "OnlyActiveMaterial")
el1.text = str(OnlyActiveMaterial)

CO1.insert(6, ET.Comment(' Set the  stepsize to be used in senistive volumes '))
el1 = ET.SubElement(CO1, "ActiveStepSize")
el1.set('unit', 'um')
el1.text = str(ActiveStepSize)

CO1.insert(8, ET.Comment(' If this is true, TrueHits will only be created for primary particles, '))
CO1.insert(9, ET.Comment(' secondaries produced by Geant4 will not create TrueHits '))
el1 = ET.SubElement(CO1, "OnlyPrimaryTrueHits")
el1.text = str(OnlyPrimaryTrueHits)

CO1.insert(11, ET.Comment(' Settings for background and exposure studies '))
el1 = ET.SubElement(CO1, "ActiveChips")
el1.set('desc', 'Set to true to make chips on sensors sensitive')
el1.text = str(ActiveChips)

el1 = ET.SubElement(CO1, "SeeNeutrons")
el1.set('desc', 'Set to true to make sensitive detectors see neutrons')
el1.text = str(SeeNeutrons)

el1 = ET.SubElement(CO1, "SensitiveThreshold")
el1.set('desc', 'Minimum deposited energy per geant4 step')
el1.set('unit', 'eV')
el1.text = str(SensitiveThreshold)

CO1.insert(15, ET.Comment(' Include Alignment information '))
el1 = ET.SubElement(CO1, "xi:include")
el1.set('href', AlignmentFile)

CO1.insert(17, ET.Comment(' Include definition of logical volume encapsulating the VTX '))
el1 = ET.SubElement(CO1, "xi:include")
el1.set('href', EnvelopeFile)

CO1.insert(19, ET.Comment(' Include the definition of the mechanical support '))
el1 = ET.SubElement(CO1, "xi:include")
el1.set('href', SupportFile)

CO1.insert(21, ET.Comment(' Include the definition of all the components like ladders, sensors and chips '))
el1 = ET.SubElement(CO1, "xi:include")
el1.set('href', ComponentsFile)

CO1.insert(23, ET.Comment(' include radiation sensor definition '))
el1 = ET.SubElement(CO1, "xi:include")
el1.set('href', RadiationSensorsFile)

CO1.insert(25, ET.Comment(' VTX is build as two half shells which will be attached to the beampipe '))
CO1.insert(26, ET.Comment(' from the sides. We follow this scheme in the simulation geometry for '))
CO1.insert(27, ET.Comment(' alignment purposes. Each shell consists of a number of ladders for '))
CO1.insert(28, ET.Comment(' each layer. '))

HS1 = ET.SubElement(CO1, "HalfShell")
HS1.set('name', 'Ying')

HS1.insert(0, ET.Comment(' For this shell we place the support without any rotation '))
SA1 = ET.SubElement(HS1, "shellAngle")
SA1.set('unit', 'deg')
SA1.text = str(shellAngleYing)

HS1.insert(2, ET.Comment(' Define the ladders in each layer with increasing phi angle '))

nlayers = 6
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
        nLaddersYing = 15
        nLadders = 30

    if layer == 4:
        nLaddersYing = 9
        nLadders = 18

    if layer == 5:
        nLaddersYing = 12
        nLadders = 24

    if layer == 6:
        nLaddersYing = 15
        nLadders = 30

    LY1 = ET.SubElement(HS1, "Layer")
    LY1.set('id',  str(layer))

    dPhi = 360/nLadders

    for id in range(1, nLaddersYing+1):

        ang = str(dPhi*(id-1))

        angt = ang[0:6]

        LA1 = ET.SubElement(LY1, "Ladder")
        LA1.set('id',  str(id))

        el1 = ET.SubElement(LA1, "phi")
        el1.set('unit', 'deg')
        el1.text = angt

HS1 = ET.SubElement(CO1, "HalfShell")
HS1.set('name', 'Yang')

HS1.insert(0, ET.Comment(' Place the support with an rotation of 180 degree around the z-axis '))
HS1.insert(1, ET.Comment(' compared to form second half  '))
SA1 = ET.SubElement(HS1, "shellAngle")
SA1.set('unit', 'deg')
SA1.text = str(shellAngleYang)

HS1.insert(3, ET.Comment(' Define the ladders in each layer with increasing phi angle. The '))
HS1.insert(4, ET.Comment(' ladder ids are unique per layer and thus are continued from the '))
HS1.insert(5, ET.Comment('first half shell '))

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
        nLaddersYing = 15
        nLaddersYang = 30
        nLadders = 30

    if layer == 4:
        nLaddersYing = 9
        nLaddersYang = 18
        nLadders = 18

    if layer == 5:
        nLaddersYing = 12
        nLaddersYang = 24
        nLadders = 24

    if layer == 6:
        nLaddersYing = 15
        nLaddersYang = 30
        nLadders = 30

    LY1 = ET.SubElement(HS1, "Layer")
    LY1.set('id',  str(layer))

    dPhi = 360/nLadders

    for id in range(1, nLaddersYing+1):

        ang = str(dPhi*(id-1))

        angt = ang[0:6]

        LA1 = ET.SubElement(LY1, "Ladder")
        LA1.set('id',  str(id))

        el1 = ET.SubElement(LA1, "phi")
        el1.set('unit', 'deg')
        el1.text = angt

indentL(root)

tree = ET.ElementTree(root)

tree.write(fileName, encoding="UTF-8", xml_declaration=True)
