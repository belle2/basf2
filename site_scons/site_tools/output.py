#!/usr/bin/env python
# -*- coding: utf-8 -*-

from root_dict import rootcint
from reg_map import map_builder
from SCons.Script import GetOption

# compile: blue
# link   : green
# install: purple
# map    : cyan
# dict   : yellow
# cleanup: black/white

color_map_none = {
    'end': '',
    'compile': '',
    'link': '',
    'install': '',
    'map': '',
    'dict': '',
    'cleanup': '',
    }
color_map_light = {
    'end': '\033[0m',
    'compile': '\033[94m',
    'link': '\033[92m',
    'install': '\033[95m',
    'map': '\033[96m',
    'dict': '\033[93m',
    'cleanup': '\033[37m',
    }
color_map_dark = {
    'end': '\033[0m',
    'compile': '\033[2m\033[34m',
    'link': '\033[2m\033[32m',
    'install': '\033[2m\033[35m',
    'map': '\033[2m\033[36m',
    'dict': '\033[2m\033[33m',
    'cleanup': '\033[30m',
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
            CXXCOMSTR=color_map['compile'] + '*** compiling  : ${SOURCE}'
                + color_map['end'],
            FORTRANPPCOMSTR='${CXXCOMSTR}',
            SHFORTRANPPCOMSTR='${CXXCOMSTR}',
            SHLINKCOMSTR='${LINKCOMSTR}',
            LINKCOMSTR=color_map['link'] + '*** linking    : ${TARGET}'
                + color_map['end'],
            INSTALLSTR=color_map['install'] + '*** ' + install_text
                + ' : ${TARGET}' + color_map['end'],
            MAPCOMSTR=color_map['map'] + '*** map        : ${TARGET}'
                + color_map['end'],
            ROOTCINTCOMSTR=color_map['dict'] + '*** dictionary : ${TARGET}'
                + color_map['end'],
            CLEANUPCOMSTR=color_map['cleanup'] + '*** removing   : %s'
                + color_map['end'],
            )


def exists(env):
    return True


