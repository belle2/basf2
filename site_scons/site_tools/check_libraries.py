#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from SCons.Script import AddOption, GetOption, BUILD_TARGETS
from SCons import Node
from SCons.Action import Action
import re
import subprocess


def get_dtneeded(filename):
    """Use readelf (from binutils) to get a list of required libraries for a
    given shared object file. This will just look at the NEEDED entries in the
    dynamic table so in contrast to ldd it will not resolve child
    dependencies recursively. It will also just return the library names, not in
    which directory they might be found."""

    try:
        re_readelf = re.compile(r"^\s*0x0.*\(NEEDED\).*\[(.*)\]\s*$", re.M)
        readelf_out = subprocess.check_output(["readelf", "-d", filename], stderr=subprocess.STDOUT,
                                              env=dict(os.environ, LC_ALL="C"), encoding="utf-8")
        needed_entries = re_readelf.findall(readelf_out)
        return needed_entries
    except Exception as e:
        print("Could not get dependencies for library %s: %s" % (filename, e))
        return None


def get_env_list(env, key):
    """Get a list from the environment by substituting all values and converting
    them to str.  For example to get the list of libraries to be linked in

        >>> get_env_list(env, "LIBS")
        ["framework", "framework_dataobjects"]
    """
    return map(str, env.subst_list(key)[0])


def get_package(env, node):
    """Determine the package of a node by looking at it's sources
    Hopefully the sources will be inside the build directory so look for the
    first one there and determine the package name from its name."""
    builddir = env.Dir("$BUILDDIR").get_abspath()
    for n in node.sources:
        fullpath = n.get_abspath()
        if fullpath.startswith(builddir):
            # split the path components inside the build dir
            components = os.path.relpath(fullpath, builddir).split(os.path.sep)
            # so if the file is directly in the build dir we take the part in
            # front of the _
            if len(components) == 1:
                return components[0].split("_", 2)[0]
            else:
                return components[0]

    return "none"


def print_libs(title, text, pkg, lib, libs):
    """Print information on extra/missing libraries"""
    for library in sorted(libs):
        print("%s:%s:%s -> %s (%s)" % (title, pkg, lib, library, text))


def check_libraries(target, source, env):
    """Check library dependencies. This is called as a builder hence the target and
    source arguments which are ignored."""

    libdir = env.Dir("$LIBDIR").get_abspath()
    # check libraries and modules directories
    stack = [env.Dir("$LIBDIR"), env.Dir("$MODDIR")]
    # and make a list of all .so objects we actually built
    libraries = []
    while stack:
        dirnode = stack.pop()
        # loop over all nodes which are childrens of this directory
        for node in dirnode.all_children():
            if isinstance(node, Node.FS.Dir):
                # and add directories to the list to investigate
                stack.append(node)
            elif node.has_explicit_builder() and str(node).endswith(".so"):
                # FIXME: this one needs some special love because it doesn't
                # really depend on all the dataobjects libraries, we just force
                # it to. Skip for now
                if os.path.basename(node.get_abspath()) == "libdataobjects.so":
                    continue
                fullname = str(node)
                name = os.path.basename(fullname)
                # is it in a modules directory? if so add it to the name
                if "modules" in fullname.split(os.path.sep):
                    name = "modules/" + name
                # find out which package the file belongs to
                pkg = get_package(env, node)
                # and add lib to the list of libraries
                libraries.append((pkg, name, node))

    # now we have them sorted, go through the list
    for pkg, lib, node in sorted(libraries):
        # get the list of libraries mentioned in the SConscript
        given = get_env_list(node.get_build_env(), "$LIBS")
        # and the list of libraries actually needed by the library
        needed = get_dtneeded(node.get_abspath())
        # ignore it if something is wrong with reading the library
        if needed is None:
            continue

        def remove_libprefix(x):
            """small helper to get rid of lib* prefix for requirements as SCons
            seems to do this transparently as well"""
            if x.startswith("lib"):
                print_libs("LIB_WARNING", "dependency given as lib*, please remove "
                           "'lib' prefix in SConscript", pkg, str(node), [x])
                x = x[3:]
            return x

        # filter lib* from all given libraries and emit a Warning for each
        given = map(remove_libprefix, given)

        # TODO: the list of libraries to link against is short name,
        # e.g. framework instead of libframework.so so we have to fix
        # these lists. However for external libraries this is usually
        # complicated by so versions, e.g. the list of linked libraries
        # will not contain libc.so but libc.so.6 and to make matters
        # worse libraries like CLHEP will end up as libCLHEP-$version.so
        # In theory we need to emulate the linker behaviour here to get
        # a perfect matching. A resonable safe assumption would probably
        # be to require just the beginning to match.

        # But for now we just restrict ourselves to libraries in $LIBDIR
        # because we know they don't have so-versions. So reduce the
        # lists to libraries which we can actually find in $LIBDIR
        given_internal = set("lib%s.so" % library for library in given if os.path.exists(os.path.join(libdir,
                                                                                         "lib%s.so" % library)))
        needed_internal = set(library for library in needed if os.path.exists(os.path.join(libdir, library)))

        # now check for extra or missing direct dependencies using
        # simple set operations
        if GetOption("libcheck_extra"):
            extra = given_internal - needed_internal
            print_libs("LIB_EXTRA", "dependency not needed and can be removed from SConscript", pkg, lib, extra)

        if GetOption("libcheck_missing"):
            missing = needed_internal - given_internal
            print_libs("LIB_MISSING", "library needed directly, please add to SConscript", pkg, lib, missing)

    print("*** finished checking library dependencies")


def run(env):
    AddOption("--check-extra-libraries", dest="libcheck_extra", action="store_true", default=False,
              help="if given all libraries will be checked for dependencies in "
              "the SConscript which are not actually needed")
    AddOption("--check-missing-libraries", dest="libcheck_missing", action="store_true", default=False,
              help="if given all libraries will be checked for missing direct dependencies after build")
    # check if any of the two options is set and if so run the checker
    if env.GetOption("libcheck_extra") or env.GetOption("libcheck_missing"):
        # but we need to run it after the build so add a pseudo build command
        libcheck = "#.check_libraries_pseudotarget"
        check_action = Action(check_libraries, "*** Checking library dependencies...")
        env.Command(libcheck, None, check_action)
        env.AlwaysBuild(libcheck)
        # which depends on all other build targets so depending on wether a list
        # of build targets is defined ...
        if not BUILD_TARGETS:
            # we want to run this last so if no targets are specified make sure check is
            # at least run after libraries and modules are built
            env.Depends(libcheck, ["lib", "modules"])
        else:
            # otherwise let it depend just on all defined targets
            env.Depends(libcheck, BUILD_TARGETS)
            # and add make sure it's included in the build targets
            BUILD_TARGETS.append(libcheck)


def generate(env):
    env.AddMethod(run, "CheckLibraryDependencies")


def exists(env):
    return True
