#!/usr/bin/env python
# -*- coding: utf-8 -*-

from SCons.Action import Action

strip_debug_action = Action(
    'objcopy --only-keep-debug ${TARGET} ${TARGET.dir}/.debug/${TARGET.file}.debug && '
    'strip --strip-debug --strip-unneeded ${TARGET} && '
    'objcopy --add-gnu-debuglink=${TARGET.dir}/.debug/${TARGET.file}.debug ${TARGET}',
    "${STRIPCOMSTR}", chdir=False)


def strip_debug_method(env, target):
    result = []
    for t in target:
        result.append(t.dir.Dir(".debug").File(t.name+".debug"))
        env.SideEffect(result[-1], t)
        env.AddPostAction(t, strip_debug_action)
        env.Clean(t, result[-1])
    return result


def generate(env):
    env.AddMethod(strip_debug_method, 'StripDebug')


def exists(env):
    return True
