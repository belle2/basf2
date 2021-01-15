#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import basf2
import ROOT
from ROOT import Belle2
import numpy as np

from runIPVXD_CAF import get_calibration
import millepede_calibration as mpc

cal = get_calibration(dict(), [tag for tag in basf2.conditions.globaltags])
collector_file = 'CollectorOutput.root'
collector_file = mpc.collect(cal,
                             'dimuon_skim',
                             [f for f in Belle2.Environment.Instance().getInputFilesOverride()],
                             collector_file)
mpc.calibrate(cal, [collector_file])

algo = cal.algorithms[0].algorithm


# Get the payloads into handy variables
payloads = list(algo.getPayloads())
vxd = None
for payload in payloads:
    if payload.name == 'VXDAlignment':
        vxd = payload.object.IsA().DynamicCast(Belle2.VXDAlignment().IsA(), payload.object, False)


# Profile plot for all determined parameters
profile = ROOT.TH1F(
    "profile",
    "correction & errors",
    algo.result().getNoDeterminedParameters(),
    1,
    algo.result().getNoDeterminedParameters())

# Define some branch variables
param = np.zeros(1, dtype=int)
value = np.zeros(1, dtype=float)
correction = np.zeros(1, dtype=float)
error = np.zeros(1, dtype=float)
layer = np.zeros(1, dtype=int)
ladder = np.zeros(1, dtype=int)
sensor = np.zeros(1, dtype=int)
segment = np.zeros(1, dtype=int)
x = np.zeros(1, dtype=float)
y = np.zeros(1, dtype=float)
z = np.zeros(1, dtype=float)
eigenweight = np.zeros(1, dtype=float)

# Tree with VXD data
vxdtree = ROOT.TTree('vxd', 'VXD data')
vxdtree.Branch('layer', layer, 'layer/I')
vxdtree.Branch('ladder', ladder, 'ladder/I')
vxdtree.Branch('sensor', sensor, 'sensor/I')
vxdtree.Branch('segment', segment, 'segment/I')
vxdtree.Branch('param', param, 'param/I')
vxdtree.Branch('value', value, 'value/D')
vxdtree.Branch('correction', correction, 'correction/D')
vxdtree.Branch('error', error, 'error/D')
vxdtree.Branch('x', x, 'x/D')
vxdtree.Branch('y', y, 'y/D')
vxdtree.Branch('z', z, 'z/D')
vxdtree.Branch('eigenweight', eigenweight, 'eigenweight/D')

corrections = Belle2.VXDAlignment()
errors = Belle2.VXDAlignment()
eigenweights = Belle2.VXDAlignment()

# Index of determined param
ibin = 0
for ipar in range(0, algo.result().getNoParameters()):
    if not algo.result().isParameterDetermined(ipar):
        continue

    ibin = ibin + 1

    label = Belle2.GlobalLabel(algo.result().getParameterLabel(ipar))
    param[0] = label.getParameterId()
    correction[0] = algo.result().getParameterCorrection(ipar)
    error[0] = algo.result().getParameterError(ipar)

    if (label.getUniqueId() == Belle2.VXDAlignment.getGlobalUniqueID()):
        sid = label.getElementId()
        pid = label.getParameterId()
        ew = algo.result().getEigenVectorElement(0, ipar)  # + algo.result().getEigenVectorElement(1, ipar)

        if vxd:
            value[0] = vxd.get(sid, pid)
        else:
            value[0] = 0.
        corrections.set(sid, pid, correction[0])
        errors.set(sid, pid, error[0])
        eigenweights.set(sid, pid, ew)

        layer[0] = Belle2.VxdID(sid).getLayerNumber()
        ladder[0] = Belle2.VxdID(sid).getLadderNumber()
        sensor[0] = Belle2.VxdID(sid).getSensorNumber()
        segment[0] = Belle2.VxdID(sid).getSegmentNumber()

        x[0] = 0.
        y[0] = 0.
        z[0] = 0.
        eigenweight[0] = ew
        vxdtree.Fill()

    profile.SetBinContent(ibin, value[0])
    profile.SetBinError(ibin, error[0])

condition = 0

if algo.result().getNoEigenPairs():
    maxEigenValue = algo.result().getEigenNumber(algo.result().getNoEigenPairs() - 1)
    minEigenValue = algo.result().getEigenNumber(0)
    condition = maxEigenValue / minEigenValue

if condition:
    print("Condition number of the matrix: ", condition)

diagfile = ROOT.TFile('MillepedeJobDiagnostics.root', 'recreate')
diagfile.cd()
if vxd:
    vxd.Write('values')
corrections.Write('corrections')
errors.Write('errors')
eigenweights.Write('eigenweights')
vxdtree.Write('vxdtree')
profile.Write('profile')
diagfile.Close()
