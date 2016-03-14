#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This file was automatically created by FeynRules 2.0.26
# Mathematica version: 9.0 for Linux x86 (64-bit) (November 20, 2012)
# Date: Wed 23 Jul 2014 02:04:18

from object_library import all_parameters, Parameter

from function_library import complexconjugate, re, im, csc, sec, acsc, asec, \
    cot

# This is a default parameter object representing 0.
ZERO = Parameter(name='ZERO', nature='internal', type='real', value='0.0',
                 texname='0')

# User-defined parameters.
cabi = Parameter(
    name='cabi',
    nature='external',
    type='real',
    value=0.227736,
    texname='\\theta _c',
    lhablock='CKMBLOCK',
    lhacode=[1],
    )

aEWM1 = Parameter(
    name='aEWM1',
    nature='external',
    type='real',
    value=127.9,
    texname='\\text{aEWM1}',
    lhablock='SMINPUTS',
    lhacode=[1],
    )

Gf = Parameter(
    name='Gf',
    nature='external',
    type='real',
    value=0.0000116637,
    texname='G_f',
    lhablock='SMINPUTS',
    lhacode=[2],
    )

aS = Parameter(
    name='aS',
    nature='external',
    type='real',
    value=0.1184,
    texname='\\text{aS}',
    lhablock='SMINPUTS',
    lhacode=[3],
    )

ymdo = Parameter(
    name='ymdo',
    nature='external',
    type='real',
    value=0.00504,
    texname='\\text{ymdo}',
    lhablock='YUKAWA',
    lhacode=[1],
    )

ymup = Parameter(
    name='ymup',
    nature='external',
    type='real',
    value=0.0025499999999999997,
    texname='\\text{ymup}',
    lhablock='YUKAWA',
    lhacode=[2],
    )

yms = Parameter(
    name='yms',
    nature='external',
    type='real',
    value=0.101,
    texname='\\text{yms}',
    lhablock='YUKAWA',
    lhacode=[3],
    )

ymc = Parameter(
    name='ymc',
    nature='external',
    type='real',
    value=1.27,
    texname='\\text{ymc}',
    lhablock='YUKAWA',
    lhacode=[4],
    )

ymb = Parameter(
    name='ymb',
    nature='external',
    type='real',
    value=4.7,
    texname='\\text{ymb}',
    lhablock='YUKAWA',
    lhacode=[5],
    )

ymt = Parameter(
    name='ymt',
    nature='external',
    type='real',
    value=172.,
    texname='\\text{ymt}',
    lhablock='YUKAWA',
    lhacode=[6],
    )

yme = Parameter(
    name='yme',
    nature='external',
    type='real',
    value=0.0005110000000000001,
    texname='\\text{yme}',
    lhablock='YUKAWA',
    lhacode=[11],
    )

ymm = Parameter(
    name='ymm',
    nature='external',
    type='real',
    value=0.10566,
    texname='\\text{ymm}',
    lhablock='YUKAWA',
    lhacode=[13],
    )

ymtau = Parameter(
    name='ymtau',
    nature='external',
    type='real',
    value=1.777,
    texname='\\text{ymtau}',
    lhablock='YUKAWA',
    lhacode=[15],
    )

Ge = Parameter(
    name='Ge',
    nature='external',
    type='real',
    value=0.3028177,
    texname='\\text{Ge}',
    lhablock='FRBlock',
    lhacode=[1],
    )

GChi = Parameter(
    name='GChi',
    nature='external',
    type='real',
    value=0.3028177,
    texname='\\text{GChi}',
    lhablock='FRBlock',
    lhacode=[2],
    )

Me = Parameter(
    name='Me',
    nature='external',
    type='real',
    value=0.0005110000000000001,
    texname='\\text{Me}',
    lhablock='MASS',
    lhacode=[11],
    )

MM = Parameter(
    name='MM',
    nature='external',
    type='real',
    value=0.10566,
    texname='\\text{MM}',
    lhablock='MASS',
    lhacode=[13],
    )

MTA = Parameter(
    name='MTA',
    nature='external',
    type='real',
    value=1.777,
    texname='\\text{MTA}',
    lhablock='MASS',
    lhacode=[15],
    )

MU = Parameter(
    name='MU',
    nature='external',
    type='real',
    value=0.0025499999999999997,
    texname='M',
    lhablock='MASS',
    lhacode=[2],
    )

MC = Parameter(
    name='MC',
    nature='external',
    type='real',
    value=1.42,
    texname='\\text{MC}',
    lhablock='MASS',
    lhacode=[4],
    )

MT = Parameter(
    name='MT',
    nature='external',
    type='real',
    value=172,
    texname='\\text{MT}',
    lhablock='MASS',
    lhacode=[6],
    )

MD = Parameter(
    name='MD',
    nature='external',
    type='real',
    value=0.00504,
    texname='\\text{MD}',
    lhablock='MASS',
    lhacode=[1],
    )

MS = Parameter(
    name='MS',
    nature='external',
    type='real',
    value=0.101,
    texname='\\text{MS}',
    lhablock='MASS',
    lhacode=[3],
    )

MB = Parameter(
    name='MB',
    nature='external',
    type='real',
    value=4.7,
    texname='\\text{MB}',
    lhablock='MASS',
    lhacode=[5],
    )

MChi = Parameter(
    name='MChi',
    nature='external',
    type='real',
    value=1.,
    texname='\\text{MChi}',
    lhablock='MASS',
    lhacode=[9000001],
    )

MZ = Parameter(
    name='MZ',
    nature='external',
    type='real',
    value=91.1876,
    texname='\\text{MZ}',
    lhablock='MASS',
    lhacode=[23],
    )

MAp = Parameter(
    name='MAp',
    nature='external',
    type='real',
    value=0.1,
    texname='\\text{MAp}',
    lhablock='MASS',
    lhacode=[9000008],
    )

MH = Parameter(
    name='MH',
    nature='external',
    type='real',
    value=120,
    texname='\\text{MH}',
    lhablock='MASS',
    lhacode=[25],
    )

Mpi = Parameter(
    name='Mpi',
    nature='external',
    type='real',
    value=0.13957018,
    texname='\\text{Mpi}',
    lhablock='MASS',
    lhacode=[211],
    )

WT = Parameter(
    name='WT',
    nature='external',
    type='real',
    value=1.50833649,
    texname='\\text{WT}',
    lhablock='DECAY',
    lhacode=[6],
    )

WZ = Parameter(
    name='WZ',
    nature='external',
    type='real',
    value=2.4952,
    texname='\\text{WZ}',
    lhablock='DECAY',
    lhacode=[23],
    )

WW = Parameter(
    name='WW',
    nature='external',
    type='real',
    value=2.085,
    texname='\\text{WW}',
    lhablock='DECAY',
    lhacode=[24],
    )

WAp = Parameter(
    name='WAp',
    nature='external',
    type='real',
    value=1.,
    texname='\\text{WAp}',
    lhablock='DECAY',
    lhacode=[9000008],
    )

WH = Parameter(
    name='WH',
    nature='external',
    type='real',
    value=0.00575308848,
    texname='\\text{WH}',
    lhablock='DECAY',
    lhacode=[25],
    )

Lam = Parameter(name='Lam', nature='internal', type='real',
                value='MAp/(GChi*Ge)', texname='\\text{Lam}')

aEW = Parameter(name='aEW', nature='internal', type='real', value='1/aEWM1',
                texname='\\text{aEW}')

G = Parameter(name='G', nature='internal', type='real',
              value='2*cmath.sqrt(aS)*cmath.sqrt(cmath.pi)', texname='G')

CKM1x1 = Parameter(name='CKM1x1', nature='internal', type='complex',
                   value='cmath.cos(cabi)', texname='\\text{CKM1x1}')

CKM1x2 = Parameter(name='CKM1x2', nature='internal', type='complex',
                   value='cmath.sin(cabi)', texname='\\text{CKM1x2}')

CKM1x3 = Parameter(name='CKM1x3', nature='internal', type='complex', value='0'
                   , texname='\\text{CKM1x3}')

CKM2x1 = Parameter(name='CKM2x1', nature='internal', type='complex',
                   value='-cmath.sin(cabi)', texname='\\text{CKM2x1}')

CKM2x2 = Parameter(name='CKM2x2', nature='internal', type='complex',
                   value='cmath.cos(cabi)', texname='\\text{CKM2x2}')

CKM2x3 = Parameter(name='CKM2x3', nature='internal', type='complex', value='0'
                   , texname='\\text{CKM2x3}')

CKM3x1 = Parameter(name='CKM3x1', nature='internal', type='complex', value='0'
                   , texname='\\text{CKM3x1}')

CKM3x2 = Parameter(name='CKM3x2', nature='internal', type='complex', value='0'
                   , texname='\\text{CKM3x2}')

CKM3x3 = Parameter(name='CKM3x3', nature='internal', type='complex', value='1'
                   , texname='\\text{CKM3x3}')

MW = Parameter(name='MW', nature='internal', type='real',
               value='cmath.sqrt(MZ**2/2. + cmath.sqrt(MZ**4/4. - (aEW*cmath.pi*MZ**2)/(Gf*cmath.sqrt(2))))'
               , texname='M_W')

ee = Parameter(name='ee', nature='internal', type='real',
               value='2*cmath.sqrt(aEW)*cmath.sqrt(cmath.pi)', texname='e')

sw2 = Parameter(name='sw2', nature='internal', type='real',
                value='1 - MW**2/MZ**2', texname='\\text{sw2}')

cw = Parameter(name='cw', nature='internal', type='real',
               value='cmath.sqrt(1 - sw2)', texname='c_w')

sw = Parameter(name='sw', nature='internal', type='real',
               value='cmath.sqrt(sw2)', texname='s_w')

g1 = Parameter(name='g1', nature='internal', type='real', value='ee/cw',
               texname='g_1')

gw = Parameter(name='gw', nature='internal', type='real', value='ee/sw',
               texname='g_w')

v = Parameter(name='v', nature='internal', type='real', value='(2*MW*sw)/ee',
              texname='v')

lam = Parameter(name='lam', nature='internal', type='real',
                value='MH**2/(2.*v**2)', texname='\\text{lam}')

yb = Parameter(name='yb', nature='internal', type='real',
               value='(ymb*cmath.sqrt(2))/v', texname='\\text{yb}')

yc = Parameter(name='yc', nature='internal', type='real',
               value='(ymc*cmath.sqrt(2))/v', texname='\\text{yc}')

ydo = Parameter(name='ydo', nature='internal', type='real',
                value='(ymdo*cmath.sqrt(2))/v', texname='\\text{ydo}')

ye = Parameter(name='ye', nature='internal', type='real',
               value='(yme*cmath.sqrt(2))/v', texname='\\text{ye}')

ym = Parameter(name='ym', nature='internal', type='real',
               value='(ymm*cmath.sqrt(2))/v', texname='\\text{ym}')

ys = Parameter(name='ys', nature='internal', type='real',
               value='(yms*cmath.sqrt(2))/v', texname='\\text{ys}')

yt = Parameter(name='yt', nature='internal', type='real',
               value='(ymt*cmath.sqrt(2))/v', texname='\\text{yt}')

ytau = Parameter(name='ytau', nature='internal', type='real',
                 value='(ymtau*cmath.sqrt(2))/v', texname='\\text{ytau}')

yup = Parameter(name='yup', nature='internal', type='real',
                value='(ymup*cmath.sqrt(2))/v', texname='\\text{yup}')

muH = Parameter(name='muH', nature='internal', type='real',
                value='cmath.sqrt(lam*v**2)', texname='\\mu')

