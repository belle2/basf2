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
    list = ['K+:95eff pi-:95eff',  # 1
            'K+:95eff pi-:95eff pi+:95eff pi-:95eff',  # 2
            'K+:95eff pi-:95eff pi0:ewp',  # 3
            'K+:95eff pi-:95eff pi0:ewp pi0:ewp',  # 4
            'K_S0:all pi+:95eff pi-:95eff',  # 5
            'K_S0:all pi+:95eff pi-:95eff pi+:95eff pi-:95eff',  # 6
            'K_S0:all pi0:ewp',  # 7
            'K_S0:all pi+:95eff pi-:95eff pi0:ewp',  # 8
            'K_S0:all pi0:ewp pi0:ewp',  # 9
            'K_S0:all pi+:95eff pi-:95eff pi0:ewp pi0:ewp',  # 10
            'K+:95eff pi-:95eff eta:loose',  # 11
            'K_S0:all eta:loose',  # 12
            'K_S0:all eta:loose pi+:95eff pi-:95eff',  # 13
            'K_S0:all eta:loose pi0:ewp',  # 14
            'K+:95eff K-:95eff K_S0:all',  # 15
            'K+:95eff K-:95eff K_S0:all pi0:ewp',  # 16
            'K+:95eff K+:95eff K-:95eff pi-:95eff',  # 17
            'phi:loose'  # 18
            ]
    return list


def XsplusModes():
    list = ['K+:95eff pi+:95eff pi-:95eff',  # 1
            'K+:95eff pi+:95eff pi-:95eff pi+:95eff pi-:95eff',  # 2
            'K+:95eff pi0:ewp',  # 3
            'K+:95eff pi+:95eff pi-:95eff pi0:ewp',  # 4
            'K+:95eff pi0:ewp pi0:ewp',  # 5
            'K+:95eff pi+:95eff pi-:95eff pi0:ewp pi0:ewp',  # 6
            'K_S0:all pi+:95eff',  # 7
            'K_S0:all pi+:95eff pi+:95eff pi-:95eff',  # 8
            'K_S0:all pi+:95eff pi0:ewp',  # 9
            'K_S0:all pi+:95eff pi0:ewp pi0:ewp',  # 10
            'K+:95eff eta:loose',  # 11
            'K+:95eff eta:loose pi0:ewp',  # 12
            'K+:95eff eta:loose pi+:95eff pi-:95eff',  # 13
            'K_S0:all pi+:95eff eta:loose',  # 14
            'K_S0:all pi+:95eff pi0:ewp eta:loose',  # 15
            'K+:95eff K+:95eff K-:95eff',  # 16
            'K+:95eff K+:95eff K-:95eff pi0:ewp',  # 17
            'K+:95eff K-:95eff K_S0:all pi+:95eff'  # 18
            ]
    return list


def Xd0Modes():
    list = ['rho0:loose',  # 1
            'omega:loose',  # 2
            'pi+:95eff pi-:95eff',  # 3
            'pi+:95eff pi-:95eff pi0:loose',  # 4
            'pi+:95eff pi-:95eff pi0:loose pi0:loose'  # 5
            ]
    return list


def XdplusModes():
    list = ['rho+:loose',  # 1
            'pi+:95eff pi0:ewp',  # 2
            'pi+:95eff pi+:95eff pi-:95eff pi0:ewp',  # 3
            'pi+:95eff eta:loose',  # 4
            'pi+:95eff pi+:95eff pi-:95eff'  # 5
            ]
    return list
