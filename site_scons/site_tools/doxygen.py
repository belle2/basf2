#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Custom builder for the module IO images.

import os
import re
import subprocess
from pathlib import Path
from SCons.Builder import Builder

# module in b2modmap file
module_re = \
    re.compile(r'^REG_MODULE\((\S*)\)$', re.M)


def module_io_emitter(target, source, env):
    target = []
    if not env.get('HAS_DOT', False):
        return (target, source)
    for source_file in source:
        contents = source_file.get_text_contents()
        for entry in module_re.findall(contents):
            target.append(os.path.join('build', 'module_io', entry + '.png'))
    return (target, source)


def module_io(target, source, env):
    for target_file in target:
        # make sure the target exists even if the plot creation fails
        Path(str(target_file)).touch()

        dir = os.path.dirname(str(target_file))
        module = os.path.splitext(os.path.basename(str(target_file)))[0]
        if module in ['EclDisplay', 'Rbuf2Ds', 'FastRbuf2Ds', 'Rbuf2Rbuf', 'Ds2Raw']:
            return None

        try:
            subprocess.run(['basf2', '--module-io', module],
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, cwd=dir, timeout=60)
        except Exception:
            continue
        try:
            subprocess.run(['dot', module + '.dot', '-Tpng', '-o', module + '.png'],
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, cwd=dir, timeout=60)
        except Exception:
            pass

    return None


def doxygen_groups_emitter(target, source, env):
    return (['Doxygen.h'], [])


def doxygen_groups(target, source, env):
    groups_file = open(str(target[0]), 'w')
    groups_file.write("""
/** @defgroup DataObjects Data objects
 */

/** @defgroup Modules Modules
 */

/** @defgroup Packages Packages
 */

""")

    for package in env['AVAILABLE_PACKAGES']:
        groups_file.write(f"""
/** @defgroup {package} {package}
 *  @ingroup Packages
 */

/** @defgroup {package}_dataobjects {package} data objects
 *  @ingroup {package}
 *  @ingroup DataObjects
 */

/** @defgroup {package}_modules {package} modules
 *  @ingroup {package}
 *  @ingroup Modules
 */

 """)
    groups_file.close()


def doxyfile(target, source, env):
    content = source[0].get_text_contents().replace('BELLE2_RELEASE', env.GetOption('doxygen'))
    if env.get('HAS_DOT', False):
        content = content.replace('HAVE_DOT               = NO', 'HAVE_DOT               = YES')
    target_file = open(str(target[0]), 'w')
    target_file.write(content)
    target_file.close()


# define builder for module-io plots
moduleio = Builder(action=module_io, emitter=module_io_emitter)
moduleio.action.cmdstr = '${MODULEIOCOMSTR}'

# define builder for doxygen
doxygen = Builder(action=f'doxygen $SOURCE 2>&1 > build/doxygen.log | sed "s;^{os.environ.get("BELLE2_LOCAL_DIR", "")}/;;g" 1>&2',
                  emitter=lambda target, source, env: (['build/doxygen/html/index.html'], source))


def generate(env):
    env['BUILDERS']['ModuleIo'] = moduleio
    env['BUILDERS']['DoxygenGroups'] = Builder(action=doxygen_groups, emitter=doxygen_groups_emitter)
    env['BUILDERS']['Doxyfile'] = Builder(action=doxyfile)
    env['BUILDERS']['Doxygen'] = doxygen
    for builder in ['DoxygenGroups', 'Doxyfile', 'Doxygen']:
        env['BUILDERS'][builder].action.cmdstr = '${DOXYGENCOMSTR}'


def exists(env):
    return True
