#!/usr/bin/env/python3
# -*-coding: utf-8-*-

from basf2 import *
from modularAnalysis import *

# the variables that are printed out are: Mbc, deltaE, invariant mass of
# momentum of D meson, and invariant mass of D meson and  pion.

tcpvskimpath = Path()


fileList = ['../TCPV.udst.root']
inputMdstList('MC9', fileList, path=tcpvskimpath)


from variables import variables
variablesToHistogram(
    filename='TCPV_Validation.root',
    decayString='B0:all',
    variables=[
        ('Mbc',
         100,
         5.2,
         5.3),
        ('deltaE',
         100,
         -5,
         5),
        ('daughter(0,InvM)',
         100,
         0,
         1),
        ('daughter(0,p)',
         100,
         0,
         4),
        ('daughter(1,InvM)',
         100,
         1.6,
         2),
        ('daughter(1,p)',
         100,
         0,
         4)])
process(tcpvskimpath)
print(statistics)
