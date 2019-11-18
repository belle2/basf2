#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
DB Importer for the BeamSpot (position, position error, size)
from the DQM plots of the IPDQMExpressReco module

usage:
> basf2 beamSpotImporter.py -- --exp [exp number] --run [run number] --dqm [DQM root file] --verbose

the DQM root file must contain the IPMonitoring folder

contributors (2019): G. De Marino, G. Casarosa
"""

import basf2
import ROOT
from ROOT import TVector3, TMatrixDSym, TFile, TH1F, TMath
from ROOT import Belle2
from ROOT.Belle2 import BeamSpot
from array import array
import argparse
from termcolor import colored
import sys

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="BeamSpot Importer from DQM root file")
    parser.add_argument('--exp', metavar='experiment', dest='exp', type=int, help='Experiment Number')
    parser.add_argument('--run', metavar='run', dest='run', type=int, help='Run Number')
    parser.add_argument('--dqm', metavar='dqm', dest='dqm', type=str, help='DQM ROOT file')
    parser.add_argument('--verbose', dest='verbose', action='store_const', const=True,
                        default=False, help='print BeamSpot to screen.')

    print('')

    args = parser.parse_args()

    minVertices = 10
    nSigmacut = 6

    experiment = args.exp
    run = args.run
    dqmRoot = args.dqm

    GREEN = "\033[1;32m"
    RESET = "\033[0;0m"
    sys.stdout.write(GREEN)
    print('|     experiment number = ' + str(experiment))
    print('|first valid run number = ' + str(run))
    print('|         DQM root file = ' + str(dqmRoot))
    sys.stdout.write(RESET)
    print('')

    class beamSpotImporter(basf2.Module):
        """
        Fill the BeamSpot payload and generate a local database with the beam spot parameters (position, position error, size)
        retrieving the histograms of IPMonitoring folder from DQM plots produced by the IPDQMExpressRecoModule
        """

        def beginRun(self):
            """
            For a given input histogram the payload is revised only if the histogram
            has a number of entries (aka number of reconstructed vertices) at least equal to minVertices
            """
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

            hVarX = dqmFile.Get("IPMonitoring/Var.X")
            hVarX.GetXaxis().UnZoom()
            hVarY = dqmFile.Get("IPMonitoring/Var.Y")
            hVarY.GetXaxis().UnZoom()
            hVarZ = dqmFile.Get("IPMonitoring/Var.Z")
            hVarZ.GetXaxis().UnZoom()

            # 1. vertex position
            # Computed as the medians of the vertex position histograms

            q = array('d', [0.5])
            medianX = array('d', [0.])
            medianY = array('d', [0.])
            medianZ = array('d', [0.])

            hVertexX.GetQuantiles(1, medianX, q)
            hVertexY.GetQuantiles(1, medianY, q)
            hVertexZ.GetQuantiles(1, medianZ, q)

            vertexPos = ROOT.TVector3(medianX[0], medianY[0], medianZ[0])

            # 2.  Beam spot size & covariance matrix
            # Computed from the histograms of the coordinates,
            # centered on their medians and ranging of +/- nSigmacut sigmas from the medians
            # NOTE: off-diagonal terms set to 0

            xRMS = hVertexX.GetRMS()
            yRMS = hVertexY.GetRMS()
            zRMS = hVertexZ.GetRMS()

            vertexSize = ROOT.TMatrixDSym(3)

            hVertexX.SetAxisRange(medianX[0] - nSigmacut * xRMS, medianX[0] + nSigmacut * xRMS, "X")
            hVertexY.SetAxisRange(medianY[0] - nSigmacut * yRMS, medianY[0] + nSigmacut * yRMS, "X")
            hVertexZ.SetAxisRange(medianZ[0] - nSigmacut * zRMS, medianZ[0] + nSigmacut * zRMS, "X")

            # 2.a vertex position covariance matrix
            # As first estimate, just the RMS of vertex distribution divided by the number of vertices

            vertexCov = ROOT.TMatrixDSym(3)
            vertexCov[0][1] = vertexCov[1][0] = 0
            vertexCov[0][2] = vertexCov[2][0] = 0
            vertexCov[1][2] = vertexCov[2][1] = 0
            vertexCov[0][0] = hVertexX.GetRMS() * hVertexX.GetRMS() / entries
            vertexCov[1][1] = hVertexY.GetRMS() * hVertexY.GetRMS() / entries
            vertexCov[2][2] = hVertexZ.GetRMS() * hVertexZ.GetRMS() / entries

            # 2.b beamSpot size (squared)
            # As first estimate, just the RMS of vertex distribution corrected by the error of the vertex fit

            vertexSize[0][1] = vertexSize[1][0] = 0
            vertexSize[0][2] = vertexSize[2][0] = 0
            vertexSize[1][2] = vertexSize[2][1] = 0
            vertexSize[0][0] = hVertexX.GetRMS() * hVertexX.GetRMS() - hVarX.GetMean()
            vertexSize[1][1] = hVertexY.GetRMS() * hVertexY.GetRMS() - hVarY.GetMean()
            vertexSize[2][2] = hVertexZ.GetRMS() * hVertexZ.GetRMS() - hVarZ.GetMean()

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
                print(
                    ' Size X = {} um '.format(
                        round(
                            TMath.Sqrt(
                                vertexSize[0][0]) *
                            10000,
                            2),
                        1))
                print(
                    ' Size Y = {} um '.format(
                        round(
                            TMath.Sqrt(
                                vertexSize[1][1]) *
                            10000,
                            2),
                        1))
                print(
                    ' Size Z = {} um '.format(
                        round(
                            TMath.Sqrt(
                                vertexSize[2][2]) *
                            10000,
                            2),
                        1))
                print()

                sys.stdout.write(RESET)

            # create and fill the payload
            payload = Belle2.BeamSpot()

            payload.setIP(vertexPos, vertexCov)
            payload.setSizeCovMatrix(vertexSize)

            Belle2.Database.Instance().storeData("BeamSpot", payload, iov)

    basf2.use_local_database("localDB_BeamSpot/database.txt", "localDB_BeamSpot")

    main = basf2.create_path()

    # Event info setter - execute single event
    eventinfosetter = basf2.register_module('EventInfoSetter')
    eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
    main.add_module(eventinfosetter)

    main.add_module(beamSpotImporter())

    # Process events
    basf2.process(main)
