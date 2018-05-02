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
        'K+:ewp pi-:ewp',  # 1
        'K_S0:ewp pi0:ewpHigh',  # 4
        'K_S0:ewp pi+:ewpHigh pi-:ewp2High',  # 6
        'K+:ewp pi-:ewp pi0:ewpHigh',  # 7
        'K+:ewp pi-:ewpHigh pi+:ewp2High pi-:ewp',  # 9
        'K_S0:ewp pi+:ewpHigh pi-:ewp2High pi0:ewpHigh',  # 12
        'K_S0:ewp pi+:ewpHigh pi-:ewp2High pi+:ewp pi-:ewp',  # 14
        'K+:ewp pi+:ewpHigh pi-:ewp2High pi+:ewp pi0:ewpHigh',  # 15
        'K_S0:ewp pi0:ewpHigh pi0:ewpHigh',  # 18
        'K+:ewp pi-:ewp pi0:ewpHigh pi0:ewpHigh',  # 19
        'K_S0:ewp pi+:ewpHigh pi-:ewp2High pi0:ewpHigh pi0:ewpHigh',  # 22
        # 'K_S0:ewp eta:ewp',  # 24
        # 'K+:ewp eta:ewp pi-:ewp',  # 25
        # 'K_S0:ewp eta:ewp pi0:ewpHigh',  # 28
        # 'K_S0:ewp eta:ewp pi+:ewp pi-:ewp',  # 30
        # 'K+:ewp eta:ewp pi-:ewp pi0:ewpHigh',  # 31
        'K+:ewp K-:ewp K_S0:ewp',  # 34
        'K+:ewp K+:ewp K-:ewp pi-:ewp',  # 35
        'K+:ewp K-:ewp K_S0:ewp pi0:ewpHigh',  # 38
    ]
    return list


def XsplusModes():
    list = [
        'K_S0:ewp pi+:ewp',  # 2
        'K+:ewp pi0:ewpHigh',  # 3
        'K+:ewp pi+:ewpHigh pi-:ewp2High',  # 5
        'K_S0:ewp pi+:ewp pi0:ewpHigh',  # 8
        'K_S0:ewp pi+:ewpHigh pi+:ewp2High pi-:ewp',  # 10
        'K+:ewp pi+:ewpHigh pi-:ewp2High pi0:ewpHigh',  # 11
        'K+:ewp pi+:ewpHigh pi-:ewp2High pi+:ewp pi-:ewp',  # 13
        'K_S0:ewp pi+:ewpHigh pi+:ewp2High pi-:ewp pi0:ewpHigh',  # 16
        'K+:ewp pi0:ewpHigh pi0:ewpHigh',  # 17
        'K_S0:ewp pi+:ewp pi0:ewpHigh pi0:ewpHigh',  # 20
        'K+:ewp pi+:ewpHigh pi-:ewp2High pi0:ewpHigh pi0:ewpHigh',  # 21
        # 'K+:ewp eta:ewp',  # 23
        # 'K_S0:ewp eta:ewp pi+:ewp',  # 26
        # 'K+:ewp eta:ewp pi0:ewpHigh',  # 27
        # 'K+:ewp eta:ewp pi+:ewp pi-:ewp',  # 29
        # 'K_S0:ewp eta:ewp pi+:ewp pi0:ewp ',  # 32
        'K+:ewp K+:ewp K-:ewp',  # 33
        'K+:ewp K-:ewp K_S0:ewp pi+:ewp',  # 36
        'K+:ewp K+:ewp K-:ewp pi0:ewpHigh',  # 37
    ]
    return list


def Xd0Modes():
    list = [
        'pi+:ewpHigh pi-:ewp2High',
        'pi+:ewpHigh pi-:ewp2High pi0:ewpHigh',
        'pi+:ewpHigh pi-:ewp2High pi0:ewpHigh pi0:ewpHigh',
    ]
    return list


def XdplusModes():
    list = [
        'pi+:ewp pi0:ewpHigh',
        'pi+:ewpHigh pi+:ewp2High pi-:ewp pi0:ewpHigh',
        # 'pi+:ewp eta:ewp',
        'pi+:ewpHigh pi+:ewp2High pi-:ewp',
    ]
    return list
