#!/usr/bin/env python
# -*- coding: utf-8 -*-

from SCons.Action import Action

split_debug_action = Action(
    'objcopy --only-keep-debug ${TARGET} ${TARGET.dir}/.debug/${TARGET.file}.debug && '
    'objcopy --strip-debug --strip-unneeded --add-gnu-debuglink ${TARGET.dir}/.debug/${TARGET.file}.debug ${TARGET}',
    "${STRIPCOMSTR}", chdir=False)

strip_debug_action = Action(
    'objcopy --strip-debug --strip-unneeded ${TARGET}',
    "${STRIPCOMSTR}", chdir=False)


def strip_debug_method(env, target):
    result = []
    for t in target:
        if env['SPLIT_DEBUGINFO']:
            result.append(t.dir.Dir(".debug").File(t.name+".debug"))
            env.SideEffect(result[-1], t)
            env.AddPostAction(t, split_debug_action)
            env.Clean(t, result[-1])
        else:
            env.AddPostAction(t, strip_debug_action)
    return result


def generate(env):
    env.AddMethod(strip_debug_method, 'StripDebug')
    env['SPLIT_DEBUGINFO'] = True


def exists(env):
    return True
