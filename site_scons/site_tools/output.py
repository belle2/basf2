#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from SCons.Script import GetOption

# compile: blue
# link   : green
# install: purple
# copy   : purple
# map    : cyan
# dict   : yellow
# doc    : gray
# cleanup: black/white
# strip  : black/white

color_map_none = {
    'end': '',
    'compile': '',
    'link': '',
    'install': '',
    'copy': '',
    'map': '',
    'dict': '',
    'doc': '',
    'cleanup': '',
    'strip': '',
}
color_map_light = {
    'end': '\033[0m',
    'compile': '\033[94m',
    'link': '\033[92m',
    'install': '\033[95m',
    'copy': '\033[95m',
    'map': '\033[96m',
    'dict': '\033[93m',
    'doc': '\033[37m',
    'cleanup': '\033[37m',
    'strip': '\033[37m',
}
color_map_dark = {
    'end': '\033[0m',
    'compile': '\033[2m\033[34m',
    'link': '\033[2m\033[32m',
    'install': '\033[2m\033[35m',
    'copy': '\033[2m\033[35m',
    'map': '\033[2m\033[36m',
    'dict': '\033[2m\033[33m',
    'doc': '\033[90m',
    'cleanup': '\033[30m',
    'strip': '\033[30m',
}


def generate(env):
    color_map = color_map_none
    if GetOption('color') == 'light':
        color_map = color_map_light
    elif GetOption('color') == 'dark':
        color_map = color_map_dark

    if not GetOption('verbose'):
        if GetOption('no-symlink'):
            install_text = 'installing'
        else:
            install_text = 'symlinking'
        env.Replace(
            SHCXXCOMSTR='${CXXCOMSTR}',
            CXXCOMSTR=color_map['compile'] + '*** compiling  : ${SOURCE}' + color_map['end'],
            FORTRANPPCOMSTR='${CXXCOMSTR}',
            SHFORTRANPPCOMSTR='${CXXCOMSTR}',
            CCCOMSTR='${CXXCOMSTR}',
            SHCCCOMSTR='${CXXCOMSTR}',
            SHLINKCOMSTR='${LINKCOMSTR}',
            LINKCOMSTR=color_map['link'] + '*** linking    : ${TARGET}' + color_map['end'],
            INSTALLSTR=color_map['install'] + '*** ' + install_text + ' : ${TARGET}' + color_map['end'],
            COPYCOMSTR=color_map['copy'] + '*** copying    : ${TARGET}' + color_map['end'],
            MAPCOMSTR=color_map['map'] + '*** map        : ${TARGET}' + color_map['end'],
            ROOTCLINGCOMSTR=color_map['dict'] + '*** dictionary : ${TARGET}' + color_map['end'],
            CLASSVERSIONCOMSTR=color_map['dict'] + '*** checking   : ${SOURCE}' + color_map['end'],
            MODULEIOCOMSTR=color_map['doc'] + '*** module-io  : ${TARGET}' + color_map['end'],
            DOXYGENCOMSTR=color_map['doc'] + '*** doxygen    : ${TARGET}' + color_map['end'],
            STRIPCOMSTR=color_map['strip'] + '*** stripping  : ${TARGET}' + color_map['end'],
        )

    env['CLEANUPCOMSTR'] = color_map['cleanup'] + '*** removing   : %s' \
        + color_map['end']


def exists(env):
    return True
