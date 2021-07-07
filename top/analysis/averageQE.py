#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
import sys
from ROOT import Belle2
from ROOT import TH1F, TH2F, TFile, TProfile

# --------------------------------------------------------------------
# A tool to calculate average quantum and collection efficiency from
# Nagoya QA data stored in central database, payload TOPPmtQEs
#
# usage: basf2 averageQE.py [globalTag]
# --------------------------------------------------------------------


class AverageQE(b2.Module):
    ''' determination of average quantum and collection efficiency from TOPPmtQEs '''

    def initialize(self):
        ''' run the procedure, store the results in a root file and print in xml format '''

        dbarray = Belle2.PyDBArray('TOPPmtQEs')
        if dbarray.getEntries() == 0:
            b2.B2ERROR("No entries in TOPPmtQEs")
            return

        geo = Belle2.PyDBObj('TOPGeometry')
        if not geo:
            b2.B2ERROR("No TOPGeometry")
            return

        Bfield_on = True  # use collection efficiencies for 1.5 T

        lambdaStep = dbarray[0].getLambdaStep()
        lambdaFirst = dbarray[0].getLambdaFirst()
        lambdaLast = dbarray[0].getLambdaLast()
        n = int((lambdaLast - lambdaFirst) / lambdaStep) + 1
        print(lambdaFirst, lambdaLast, lambdaStep, n)
        if Bfield_on:
            Bfield = '(1.5 T)'
        else:
            Bfield = '(0 T)'

        pde_2d = TH2F("pde_2d", "quantum times collection efficiencies " + Bfield,
                      n, lambdaFirst - lambdaStep / 2, lambdaLast + lambdaStep / 2,
                      300, 0.0, 1.0)
        pde_prof = TProfile("pde_prof", "quantum times collection efficiencies " + Bfield,
                            n, lambdaFirst - lambdaStep / 2, lambdaLast + lambdaStep / 2,
                            0.0, 1.0)
        pde_nom = TH1F("pde_nom", "nominal quantum times collection efficiency (as found in DB)",
                       n, lambdaFirst - lambdaStep / 2, lambdaLast + lambdaStep / 2)
        ce_1d = TH1F("ce_1d", "collection efficiencies " + Bfield, 100, 0.0, 1.0)

        ce = 0
        for pmtQE in dbarray:
            ce_1d.Fill(pmtQE.getCE(Bfield_on))
            ce += pmtQE.getCE(Bfield_on)
            for pmtPixel in range(1, 17):
                for i in range(n):
                    lam = lambdaFirst + lambdaStep * i
                    effi = pmtQE.getEfficiency(pmtPixel, lam, Bfield_on)
                    pde_2d.Fill(lam, effi)
                    pde_prof.Fill(lam, effi)

        nominalQE = geo.getNominalQE()
        for i in range(n):
            lam = lambdaFirst + lambdaStep * i
            effi = nominalQE.getEfficiency(lam)
            pde_nom.SetBinContent(i+1, effi)

        ce /= dbarray.getEntries()
        print('average CE =', ce)
        file = TFile('averageQE.root', 'recreate')
        pde_2d.Write()
        pde_prof.Write()
        pde_nom.Write()
        ce_1d.Write()
        file.Close()

        print()
        if Bfield_on:
            print('<ColEffi descr="average over all PMTs for 1.5 T">' + str(round(ce, 4)) + '</ColEffi>')
        else:
            print('<ColEffi descr="average over all PMTs for 0 T">' + str(round(ce, 4)) + '</ColEffi>')
        print('<LambdaFirst unit="nm">' + str(lambdaFirst) + '</LambdaFirst>')
        print('<LambdaStep unit="nm">' + str(lambdaStep) + '</LambdaStep>')
        for i in range(n):
            qe = pde_prof.GetBinContent(i+1) / ce
            print('<Qeffi>' + str(round(qe, 3)) + '</Qeffi>')
        print()


# Central database
argvs = sys.argv
if len(argvs) == 2:
    b2.use_central_database(argvs[1])

# Create path
main = b2.create_path()

# Set number of events to generate
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1]})
main.add_module(eventinfosetter)

# Run the procedure
main.add_module(AverageQE())

# Process events
b2.process(main)
