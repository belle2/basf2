#!/usr/bin/env python
# -*- coding: utf-8 -*-

##################################################################
#
# DB Importer for the BeamSpot (position, position error, size)
# from the DQM plots of the IPDQMExpressReco module
#
# usage:
# > basf2 beamSpotImporter.py -- --exp [exp number] --run [run number] --dqm [DQM root file] --verbose
#
# the DQM root file must contain the IPMonitoring folder
#
# contributors (2019): G. De Marino, G. Casarosa
##################################################################

import basf2
from basf2 import *
import ROOT
from ROOT import TVector3, TMatrixDSym, TFile, TH1F, TMath
from ROOT import Belle2
from ROOT.Belle2 import BeamSpot
from array import array
import argparse
from termcolor import colored
import sys


parser = argparse.ArgumentParser(description="BeamSpot Importer from DQM root file")
parser.add_argument('--exp', metavar='experiment', dest='exp', type=int, nargs=1, help='Experiment Number')
parser.add_argument('--run', metavar='run', dest='run', type=int, nargs=1, help='Run Number')
parser.add_argument('--dqm', metavar='dqm', dest='dqm', type=str, nargs=1, help='DQM ROOT file')
parser.add_argument('--verbose', dest='verbose', action='store_const', const=True,
                    default=False, help='print BeamSpot to screen.')

print('')

if(str(sys.argv[1]) == "help"):
    parser.print_help()
    exit(1)

args = parser.parse_args()

minVertices = 10
nSigmacut = 6

experiment = args.exp[0]
run = args.run[0]
dqmRoot = args.dqm[0]

GREEN = "\033[1;32m"
RESET = "\033[0;0m"
sys.stdout.write(GREEN)
print('|     experiment number = ' + str(experiment))
print('|first valid run number = ' + str(run))
print('|         DQM root file = ' + str(dqmRoot))
sys.stdout.write(RESET)
print('')


class beamSpotImporter(basf2.Module):

    def beginRun(self):

        iov = Belle2.IntervalOfValidity(experiment, run, -1, -1)

        dqmFile = TFile(str(dqmRoot), "read")

        if not dqmFile:
            print("file not found, check the arguments and retry.")
            return

        hVertexX = dqmFile.Get("IPMonitoring/Y4S_Vertex.X")
        hVertexX.GetXaxis().UnZoom()

        entries = hVertexX.GetEntries()

        # do not create DBObject if too few vertices
        if entries < minVertices:
            print('')
            RED = "\033[1;31m"
            sys.stdout.write(RED)
            print(' xxx WARNING! not enough vertices in this run (< ' +
                  str(minVertices) + '), the beam spot position will NOT be updated.')
            sys.stdout.write(RESET)
            print('')

            return

        hVertexY = dqmFile.Get("IPMonitoring/Y4S_Vertex.Y")
        hVertexY.GetXaxis().UnZoom()
        hVertexZ = dqmFile.Get("IPMonitoring/Y4S_Vertex.Z")
        hVertexZ.GetXaxis().UnZoom()

        hVertexXY = dqmFile.Get("IPMonitoring/Y4S_Prod.XY")
        hVertexXY.GetXaxis().UnZoom()

        hVertexYZ = dqmFile.Get("IPMonitoring/Y4S_Prod.YZ")
        hVertexYZ.GetXaxis().UnZoom()

        hVertexXZ = dqmFile.Get("IPMonitoring/Y4S_Prod.XZ")
        hVertexXZ.GetXaxis().UnZoom()

        q = array('d', [0.5])
        medianX = array('d', [0.])
        medianY = array('d', [0.])
        medianZ = array('d', [0.])
        medianXY = array('d', [0.])
        medianYZ = array('d', [0.])
        medianXZ = array('d', [0.])

        hVertexX.GetQuantiles(1, medianX, q)
        hVertexY.GetQuantiles(1, medianY, q)
        hVertexZ.GetQuantiles(1, medianZ, q)
        hVertexXY.GetQuantiles(1, medianXY, q)
        hVertexYZ.GetQuantiles(1, medianYZ, q)
        hVertexXZ.GetQuantiles(1, medianXZ, q)

        # vertex position
        # Computed as the medians of the vertex position histograms
        vertexPos = ROOT.TVector3(medianX[0], medianY[0], medianZ[0])

        # vertex position error
        # Computed as the squared RMS of the vertex position histograms, divided by the number of entries
        # Off-diagonal terms are set to zero, for the moment
        vertexCov = ROOT.TMatrixDSym(3)
        xRMS = max(hVertexX.GetRMS(), hVertexX.GetBinWidth(1) / 2)
        yRMS = max(hVertexY.GetRMS(), hVertexY.GetBinWidth(1) / 2)
        zRMS = max(hVertexZ.GetRMS(), hVertexZ.GetBinWidth(1) / 2)
        xyRMS = hVertexXY.GetRMS()
        yzRMS = hVertexYZ.GetRMS()
        xzRMS = hVertexXZ.GetRMS()

        vertexCov[0][0] = xRMS * xRMS / entries
        vertexCov[1][1] = yRMS * yRMS / entries
        vertexCov[2][2] = zRMS * zRMS / entries
        vertexCov[0][1] = 0
        vertexCov[0][2] = 0
        vertexCov[1][2] = 0
        vertexCov[1][0] = 0
        vertexCov[2][0] = 0
        vertexCov[2][1] = 0

        # beam spot size
        vertexSize = ROOT.TMatrixDSym(3)

        hVertexX.SetAxisRange(medianX[0] - nSigmacut * xRMS, medianX[0] + nSigmacut * xRMS, "X")
        hVertexY.SetAxisRange(medianY[0] - nSigmacut * yRMS, medianY[0] + nSigmacut * yRMS, "X")
        hVertexZ.SetAxisRange(medianZ[0] - nSigmacut * zRMS, medianZ[0] + nSigmacut * zRMS, "X")
        hVertexXY.SetAxisRange(medianXY[0] - nSigmacut * xyRMS, medianXY[0] + nSigmacut * xyRMS, "X")
        hVertexYZ.SetAxisRange(medianYZ[0] - nSigmacut * yzRMS, medianYZ[0] + nSigmacut * yzRMS, "X")
        hVertexXZ.SetAxisRange(medianXZ[0] - nSigmacut * xzRMS, medianYZ[0] + nSigmacut * xzRMS, "X")

        xRMScut = hVertexX.GetRMS()
        yRMScut = hVertexY.GetRMS()
        zRMScut = hVertexZ.GetRMS()
        xyRMScut = hVertexXY.GetRMS()
        yzRMScut = hVertexYZ.GetRMS()
        xzRMScut = hVertexXZ.GetRMS()

        vertexSize[0][0] = xRMScut * xRMScut
        vertexSize[1][1] = yRMScut * yRMScut
        vertexSize[2][2] = zRMScut * zRMScut
        vertexSize[0][1] = xyRMScut * xyRMScut
        vertexSize[0][2] = xzRMScut * xzRMScut
        vertexSize[1][2] = yzRMScut * yzRMScut
        vertexSize[1][0] = xyRMScut * xyRMScut
        vertexSize[2][0] = xzRMScut * xzRMScut
        vertexSize[2][1] = yzRMScut * yzRMScut

        if args.verbose:
            bBLUE = "\033[1;34m"
            BLUE = "\033[0;34m"
            sys.stdout.write(bBLUE)
            print()
            print(
                '~~ BeamSpot for Experiment = ' +
                str(experiment) +
                ', Run = ' +
                str(run) +
                ' , number of vertices = ' +
                str(entries) +
                ' ~~')
            print()
            sys.stdout.write(bBLUE)
            print('-> the vertex position (cm):')
            sys.stdout.write(RESET)

            sys.stdout.write(BLUE)
            vertexPos.Print()
            print(' Position X = {} mm '.format(round(vertexPos[0] * 10, 4)))
            print(' Position Y = {} mm '.format(round(vertexPos[1] * 10, 4)))
            print(' Position Z = {} mm '.format(round(vertexPos[2] * 10, 4)))
            print()
            sys.stdout.write(bBLUE)
            print('-> the vertex position error matrix (cm2):')
            sys.stdout.write(RESET)

            sys.stdout.write(BLUE)
            vertexCov.Print()
            print(
                ' Error X = {} um '.format(
                    round(
                        TMath.Sqrt(
                            vertexCov[0][0]) *
                        10000,
                        2),
                    1))
            print(
                ' Error Y = {} um '.format(
                    round(
                        TMath.Sqrt(
                            vertexCov[1][1]) *
                        10000,
                        2),
                    1))
            print(
                ' Error Z = {} um '.format(
                    round(
                        TMath.Sqrt(
                            vertexCov[2][2]) *
                        10000,
                        2),
                    1))
            print()
            sys.stdout.write(bBLUE)
            print('-> the luminous region size matrix (cm2):')
            sys.stdout.write(RESET)
            sys.stdout.write(BLUE)
            vertexSize.Print()
            print()

            sys.stdout.write(RESET)

        # create and fill the payload
        payload = Belle2.BeamSpot()

        payload.setVertex(vertexPos, vertexCov)
        payload.setSize(vertexSize)

        Belle2.Database.Instance().storeData("BeamSpot", payload, iov)


use_local_database("localDB_BeamSpot/database.txt", "localDB_BeamSpot")

main = create_path()

# Event info setter - execute single event
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
main.add_module(eventinfosetter)

main.add_module(beamSpotImporter())

# Process events
process(main)
