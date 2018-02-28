#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Common lists 'X{d,s}' for inclusive B --> X modes.
# EWP group.
#
# S. Cunliffe and A. Ishikawa  Feb 2018
#
######################################################


def Xs0Modes():
    list = [
        # ordered as in BN1480
        'K+:95eff pi-:95eff',  # 1
        'K_S0:ewp pi0:ewp',  # 4
        'K_S0:ewp pi+:ewpHigh pi-:ewp2High',  # 6
        'K+:95eff pi-:95eff pi0:ewp',  # 7
        'K+:95eff pi-:ewpHigh pi+:ewp2High pi-:95eff',  # 9
        'K_S0:ewp pi+:ewpHigh pi-:ewp2High pi0:ewp',  # 12
        'K_S0:ewp pi+:ewpHigh pi-:ewp2High pi+:95eff pi-:95eff',  # 14
        'K+:95eff pi+:ewpHigh pi-:ewp2High pi+:95eff pi0:ewp',  # 15
        'K_S0:ewp pi0:ewpHigh pi0:ewp',  # 18
        'K+:95eff pi-:95eff pi0:ewpHigh pi0:ewp',  # 19
        'K_S0:ewp pi+:ewpHigh pi-:ewp2High pi0:ewpHigh pi0:ewp',  # 22
        # 'K_S0:ewp eta:ewp',  # 24
        # 'K+:95eff eta:ewp pi-:95eff',  # 25
        # 'K_S0:ewp eta:ewp pi0:ewp',  # 28
        # 'K_S0:ewp eta:ewp pi+:95eff pi-:95eff',  # 30
        # 'K+:95eff eta:ewp pi-:95eff pi0:ewp',  # 31
        'K+:95eff K-:95eff K_S0:ewp',  # 34
        'K+:95eff K+:95eff K-:95eff pi-:95eff',  # 35
        'K+:95eff K-:95eff K_S0:ewp pi0:ewp',  # 38
    ]
    return list


def XsplusModes():
    list = [
        'K_S0:ewp pi+:95eff',  # 2
        'K+:95eff pi0:ewp',  # 3
        'K+:95eff pi+:ewpHigh pi-:ewp2High',  # 5
        'K_S0:ewp pi+:95eff pi0:ewp',  # 8
        'K_S0:ewp pi+:ewpHigh pi+:ewp2High pi-:95eff',  # 10
        'K+:95eff pi+:ewpHigh pi-:ewp2High pi0:ewp',  # 11
        'K+:95eff pi+:ewpHigh pi-:ewp2High pi+:95eff pi-:95eff',  # 13
        'K_S0:ewp pi+:ewpHigh pi+:ewp2High pi-:95eff pi0:ewp',  # 16
        'K+:95eff pi0:ewpHigh pi0:ewp',  # 17
        'K_S0:ewp pi+:95eff pi0:ewpHigh pi0:ewp',  # 20
        'K+:95eff pi+:ewpHigh pi-:ewp2High pi0:ewpHigh pi0:ewp',  # 21
        # 'K+:95eff eta:ewp',  # 23
        # 'K_S0:ewp eta:ewp pi+:95eff',  # 26
        # 'K+:95eff eta:ewp pi0:ewp',  # 27
        # 'K+:95eff eta:ewp pi+:95eff pi-:95eff',  # 29
        # 'K_S0:ewp eta:ewp pi+:95eff pi0:ewp ',  # 32
        'K+:95eff K+:95eff K-:95eff',  # 33
        'K+:95eff K-:95eff K_S0:ewp pi+:95eff',  # 36
        'K+:95eff K+:95eff K-:95eff pi0:ewp',  # 37
    ]
    return list


def Xd0Modes():
    list = [
        'pi+:ewpHigh pi-:ewp2High',
        'pi+:ewpHigh pi-:ewp2High pi0:ewp',
        'pi+:ewpHigh pi-:ewp2High pi0:ewpHigh pi0:ewp',
    ]
    return list


def XdplusModes():
    list = [
        'pi+:95eff pi0:ewp',
        'pi+:ewpHigh pi+:ewp2High pi-:95eff pi0:ewp',
        # 'pi+:95eff eta:ewp',
        'pi+:ewpHigh pi+:ewp2High pi-:95eff',
    ]
    return list
