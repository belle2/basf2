#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Common lists 'X{d,s}' for inclusive B --> X modes.
# EWP group.
#
# S. Cunliffe  Jan 2018
#
######################################################


def Xs0Modes():
    list = ['K+:loose pi-:loose',  # 1
            'K+:loose pi-:loose pi+:loose pi-:loose',  # 2
            'K+:loose pi-:loose pi0:ewp',  # 3
            'K+:loose pi-:loose pi0:ewp pi0:ewp',  # 4
            'K_S0:all pi+:loose pi-:loose',  # 5
            'K_S0:all pi+:loose pi-:loose pi+:loose pi-:loose',  # 6
            'K_S0:all pi0:ewp',  # 7
            'K_S0:all pi+:loose pi-:loose pi0:ewp',  # 8
            'K_S0:all pi0:ewp pi0:ewp',  # 9
            'K_S0:all pi+:loose pi-:loose pi0:ewp pi0:ewp',  # 10
            'K+:loose pi-:loose eta:loose',  # 11
            'K_S0:all eta:loose',  # 12
            'K_S0:all eta:loose pi+:loose pi-:loose',  # 13
            'K_S0:all eta:loose pi0:ewp',  # 14
            'K+:loose K-:loose K_S0:all',  # 15
            'K+:loose K-:loose K_S0:all pi0:ewp',  # 16
            'K+:loose K+:loose K-:loose pi-:loose',  # 17
            'phi:loose'  # 18
            ]
    return list


def XsplusModes():
    list = ['K+:loose pi+:loose pi-:loose',  # 1
            'K+:loose pi+:loose pi-:loose pi+:loose pi-:loose',  # 2
            'K+:loose pi0:ewp',  # 3
            'K+:loose pi+:loose pi-:loose pi0:ewp',  # 4
            'K+:loose pi0:ewp pi0:ewp',  # 5
            'K+:loose pi+:loose pi-:loose pi0:ewp pi0:ewp',  # 6
            'K_S0:all pi+:loose',  # 7
            'K_S0:all pi+:loose pi+:loose pi-:loose',  # 8
            'K_S0:all pi+:loose pi0:ewp',  # 9
            'K_S0:all pi+:loose pi0:ewp pi0:ewp',  # 10
            'K+:loose eta:loose',  # 11
            'K+:loose eta:loose pi0:ewp',  # 12
            'K+:loose eta:loose pi+:loose pi-:loose',  # 13
            'K_S0:all pi+:loose eta:loose',  # 14
            'K_S0:all pi+:loose pi0:ewp eta:loose',  # 15
            'K+:loose K+:loose K-:loose',  # 16
            'K+:loose K+:loose K-:loose pi0:ewp',  # 17
            'K+:loose K-:loose K_S0:all pi+:loose'  # 18
            ]
    return list


def Xd0Modes():
    list = ['rho0:loose',  # 1
            'omega:loose',  # 2
            'pi+:loose pi-:loose',  # 3
            'pi+:loose pi-:loose pi0:loose',  # 4
            'pi+:loose pi-:loose pi0:loose pi0:loose'  # 5
            ]
    return list


def XdplusModes():
    list = ['rho+:loose',  # 1
            'pi+:loose pi0:ewp',  # 2
            'pi+:loose pi+:loose pi-:loose pi0:ewp',  # 3
            'pi+:loose eta:loose',  # 4
            'pi+:loose pi+:loose pi-:loose'  # 5
            ]
    return list
