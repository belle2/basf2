#!/usr/bin/env python
# -*- coding: utf-8 -*-

##################################################################
#
# DB Importer for the BeamSpot (position, position error, size)
# from the DQM plots of the IPDQMExpressReco module
#
# usage:
# > basf2 beamSpotImporter.py -- --exp [exp number] --run [run number] --dqm [DQM root file]
#
# the DQM root file must contain the IPMonitoring folder
#
# contributors (2019): G. De Marino, G. Casarosa
##################################################################

import basf2
from basf2 import *
import ROOT
from ROOT import TVector3, TMatrixDSym, TFile, TH1F
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

print('')

if(str(sys.argv[1]) == "help"):
    parser.print_help()
    exit(1)

args = parser.parse_args()

minVertices = 10

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

        entries = hVertexX.GetEntries()

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
        hVertexZ = dqmFile.Get("IPMonitoring/Y4S_Vertex.Z")

        q = array('d', [0.5])
        medianX = array('d', [0.])
        medianY = array('d', [0.])
        medianZ = array('d', [0.])

        hVertexX.GetQuantiles(1, medianX, q)
        hVertexY.GetQuantiles(1, medianY, q)
        hVertexZ.GetQuantiles(1, medianZ, q)

        # vertex position
        # Computed as the medians of the vertex position histograms
        vertexPos = ROOT.TVector3(medianX[0], medianY[0], medianZ[0])
        vertexPos.Print()

        # vertex position error
        # Computed as the squared RMS of the vertex position histograms, divided by the number of entries
        # Off-diagonal terms are set to zero, for the moment
        vertexCov = ROOT.TMatrixDSym(3)
        vertexCov[0][0] = hVertexX.GetRMS() * hVertexX.GetRMS() / entries
        vertexCov[1][1] = hVertexY.GetRMS() * hVertexY.GetRMS() / entries
        vertexCov[2][2] = hVertexZ.GetRMS() * hVertexZ.GetRMS() / entries
        vertexCov[0][1] = 0
        vertexCov[0][2] = 0
        vertexCov[1][2] = 0
        vertexCov[1][0] = 0
        vertexCov[2][0] = 0
        vertexCov[2][1] = 0
        vertexCov.Print()

        # beam spot size
        vertexSize = ROOT.TMatrixDSym(3)
        hVarianceX = dqmFile.Get("IPMonitoring/Var.X")
        hVarianceY = dqmFile.Get("IPMonitoring/Var.Y")
        hVarianceZ = dqmFile.Get("IPMonitoring/Var.Z")
        hVarianceXY = dqmFile.Get("IPMonitoring/Covar.XY")
        hVarianceXZ = dqmFile.Get("IPMonitoring/Covar.XZ")
        hVarianceYZ = dqmFile.Get("IPMonitoring/Covar.YZ")
        vertexSize[0][0] = 0.1
        vertexSize[1][1] = 0.2
        vertexSize[2][2] = 0.3
        vertexSize.Print()

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
