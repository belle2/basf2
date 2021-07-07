#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This file was automatically created by FeynRules 2.0.26
# Mathematica version: 9.0 for Linux x86 (64-bit) (November 20, 2012)
# Date: Wed 23 Jul 2014 02:04:18

from object_library import all_decays, Decay
import particles as P

Decay_Ap = Decay(name='Decay_Ap', particle=P.Ap, partial_widths={
    (P.b,
     P.b__tilde__): '((12*Ge**2*MAp**2 + 24*Ge**2*MB**2)*cmath.sqrt(MAp**4 - 4*MAp**2*MB**2))/(48.*cmath.pi*abs(MAp)**3)',
    (P.c,
     P.c__tilde__): '((12*Ge**2*MAp**2 + 24*Ge**2*MC**2)*cmath.sqrt(MAp**4 - 4*MAp**2*MC**2))/(48.*cmath.pi*abs(MAp)**3)',
    (P.chi,
     P.chi__tilde__): '((4*GChi**2*MAp**2 + 8*GChi**2*MChi**2)*cmath.sqrt(MAp**4 - 4*MAp**2*MChi**2))/(48.*cmath.pi*abs(MAp)**3)',
    (P.d,
     P.d__tilde__): '((12*Ge**2*MAp**2 + 24*Ge**2*MD**2)*cmath.sqrt(MAp**4 - 4*MAp**2*MD**2))/(48.*cmath.pi*abs(MAp)**3)',
    (P.e__minus__,
     P.e__plus__): '((4*Ge**2*MAp**2 + 8*Ge**2*Me**2)*cmath.sqrt(MAp**4 - 4*MAp**2*Me**2))/(48.*cmath.pi*abs(MAp)**3)',
    (P.m__minus__,
     P.m__plus__): '((4*Ge**2*MAp**2 + 8*Ge**2*MM**2)*cmath.sqrt(MAp**4 - 4*MAp**2*MM**2))/(48.*cmath.pi*abs(MAp)**3)',
    (P.pi__minus__,
     P.pi__plus__): '((Ge**2*MAp**2 - 4*Ge**2*Mpi**2)*cmath.sqrt(MAp**4 - 4*MAp**2*Mpi**2))/(48.*cmath.pi*abs(MAp)**3)',
    (P.s,
     P.s__tilde__): '((12*Ge**2*MAp**2 + 24*Ge**2*MS**2)*cmath.sqrt(MAp**4 - 4*MAp**2*MS**2))/(48.*cmath.pi*abs(MAp)**3)',
    (P.t,
     P.t__tilde__): '((12*Ge**2*MAp**2 + 24*Ge**2*MT**2)*cmath.sqrt(MAp**4 - 4*MAp**2*MT**2))/(48.*cmath.pi*abs(MAp)**3)',
    (P.tt__minus__,
     P.tt__plus__): '((4*Ge**2*MAp**2 + 8*Ge**2*MTA**2)*cmath.sqrt(MAp**4 - 4*MAp**2*MTA**2))/(48.*cmath.pi*abs(MAp)**3)',
    (P.u,
     P.u__tilde__): '((12*Ge**2*MAp**2 + 24*Ge**2*MU**2)*cmath.sqrt(MAp**4 - 4*MAp**2*MU**2))/(48.*cmath.pi*abs(MAp)**3)',
})

Decay_b = Decay(
    name='Decay_b',
    particle=P.b,
    partial_widths={
        (
            P.W__minus__,
            P.c): '(((3*CKM2x3*ee**2*MB**2*complexconjugate(CKM2x3))/(2.*sw**2) + (3*CKM2x3*ee**2*MC**2*complexconjugate(CKM2x3))/(2.*sw**2) + (3*CKM2x3*ee**2*MB**4*complexconjugate(CKM2x3))/(2.*MW**2*sw**2) - (3*CKM2x3*ee**2*MB**2*MC**2*complexconjugate(CKM2x3))/(MW**2*sw**2) + (3*CKM2x3*ee**2*MC**4*complexconjugate(CKM2x3))/(2.*MW**2*sw**2) - (3*CKM2x3*ee**2*MW**2*complexconjugate(CKM2x3))/sw**2)*cmath.sqrt(MB**4 - 2*MB**2*MC**2 + MC**4 - 2*MB**2*MW**2 - 2*MC**2*MW**2 + MW**4))/(96.*cmath.pi*abs(MB)**3)',  # noqa
        (P.W__minus__,
         P.t): '(((3*CKM3x3*ee**2*MB**2*complexconjugate(CKM3x3))/(2.*sw**2) + (3*CKM3x3*ee**2*MT**2*complexconjugate(CKM3x3))/(2.*sw**2) + (3*CKM3x3*ee**2*MB**4*complexconjugate(CKM3x3))/(2.*MW**2*sw**2) - (3*CKM3x3*ee**2*MB**2*MT**2*complexconjugate(CKM3x3))/(MW**2*sw**2) + (3*CKM3x3*ee**2*MT**4*complexconjugate(CKM3x3))/(2.*MW**2*sw**2) - (3*CKM3x3*ee**2*MW**2*complexconjugate(CKM3x3))/sw**2)*cmath.sqrt(MB**4 - 2*MB**2*MT**2 + MT**4 - 2*MB**2*MW**2 - 2*MT**2*MW**2 + MW**4))/(96.*cmath.pi*abs(MB)**3)',  # noqa
        (P.W__minus__,
         P.u): '(((3*CKM1x3*ee**2*MB**2*complexconjugate(CKM1x3))/(2.*sw**2) + (3*CKM1x3*ee**2*MU**2*complexconjugate(CKM1x3))/(2.*sw**2) + (3*CKM1x3*ee**2*MB**4*complexconjugate(CKM1x3))/(2.*MW**2*sw**2) - (3*CKM1x3*ee**2*MB**2*MU**2*complexconjugate(CKM1x3))/(MW**2*sw**2) + (3*CKM1x3*ee**2*MU**4*complexconjugate(CKM1x3))/(2.*MW**2*sw**2) - (3*CKM1x3*ee**2*MW**2*complexconjugate(CKM1x3))/sw**2)*cmath.sqrt(MB**4 - 2*MB**2*MU**2 + MU**4 - 2*MB**2*MW**2 - 2*MU**2*MW**2 + MW**4))/(96.*cmath.pi*abs(MB)**3)'})  # noqa

Decay_c = Decay(
    name='Decay_c',
    particle=P.c,
    partial_widths={
        (
            P.W__plus__,
            P.b): '(((3*CKM2x3*ee**2*MB**2*complexconjugate(CKM2x3))/(2.*sw**2) + (3*CKM2x3*ee**2*MC**2*complexconjugate(CKM2x3))/(2.*sw**2) + (3*CKM2x3*ee**2*MB**4*complexconjugate(CKM2x3))/(2.*MW**2*sw**2) - (3*CKM2x3*ee**2*MB**2*MC**2*complexconjugate(CKM2x3))/(MW**2*sw**2) + (3*CKM2x3*ee**2*MC**4*complexconjugate(CKM2x3))/(2.*MW**2*sw**2) - (3*CKM2x3*ee**2*MW**2*complexconjugate(CKM2x3))/sw**2)*cmath.sqrt(MB**4 - 2*MB**2*MC**2 + MC**4 - 2*MB**2*MW**2 - 2*MC**2*MW**2 + MW**4))/(96.*cmath.pi*abs(MC)**3)',  # noqa
        (P.W__plus__,
         P.d): '(((3*CKM2x1*ee**2*MC**2*complexconjugate(CKM2x1))/(2.*sw**2) + (3*CKM2x1*ee**2*MD**2*complexconjugate(CKM2x1))/(2.*sw**2) + (3*CKM2x1*ee**2*MC**4*complexconjugate(CKM2x1))/(2.*MW**2*sw**2) - (3*CKM2x1*ee**2*MC**2*MD**2*complexconjugate(CKM2x1))/(MW**2*sw**2) + (3*CKM2x1*ee**2*MD**4*complexconjugate(CKM2x1))/(2.*MW**2*sw**2) - (3*CKM2x1*ee**2*MW**2*complexconjugate(CKM2x1))/sw**2)*cmath.sqrt(MC**4 - 2*MC**2*MD**2 + MD**4 - 2*MC**2*MW**2 - 2*MD**2*MW**2 + MW**4))/(96.*cmath.pi*abs(MC)**3)',  # noqa
        (P.W__plus__,
         P.s): '(((3*CKM2x2*ee**2*MC**2*complexconjugate(CKM2x2))/(2.*sw**2) + (3*CKM2x2*ee**2*MS**2*complexconjugate(CKM2x2))/(2.*sw**2) + (3*CKM2x2*ee**2*MC**4*complexconjugate(CKM2x2))/(2.*MW**2*sw**2) - (3*CKM2x2*ee**2*MC**2*MS**2*complexconjugate(CKM2x2))/(MW**2*sw**2) + (3*CKM2x2*ee**2*MS**4*complexconjugate(CKM2x2))/(2.*MW**2*sw**2) - (3*CKM2x2*ee**2*MW**2*complexconjugate(CKM2x2))/sw**2)*cmath.sqrt(MC**4 - 2*MC**2*MS**2 + MS**4 - 2*MC**2*MW**2 - 2*MS**2*MW**2 + MW**4))/(96.*cmath.pi*abs(MC)**3)'})  # noqa

Decay_d = Decay(
    name='Decay_d',
    particle=P.d,
    partial_widths={
        (
            P.W__minus__,
            P.c): '(((3*CKM2x1*ee**2*MC**2*complexconjugate(CKM2x1))/(2.*sw**2) + (3*CKM2x1*ee**2*MD**2*complexconjugate(CKM2x1))/(2.*sw**2) + (3*CKM2x1*ee**2*MC**4*complexconjugate(CKM2x1))/(2.*MW**2*sw**2) - (3*CKM2x1*ee**2*MC**2*MD**2*complexconjugate(CKM2x1))/(MW**2*sw**2) + (3*CKM2x1*ee**2*MD**4*complexconjugate(CKM2x1))/(2.*MW**2*sw**2) - (3*CKM2x1*ee**2*MW**2*complexconjugate(CKM2x1))/sw**2)*cmath.sqrt(MC**4 - 2*MC**2*MD**2 + MD**4 - 2*MC**2*MW**2 - 2*MD**2*MW**2 + MW**4))/(96.*cmath.pi*abs(MD)**3)',  # noqa
        (P.W__minus__,
         P.t): '(((3*CKM3x1*ee**2*MD**2*complexconjugate(CKM3x1))/(2.*sw**2) + (3*CKM3x1*ee**2*MT**2*complexconjugate(CKM3x1))/(2.*sw**2) + (3*CKM3x1*ee**2*MD**4*complexconjugate(CKM3x1))/(2.*MW**2*sw**2) - (3*CKM3x1*ee**2*MD**2*MT**2*complexconjugate(CKM3x1))/(MW**2*sw**2) + (3*CKM3x1*ee**2*MT**4*complexconjugate(CKM3x1))/(2.*MW**2*sw**2) - (3*CKM3x1*ee**2*MW**2*complexconjugate(CKM3x1))/sw**2)*cmath.sqrt(MD**4 - 2*MD**2*MT**2 + MT**4 - 2*MD**2*MW**2 - 2*MT**2*MW**2 + MW**4))/(96.*cmath.pi*abs(MD)**3)',  # noqa
        (P.W__minus__,
         P.u): '(((3*CKM1x1*ee**2*MD**2*complexconjugate(CKM1x1))/(2.*sw**2) + (3*CKM1x1*ee**2*MU**2*complexconjugate(CKM1x1))/(2.*sw**2) + (3*CKM1x1*ee**2*MD**4*complexconjugate(CKM1x1))/(2.*MW**2*sw**2) - (3*CKM1x1*ee**2*MD**2*MU**2*complexconjugate(CKM1x1))/(MW**2*sw**2) + (3*CKM1x1*ee**2*MU**4*complexconjugate(CKM1x1))/(2.*MW**2*sw**2) - (3*CKM1x1*ee**2*MW**2*complexconjugate(CKM1x1))/sw**2)*cmath.sqrt(MD**4 - 2*MD**2*MU**2 + MU**4 - 2*MD**2*MW**2 - 2*MU**2*MW**2 + MW**4))/(96.*cmath.pi*abs(MD)**3)'})  # noqa

Decay_e__minus__ = Decay(name='Decay_e__minus__', particle=P.e__minus__, partial_widths={(
    P.W__minus__, P.ve): '((Me**2 - MW**2)*((ee**2*Me**2)/(2.*sw**2) + (ee**2*Me**4)/(2.*MW**2*sw**2) - (ee**2*MW**2)/sw**2))/(32.*cmath.pi*abs(Me)**3)'})  # noqa

Decay_H = Decay(name='Decay_H', particle=P.H, partial_widths={
    (P.b,
     P.b__tilde__): '((-12*MB**2*yb**2 + 3*MH**2*yb**2)*cmath.sqrt(-4*MB**2*MH**2 + MH**4))/(16.*cmath.pi*abs(MH)**3)',
    (P.c,
     P.c__tilde__): '((-12*MC**2*yc**2 + 3*MH**2*yc**2)*cmath.sqrt(-4*MC**2*MH**2 + MH**4))/(16.*cmath.pi*abs(MH)**3)',
    (P.d,
     P.d__tilde__): '((-12*MD**2*ydo**2 + 3*MH**2*ydo**2)*cmath.sqrt(-4*MD**2*MH**2 + MH**4))/(16.*cmath.pi*abs(MH)**3)',
    (P.e__minus__,
     P.e__plus__): '((-4*Me**2*ye**2 + MH**2*ye**2)*cmath.sqrt(-4*Me**2*MH**2 + MH**4))/(16.*cmath.pi*abs(MH)**3)',
    (P.m__minus__,
     P.m__plus__): '((MH**2*ym**2 - 4*MM**2*ym**2)*cmath.sqrt(MH**4 - 4*MH**2*MM**2))/(16.*cmath.pi*abs(MH)**3)',
    (P.s,
     P.s__tilde__): '((3*MH**2*ys**2 - 12*MS**2*ys**2)*cmath.sqrt(MH**4 - 4*MH**2*MS**2))/(16.*cmath.pi*abs(MH)**3)',
    (P.t,
     P.t__tilde__): '((3*MH**2*yt**2 - 12*MT**2*yt**2)*cmath.sqrt(MH**4 - 4*MH**2*MT**2))/(16.*cmath.pi*abs(MH)**3)',
    (P.tt__minus__,
     P.tt__plus__): '((MH**2*ytau**2 - 4*MTA**2*ytau**2)*cmath.sqrt(MH**4 - 4*MH**2*MTA**2))/(16.*cmath.pi*abs(MH)**3)',
    (P.u,
     P.u__tilde__): '((3*MH**2*yup**2 - 12*MU**2*yup**2)*cmath.sqrt(MH**4 - 4*MH**2*MU**2))/(16.*cmath.pi*abs(MH)**3)',
    (P.W__minus__,
     P.W__plus__): '(((3*ee**4*v**2)/(4.*sw**4) + (ee**4*MH**4*v**2)/(16.*MW**4*sw**4) - (ee**4*MH**2*v**2)/(4.*MW**2*sw**4))*cmath.sqrt(MH**4 - 4*MH**2*MW**2))/(16.*cmath.pi*abs(MH)**3)',  # noqa
    (P.Z,
     P.Z): '(((9*ee**4*v**2)/2. + (3*ee**4*MH**4*v**2)/(8.*MZ**4) - (3*ee**4*MH**2*v**2)/(2.*MZ**2) + (3*cw**4*ee**4*v**2)/(4.*sw**4) + (cw**4*ee**4*MH**4*v**2)/(16.*MZ**4*sw**4) - (cw**4*ee**4*MH**2*v**2)/(4.*MZ**2*sw**4) + (3*cw**2*ee**4*v**2)/sw**2 + (cw**2*ee**4*MH**4*v**2)/(4.*MZ**4*sw**2) - (cw**2*ee**4*MH**2*v**2)/(MZ**2*sw**2) + (3*ee**4*sw**2*v**2)/cw**2 + (ee**4*MH**4*sw**2*v**2)/(4.*cw**2*MZ**4) - (ee**4*MH**2*sw**2*v**2)/(cw**2*MZ**2) + (3*ee**4*sw**4*v**2)/(4.*cw**4) + (ee**4*MH**4*sw**4*v**2)/(16.*cw**4*MZ**4) - (ee**4*MH**2*sw**4*v**2)/(4.*cw**4*MZ**2))*cmath.sqrt(MH**4 - 4*MH**2*MZ**2))/(32.*cmath.pi*abs(MH)**3)',  # noqa
})

Decay_m__minus__ = Decay(name='Decay_m__minus__', particle=P.m__minus__, partial_widths={(
    P.W__minus__, P.vm): '((MM**2 - MW**2)*((ee**2*MM**2)/(2.*sw**2) + (ee**2*MM**4)/(2.*MW**2*sw**2) - (ee**2*MW**2)/sw**2))/(32.*cmath.pi*abs(MM)**3)'})  # noqa

Decay_s = Decay(
    name='Decay_s',
    particle=P.s,
    partial_widths={
        (
            P.W__minus__,
            P.c): '(((3*CKM2x2*ee**2*MC**2*complexconjugate(CKM2x2))/(2.*sw**2) + (3*CKM2x2*ee**2*MS**2*complexconjugate(CKM2x2))/(2.*sw**2) + (3*CKM2x2*ee**2*MC**4*complexconjugate(CKM2x2))/(2.*MW**2*sw**2) - (3*CKM2x2*ee**2*MC**2*MS**2*complexconjugate(CKM2x2))/(MW**2*sw**2) + (3*CKM2x2*ee**2*MS**4*complexconjugate(CKM2x2))/(2.*MW**2*sw**2) - (3*CKM2x2*ee**2*MW**2*complexconjugate(CKM2x2))/sw**2)*cmath.sqrt(MC**4 - 2*MC**2*MS**2 + MS**4 - 2*MC**2*MW**2 - 2*MS**2*MW**2 + MW**4))/(96.*cmath.pi*abs(MS)**3)',  # noqa
        (P.W__minus__,
         P.t): '(((3*CKM3x2*ee**2*MS**2*complexconjugate(CKM3x2))/(2.*sw**2) + (3*CKM3x2*ee**2*MT**2*complexconjugate(CKM3x2))/(2.*sw**2) + (3*CKM3x2*ee**2*MS**4*complexconjugate(CKM3x2))/(2.*MW**2*sw**2) - (3*CKM3x2*ee**2*MS**2*MT**2*complexconjugate(CKM3x2))/(MW**2*sw**2) + (3*CKM3x2*ee**2*MT**4*complexconjugate(CKM3x2))/(2.*MW**2*sw**2) - (3*CKM3x2*ee**2*MW**2*complexconjugate(CKM3x2))/sw**2)*cmath.sqrt(MS**4 - 2*MS**2*MT**2 + MT**4 - 2*MS**2*MW**2 - 2*MT**2*MW**2 + MW**4))/(96.*cmath.pi*abs(MS)**3)',  # noqa
        (P.W__minus__,
         P.u): '(((3*CKM1x2*ee**2*MS**2*complexconjugate(CKM1x2))/(2.*sw**2) + (3*CKM1x2*ee**2*MU**2*complexconjugate(CKM1x2))/(2.*sw**2) + (3*CKM1x2*ee**2*MS**4*complexconjugate(CKM1x2))/(2.*MW**2*sw**2) - (3*CKM1x2*ee**2*MS**2*MU**2*complexconjugate(CKM1x2))/(MW**2*sw**2) + (3*CKM1x2*ee**2*MU**4*complexconjugate(CKM1x2))/(2.*MW**2*sw**2) - (3*CKM1x2*ee**2*MW**2*complexconjugate(CKM1x2))/sw**2)*cmath.sqrt(MS**4 - 2*MS**2*MU**2 + MU**4 - 2*MS**2*MW**2 - 2*MU**2*MW**2 + MW**4))/(96.*cmath.pi*abs(MS)**3)'})  # noqa

Decay_t = Decay(
    name='Decay_t',
    particle=P.t,
    partial_widths={
        (
            P.W__plus__,
            P.b): '(((3*CKM3x3*ee**2*MB**2*complexconjugate(CKM3x3))/(2.*sw**2) + (3*CKM3x3*ee**2*MT**2*complexconjugate(CKM3x3))/(2.*sw**2) + (3*CKM3x3*ee**2*MB**4*complexconjugate(CKM3x3))/(2.*MW**2*sw**2) - (3*CKM3x3*ee**2*MB**2*MT**2*complexconjugate(CKM3x3))/(MW**2*sw**2) + (3*CKM3x3*ee**2*MT**4*complexconjugate(CKM3x3))/(2.*MW**2*sw**2) - (3*CKM3x3*ee**2*MW**2*complexconjugate(CKM3x3))/sw**2)*cmath.sqrt(MB**4 - 2*MB**2*MT**2 + MT**4 - 2*MB**2*MW**2 - 2*MT**2*MW**2 + MW**4))/(96.*cmath.pi*abs(MT)**3)',  # noqa
        (P.W__plus__,
         P.d): '(((3*CKM3x1*ee**2*MD**2*complexconjugate(CKM3x1))/(2.*sw**2) + (3*CKM3x1*ee**2*MT**2*complexconjugate(CKM3x1))/(2.*sw**2) + (3*CKM3x1*ee**2*MD**4*complexconjugate(CKM3x1))/(2.*MW**2*sw**2) - (3*CKM3x1*ee**2*MD**2*MT**2*complexconjugate(CKM3x1))/(MW**2*sw**2) + (3*CKM3x1*ee**2*MT**4*complexconjugate(CKM3x1))/(2.*MW**2*sw**2) - (3*CKM3x1*ee**2*MW**2*complexconjugate(CKM3x1))/sw**2)*cmath.sqrt(MD**4 - 2*MD**2*MT**2 + MT**4 - 2*MD**2*MW**2 - 2*MT**2*MW**2 + MW**4))/(96.*cmath.pi*abs(MT)**3)',  # noqa
        (P.W__plus__,
         P.s): '(((3*CKM3x2*ee**2*MS**2*complexconjugate(CKM3x2))/(2.*sw**2) + (3*CKM3x2*ee**2*MT**2*complexconjugate(CKM3x2))/(2.*sw**2) + (3*CKM3x2*ee**2*MS**4*complexconjugate(CKM3x2))/(2.*MW**2*sw**2) - (3*CKM3x2*ee**2*MS**2*MT**2*complexconjugate(CKM3x2))/(MW**2*sw**2) + (3*CKM3x2*ee**2*MT**4*complexconjugate(CKM3x2))/(2.*MW**2*sw**2) - (3*CKM3x2*ee**2*MW**2*complexconjugate(CKM3x2))/sw**2)*cmath.sqrt(MS**4 - 2*MS**2*MT**2 + MT**4 - 2*MS**2*MW**2 - 2*MT**2*MW**2 + MW**4))/(96.*cmath.pi*abs(MT)**3)'})  # noqa

Decay_tt__minus__ = Decay(name='Decay_tt__minus__', particle=P.tt__minus__, partial_widths={(
    P.W__minus__, P.vt): '((MTA**2 - MW**2)*((ee**2*MTA**2)/(2.*sw**2) + (ee**2*MTA**4)/(2.*MW**2*sw**2) - (ee**2*MW**2)/sw**2))/(32.*cmath.pi*abs(MTA)**3)'})  # noqa

Decay_u = Decay(
    name='Decay_u',
    particle=P.u,
    partial_widths={
        (
            P.W__plus__,
            P.b): '(((3*CKM1x3*ee**2*MB**2*complexconjugate(CKM1x3))/(2.*sw**2) + (3*CKM1x3*ee**2*MU**2*complexconjugate(CKM1x3))/(2.*sw**2) + (3*CKM1x3*ee**2*MB**4*complexconjugate(CKM1x3))/(2.*MW**2*sw**2) - (3*CKM1x3*ee**2*MB**2*MU**2*complexconjugate(CKM1x3))/(MW**2*sw**2) + (3*CKM1x3*ee**2*MU**4*complexconjugate(CKM1x3))/(2.*MW**2*sw**2) - (3*CKM1x3*ee**2*MW**2*complexconjugate(CKM1x3))/sw**2)*cmath.sqrt(MB**4 - 2*MB**2*MU**2 + MU**4 - 2*MB**2*MW**2 - 2*MU**2*MW**2 + MW**4))/(96.*cmath.pi*abs(MU)**3)',  # noqa
        (P.W__plus__,
         P.d): '(((3*CKM1x1*ee**2*MD**2*complexconjugate(CKM1x1))/(2.*sw**2) + (3*CKM1x1*ee**2*MU**2*complexconjugate(CKM1x1))/(2.*sw**2) + (3*CKM1x1*ee**2*MD**4*complexconjugate(CKM1x1))/(2.*MW**2*sw**2) - (3*CKM1x1*ee**2*MD**2*MU**2*complexconjugate(CKM1x1))/(MW**2*sw**2) + (3*CKM1x1*ee**2*MU**4*complexconjugate(CKM1x1))/(2.*MW**2*sw**2) - (3*CKM1x1*ee**2*MW**2*complexconjugate(CKM1x1))/sw**2)*cmath.sqrt(MD**4 - 2*MD**2*MU**2 + MU**4 - 2*MD**2*MW**2 - 2*MU**2*MW**2 + MW**4))/(96.*cmath.pi*abs(MU)**3)',  # noqa
        (P.W__plus__,
         P.s): '(((3*CKM1x2*ee**2*MS**2*complexconjugate(CKM1x2))/(2.*sw**2) + (3*CKM1x2*ee**2*MU**2*complexconjugate(CKM1x2))/(2.*sw**2) + (3*CKM1x2*ee**2*MS**4*complexconjugate(CKM1x2))/(2.*MW**2*sw**2) - (3*CKM1x2*ee**2*MS**2*MU**2*complexconjugate(CKM1x2))/(MW**2*sw**2) + (3*CKM1x2*ee**2*MU**4*complexconjugate(CKM1x2))/(2.*MW**2*sw**2) - (3*CKM1x2*ee**2*MW**2*complexconjugate(CKM1x2))/sw**2)*cmath.sqrt(MS**4 - 2*MS**2*MU**2 + MU**4 - 2*MS**2*MW**2 - 2*MU**2*MW**2 + MW**4))/(96.*cmath.pi*abs(MU)**3)'})  # noqa

Decay_W__plus__ = Decay(
    name='Decay_W__plus__',
    particle=P.W__plus__,
    partial_widths={
        (
            P.c,
            P.b__tilde__): '(((-3*CKM2x3*ee**2*MB**2*complexconjugate(CKM2x3))/(2.*sw**2) - (3*CKM2x3*ee**2*MC**2*complexconjugate(CKM2x3))/(2.*sw**2) - (3*CKM2x3*ee**2*MB**4*complexconjugate(CKM2x3))/(2.*MW**2*sw**2) + (3*CKM2x3*ee**2*MB**2*MC**2*complexconjugate(CKM2x3))/(MW**2*sw**2) - (3*CKM2x3*ee**2*MC**4*complexconjugate(CKM2x3))/(2.*MW**2*sw**2) + (3*CKM2x3*ee**2*MW**2*complexconjugate(CKM2x3))/sw**2)*cmath.sqrt(MB**4 - 2*MB**2*MC**2 + MC**4 - 2*MB**2*MW**2 - 2*MC**2*MW**2 + MW**4))/(48.*cmath.pi*abs(MW)**3)',  # noqa
        (P.c,
         P.d__tilde__): '(((-3*CKM2x1*ee**2*MC**2*complexconjugate(CKM2x1))/(2.*sw**2) - (3*CKM2x1*ee**2*MD**2*complexconjugate(CKM2x1))/(2.*sw**2) - (3*CKM2x1*ee**2*MC**4*complexconjugate(CKM2x1))/(2.*MW**2*sw**2) + (3*CKM2x1*ee**2*MC**2*MD**2*complexconjugate(CKM2x1))/(MW**2*sw**2) - (3*CKM2x1*ee**2*MD**4*complexconjugate(CKM2x1))/(2.*MW**2*sw**2) + (3*CKM2x1*ee**2*MW**2*complexconjugate(CKM2x1))/sw**2)*cmath.sqrt(MC**4 - 2*MC**2*MD**2 + MD**4 - 2*MC**2*MW**2 - 2*MD**2*MW**2 + MW**4))/(48.*cmath.pi*abs(MW)**3)',  # noqa
        (P.c,
         P.s__tilde__): '(((-3*CKM2x2*ee**2*MC**2*complexconjugate(CKM2x2))/(2.*sw**2) - (3*CKM2x2*ee**2*MS**2*complexconjugate(CKM2x2))/(2.*sw**2) - (3*CKM2x2*ee**2*MC**4*complexconjugate(CKM2x2))/(2.*MW**2*sw**2) + (3*CKM2x2*ee**2*MC**2*MS**2*complexconjugate(CKM2x2))/(MW**2*sw**2) - (3*CKM2x2*ee**2*MS**4*complexconjugate(CKM2x2))/(2.*MW**2*sw**2) + (3*CKM2x2*ee**2*MW**2*complexconjugate(CKM2x2))/sw**2)*cmath.sqrt(MC**4 - 2*MC**2*MS**2 + MS**4 - 2*MC**2*MW**2 - 2*MS**2*MW**2 + MW**4))/(48.*cmath.pi*abs(MW)**3)',  # noqa
        (P.t,
         P.b__tilde__): '(((-3*CKM3x3*ee**2*MB**2*complexconjugate(CKM3x3))/(2.*sw**2) - (3*CKM3x3*ee**2*MT**2*complexconjugate(CKM3x3))/(2.*sw**2) - (3*CKM3x3*ee**2*MB**4*complexconjugate(CKM3x3))/(2.*MW**2*sw**2) + (3*CKM3x3*ee**2*MB**2*MT**2*complexconjugate(CKM3x3))/(MW**2*sw**2) - (3*CKM3x3*ee**2*MT**4*complexconjugate(CKM3x3))/(2.*MW**2*sw**2) + (3*CKM3x3*ee**2*MW**2*complexconjugate(CKM3x3))/sw**2)*cmath.sqrt(MB**4 - 2*MB**2*MT**2 + MT**4 - 2*MB**2*MW**2 - 2*MT**2*MW**2 + MW**4))/(48.*cmath.pi*abs(MW)**3)',  # noqa
        (P.t,
         P.d__tilde__): '(((-3*CKM3x1*ee**2*MD**2*complexconjugate(CKM3x1))/(2.*sw**2) - (3*CKM3x1*ee**2*MT**2*complexconjugate(CKM3x1))/(2.*sw**2) - (3*CKM3x1*ee**2*MD**4*complexconjugate(CKM3x1))/(2.*MW**2*sw**2) + (3*CKM3x1*ee**2*MD**2*MT**2*complexconjugate(CKM3x1))/(MW**2*sw**2) - (3*CKM3x1*ee**2*MT**4*complexconjugate(CKM3x1))/(2.*MW**2*sw**2) + (3*CKM3x1*ee**2*MW**2*complexconjugate(CKM3x1))/sw**2)*cmath.sqrt(MD**4 - 2*MD**2*MT**2 + MT**4 - 2*MD**2*MW**2 - 2*MT**2*MW**2 + MW**4))/(48.*cmath.pi*abs(MW)**3)',  # noqa
        (P.t,
         P.s__tilde__): '(((-3*CKM3x2*ee**2*MS**2*complexconjugate(CKM3x2))/(2.*sw**2) - (3*CKM3x2*ee**2*MT**2*complexconjugate(CKM3x2))/(2.*sw**2) - (3*CKM3x2*ee**2*MS**4*complexconjugate(CKM3x2))/(2.*MW**2*sw**2) + (3*CKM3x2*ee**2*MS**2*MT**2*complexconjugate(CKM3x2))/(MW**2*sw**2) - (3*CKM3x2*ee**2*MT**4*complexconjugate(CKM3x2))/(2.*MW**2*sw**2) + (3*CKM3x2*ee**2*MW**2*complexconjugate(CKM3x2))/sw**2)*cmath.sqrt(MS**4 - 2*MS**2*MT**2 + MT**4 - 2*MS**2*MW**2 - 2*MT**2*MW**2 + MW**4))/(48.*cmath.pi*abs(MW)**3)',  # noqa
        (P.u,
         P.b__tilde__): '(((-3*CKM1x3*ee**2*MB**2*complexconjugate(CKM1x3))/(2.*sw**2) - (3*CKM1x3*ee**2*MU**2*complexconjugate(CKM1x3))/(2.*sw**2) - (3*CKM1x3*ee**2*MB**4*complexconjugate(CKM1x3))/(2.*MW**2*sw**2) + (3*CKM1x3*ee**2*MB**2*MU**2*complexconjugate(CKM1x3))/(MW**2*sw**2) - (3*CKM1x3*ee**2*MU**4*complexconjugate(CKM1x3))/(2.*MW**2*sw**2) + (3*CKM1x3*ee**2*MW**2*complexconjugate(CKM1x3))/sw**2)*cmath.sqrt(MB**4 - 2*MB**2*MU**2 + MU**4 - 2*MB**2*MW**2 - 2*MU**2*MW**2 + MW**4))/(48.*cmath.pi*abs(MW)**3)',  # noqa
        (P.u,
         P.d__tilde__): '(((-3*CKM1x1*ee**2*MD**2*complexconjugate(CKM1x1))/(2.*sw**2) - (3*CKM1x1*ee**2*MU**2*complexconjugate(CKM1x1))/(2.*sw**2) - (3*CKM1x1*ee**2*MD**4*complexconjugate(CKM1x1))/(2.*MW**2*sw**2) + (3*CKM1x1*ee**2*MD**2*MU**2*complexconjugate(CKM1x1))/(MW**2*sw**2) - (3*CKM1x1*ee**2*MU**4*complexconjugate(CKM1x1))/(2.*MW**2*sw**2) + (3*CKM1x1*ee**2*MW**2*complexconjugate(CKM1x1))/sw**2)*cmath.sqrt(MD**4 - 2*MD**2*MU**2 + MU**4 - 2*MD**2*MW**2 - 2*MU**2*MW**2 + MW**4))/(48.*cmath.pi*abs(MW)**3)',  # noqa
        (P.u,
         P.s__tilde__): '(((-3*CKM1x2*ee**2*MS**2*complexconjugate(CKM1x2))/(2.*sw**2) - (3*CKM1x2*ee**2*MU**2*complexconjugate(CKM1x2))/(2.*sw**2) - (3*CKM1x2*ee**2*MS**4*complexconjugate(CKM1x2))/(2.*MW**2*sw**2) + (3*CKM1x2*ee**2*MS**2*MU**2*complexconjugate(CKM1x2))/(MW**2*sw**2) - (3*CKM1x2*ee**2*MU**4*complexconjugate(CKM1x2))/(2.*MW**2*sw**2) + (3*CKM1x2*ee**2*MW**2*complexconjugate(CKM1x2))/sw**2)*cmath.sqrt(MS**4 - 2*MS**2*MU**2 + MU**4 - 2*MS**2*MW**2 - 2*MU**2*MW**2 + MW**4))/(48.*cmath.pi*abs(MW)**3)',  # noqa
        (P.ve,
         P.e__plus__): '((-Me**2 + MW**2)*(-(ee**2*Me**2)/(2.*sw**2) - (ee**2*Me**4)/(2.*MW**2*sw**2) + (ee**2*MW**2)/sw**2))/(48.*cmath.pi*abs(MW)**3)',  # noqa
        (P.vm,
         P.m__plus__): '((-MM**2 + MW**2)*(-(ee**2*MM**2)/(2.*sw**2) - (ee**2*MM**4)/(2.*MW**2*sw**2) + (ee**2*MW**2)/sw**2))/(48.*cmath.pi*abs(MW)**3)',  # noqa
        (P.vt,
         P.tt__plus__): '((-MTA**2 + MW**2)*(-(ee**2*MTA**2)/(2.*sw**2) - (ee**2*MTA**4)/(2.*MW**2*sw**2) + (ee**2*MW**2)/sw**2))/(48.*cmath.pi*abs(MW)**3)',  # noqa
    })

Decay_Z = Decay(name='Decay_Z', particle=P.Z, partial_widths={
    (P.b,
     P.b__tilde__): '((-7*ee**2*MB**2 + ee**2*MZ**2 - (3*cw**2*ee**2*MB**2)/(2.*sw**2) + (3*cw**2*ee**2*MZ**2)/(2.*sw**2) - (17*ee**2*MB**2*sw**2)/(6.*cw**2) + (5*ee**2*MZ**2*sw**2)/(6.*cw**2))*cmath.sqrt(-4*MB**2*MZ**2 + MZ**4))/(48.*cmath.pi*abs(MZ)**3)',  # noqa
    (P.c,
     P.c__tilde__): '((-11*ee**2*MC**2 - ee**2*MZ**2 - (3*cw**2*ee**2*MC**2)/(2.*sw**2) + (3*cw**2*ee**2*MZ**2)/(2.*sw**2) + (7*ee**2*MC**2*sw**2)/(6.*cw**2) + (17*ee**2*MZ**2*sw**2)/(6.*cw**2))*cmath.sqrt(-4*MC**2*MZ**2 + MZ**4))/(48.*cmath.pi*abs(MZ)**3)',  # noqa
    (P.d,
     P.d__tilde__): '((-7*ee**2*MD**2 + ee**2*MZ**2 - (3*cw**2*ee**2*MD**2)/(2.*sw**2) + (3*cw**2*ee**2*MZ**2)/(2.*sw**2) - (17*ee**2*MD**2*sw**2)/(6.*cw**2) + (5*ee**2*MZ**2*sw**2)/(6.*cw**2))*cmath.sqrt(-4*MD**2*MZ**2 + MZ**4))/(48.*cmath.pi*abs(MZ)**3)',  # noqa
    (P.e__minus__,
     P.e__plus__): '((-5*ee**2*Me**2 - ee**2*MZ**2 - (cw**2*ee**2*Me**2)/(2.*sw**2) + (cw**2*ee**2*MZ**2)/(2.*sw**2) + (7*ee**2*Me**2*sw**2)/(2.*cw**2) + (5*ee**2*MZ**2*sw**2)/(2.*cw**2))*cmath.sqrt(-4*Me**2*MZ**2 + MZ**4))/(48.*cmath.pi*abs(MZ)**3)',  # noqa
    (P.m__minus__,
     P.m__plus__): '((-5*ee**2*MM**2 - ee**2*MZ**2 - (cw**2*ee**2*MM**2)/(2.*sw**2) + (cw**2*ee**2*MZ**2)/(2.*sw**2) + (7*ee**2*MM**2*sw**2)/(2.*cw**2) + (5*ee**2*MZ**2*sw**2)/(2.*cw**2))*cmath.sqrt(-4*MM**2*MZ**2 + MZ**4))/(48.*cmath.pi*abs(MZ)**3)',  # noqa
    (P.s,
     P.s__tilde__): '((-7*ee**2*MS**2 + ee**2*MZ**2 - (3*cw**2*ee**2*MS**2)/(2.*sw**2) + (3*cw**2*ee**2*MZ**2)/(2.*sw**2) - (17*ee**2*MS**2*sw**2)/(6.*cw**2) + (5*ee**2*MZ**2*sw**2)/(6.*cw**2))*cmath.sqrt(-4*MS**2*MZ**2 + MZ**4))/(48.*cmath.pi*abs(MZ)**3)',  # noqa
    (P.t,
     P.t__tilde__): '((-11*ee**2*MT**2 - ee**2*MZ**2 - (3*cw**2*ee**2*MT**2)/(2.*sw**2) + (3*cw**2*ee**2*MZ**2)/(2.*sw**2) + (7*ee**2*MT**2*sw**2)/(6.*cw**2) + (17*ee**2*MZ**2*sw**2)/(6.*cw**2))*cmath.sqrt(-4*MT**2*MZ**2 + MZ**4))/(48.*cmath.pi*abs(MZ)**3)',  # noqa
    (P.tt__minus__,
     P.tt__plus__): '((-5*ee**2*MTA**2 - ee**2*MZ**2 - (cw**2*ee**2*MTA**2)/(2.*sw**2) + (cw**2*ee**2*MZ**2)/(2.*sw**2) + (7*ee**2*MTA**2*sw**2)/(2.*cw**2) + (5*ee**2*MZ**2*sw**2)/(2.*cw**2))*cmath.sqrt(-4*MTA**2*MZ**2 + MZ**4))/(48.*cmath.pi*abs(MZ)**3)',  # noqa
    (P.u,
     P.u__tilde__): '((-11*ee**2*MU**2 - ee**2*MZ**2 - (3*cw**2*ee**2*MU**2)/(2.*sw**2) + (3*cw**2*ee**2*MZ**2)/(2.*sw**2) + (7*ee**2*MU**2*sw**2)/(6.*cw**2) + (17*ee**2*MZ**2*sw**2)/(6.*cw**2))*cmath.sqrt(-4*MU**2*MZ**2 + MZ**4))/(48.*cmath.pi*abs(MZ)**3)',  # noqa
    (P.ve,
     P.ve__tilde__): '(MZ**2*(ee**2*MZ**2 + (cw**2*ee**2*MZ**2)/(2.*sw**2) + (ee**2*MZ**2*sw**2)/(2.*cw**2)))/(48.*cmath.pi*abs(MZ)**3)',  # noqa
    (P.vm,
     P.vm__tilde__): '(MZ**2*(ee**2*MZ**2 + (cw**2*ee**2*MZ**2)/(2.*sw**2) + (ee**2*MZ**2*sw**2)/(2.*cw**2)))/(48.*cmath.pi*abs(MZ)**3)',  # noqa
    (P.vt,
     P.vt__tilde__): '(MZ**2*(ee**2*MZ**2 + (cw**2*ee**2*MZ**2)/(2.*sw**2) + (ee**2*MZ**2*sw**2)/(2.*cw**2)))/(48.*cmath.pi*abs(MZ)**3)',  # noqa
    (P.W__minus__,
     P.W__plus__): '((-12*cw**2*gw**2*MW**2 - 17*cw**2*gw**2*MZ**2 + (4*cw**2*gw**2*MZ**4)/MW**2 + (cw**2*gw**2*MZ**6)/(4.*MW**4))*cmath.sqrt(-4*MW**2*MZ**2 + MZ**4))/(48.*cmath.pi*abs(MZ)**3)',  # noqa
})
