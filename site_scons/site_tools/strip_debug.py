#!/usr/bin/env python
# -*- coding: utf-8 -*-

from SCons.Action import Action

# Perform debug info splitting according to the GDB manual's guidelines
# https://sourceware.org/gdb/onlinedocs/gdb/Separate-Debug-Files.html
split_debug_action = Action(
    'objcopy ${STRIP_EXTRA_ARGUMENTS} --only-keep-debug ${TARGET.file} ${TARGET.file}.debug && '
    'objcopy --add-gnu-debuglink ${TARGET.file}.debug ${TARGET.file} && '
    'mv ${TARGET.file}.debug .debug/${TARGET.file}.debug && '
    'objcopy --strip-debug --strip-unneeded ${STRIP_EXTRA_ARGUMENTS} ${TARGET.file}',
    "${STRIPCOMSTR}", chdir=True)

strip_debug_action = Action(
    'objcopy --strip-debug --strip-unneeded ${STRIP_EXTRA_ARGUMENTS} ${TARGET}',
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
    env['STRIP_EXTRA_ARGUMENTS'] = "-R '.gnu.debuglto_*'"


def exists(env):
    return True
