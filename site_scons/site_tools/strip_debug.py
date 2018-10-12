#!/usr/bin/env python
# -*- coding: utf-8 -*-

from SCons.Builder import Builder

# define builder for stripping
strip_debug = Builder(
    action='objcopy --only-keep-debug $SOURCE $TARGET && '
    'strip --strip-debug --strip-unneeded $SOURCE && '
    'objcopy --add-gnu-debuglink=$TARGET $SOURCE',
    suffix='.debug')
strip_debug.action.cmdstr = '${STRIPCOMSTR}'


def generate(env):
    env['BUILDERS']['StripDebug'] = strip_debug


def exists(env):
    return True
