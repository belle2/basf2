"""
b2test_utils.classversion - Helper functions to inspect and verify ROOT dictionaries
------------------------------------------------------------------------------------

This module contains some functions to make sure classes which have a ROOT dictionary
are healthy by allowing to check that

* All base classes are fully defined and not forward declared
* All members have a dictionary
* The class version and checksum is as expected to avoid mistakes where the
  classdef is not increased after class changes

Users should only need to call ``b2code-classversion-check`` and ``b2code-classversion-update``
"""

import re
import os
import sys
import ROOT


class ClassVersionError(Exception):
    """Exception to report class version errors"""
    pass


def printMessage(filename, line, char, text):
    """Print a message similar to GCC or Clang: ``filename:line:column: error: text``
    so that it gets picked up by the build system"""
    print(f"{os.path.relpath(filename)}:{line}:{char}: {text}", file=sys.stderr, flush=True)


def check_base_classes(tclass):
    """Recursively check all base classes of a TClass to make sure all are well defined"""
    bases = tclass.GetListOfBases()
    if not bases:
        raise ClassVersionError(f"class {tclass.GetName()} is incomplete")
    for base in bases:
        baseclass = base.GetClassPointer()
        if not baseclass:
            raise ClassVersionError(f"incomplete base class for {tclass.GetName()}: {base.GetName()}")
        check_base_classes(baseclass)


def check_dictionary(classname):
    """Make sure we have a dictionary for the class and all its members"""
    tclass = ROOT.TClass.GetClass(classname)
    if not tclass:
        raise ClassVersionError(f"Cannot find TClass for {classname}")
    streamerinfo = tclass.GetStreamerInfo()
    if streamerinfo:
        for element in streamerinfo.GetElements():
            elementclass = element.GetClassPointer()
            if elementclass and not elementclass.IsLoaded():
                raise ClassVersionError(f"{tclass.GetName()}: Missing dictionary for {element.GetName()} "
                                        f"type {elementclass.GetName()}")


def get_class_version(classname):
    """Get the Class version and checksum for a fully qualified C++ class name"""
    tclass = ROOT.TClass.GetClass(classname)
    if not tclass:
        raise ClassVersionError(f"Cannot find TClass for {classname}")
    # good time to also check base classes
    check_base_classes(tclass)
    version = tclass.GetClassVersion()
    checksum = tclass.GetCheckSum()
    return version, checksum


def iterate_linkdef(filename):
    """Iterate through the lines of a ROOT linkdef file.

    This function is a generator that foreach line returns a tuple where the
    first element is True if there was a class link pragma on that line and
    False otherwise.

    If it was a class link pragma the second element will be a tuple
    ``(startcolumn, classname, flag, options)`` where

     * ``startcolumn`` is the column the classname started in the line
     * ``classname`` is the class name
     * ``flag`` is the optional plus or minus after the class name to select
       whether or not to support writing to file
     * ``options`` is a dictionary of the options found in the comment. The
       content of the comment is split at commas and the result are the options.
       If a options has a value after ``=`` that will be assigned, otherwise the
       value is None. For example a comment ``// implicit, version=bar`` would
       return ``{'implicit':None, 'version':'bar}``
     """
    # regular expression to find link class pragmas
    class_regex = re.compile(r"^\s*#pragma\s+link\s+C\+\+\s+class\s+(.*?)([+-]?);(?:\s*//\s*(.*))?$")
    with open(filename) as linkdef:
        for line in linkdef:
            match = class_regex.match(line)
            # if the line doesn't match return it unchanged
            if not match:
                yield False, line
            else:
                # Otherwise extract the fields
                start = match.start(1)
                classname, flag, comment = match.groups()
                # and parse the comment
                options = {}
                if comment is not None:
                    optionlist = (e.strip().split("=", 1) for e in comment.split(","))
                    options = {e[0]: e[1] if len(e) > 1 else None for e in optionlist}
                # and return the separate parts
                yield True, (start, classname, flag, options)


def check_linkdef(filename):
    """Check a linkdef file for expected versions/checksums

    * If the class has ``version=N, checksum=0x1234`` in the comment in the
      linkdef compare that to what ROOT has created
    * If not warn about the missing version and checksum
    * But skip classes which are not stored to file.
    """
    errors = 0

    def get_int(container, key):
        """Get the value from a container if it exists and try to convert to int.
        Otherwise show error"""
        nonlocal errors
        value = options.get(key, None)
        if value is not None:
            try:
                return int(value, 0)
            except ValueError as e:
                printMessage(filename, nr, column, f"error: expected {key} for {classname} is not valid: {e}")
                errors += 1

    # loop over all lines in the linkdef
    for nr, (isclass, content) in enumerate(iterate_linkdef(filename)):
        # and ignore everything not a class
        if not isclass:
            continue
        column, classname, flag, options = content

        # check if we can actually load the class
        try:
            version, checksum = get_class_version(classname)
        except ClassVersionError as e:
            printMessage(filename, nr, column, f"error: {e}")
            continue

        # no need to check anything else if we don't have storage enabled
        if flag == "-":
            continue
        # but warn if we use old syle streamer
        elif flag == "":
            printMessage(filename, nr, column, f"warning: {classname} using old ROOT3 streamer format. "
                         "Please add + or - after the classname")

        # This class seems to be intended to serialize so make sure we can
        if version != 0:
            try:
                check_dictionary(classname)
            except ClassVersionError as e:
                printMessage(filename, nr, column, f"error: {e}")

        # and check expected version/checksum
        expected_version = get_int(options, "version")
        expected_checksum = get_int(options, "checksum")
        if expected_version is None or expected_checksum is None:
            printMessage(filename, nr, column, f"warning: {classname} has no expected version and checksum")
            continue

        # And now we know what we have ... so check the version
        if version != expected_version:
            printMessage(filename, nr, column, f"error: {classname} expected version mismatch, please update the linkdef")
            errors += 1
        # And if it's non-zero also check the checksum. Zero means deactivated streamer
        elif checksum != expected_checksum and version != 0:
            printMessage(filename, nr, column, f"error: {classname} checksum mismatch! "
                         "Did you forget increasing the linkdef version?")
            errors += 1

    return errors == 0


def update_linkdef(filename):
    """
    Update a given linkdef file and update all the versions and checksums

    Parameters:
        filename (str): The linkdef.h file to be updated
    """

    lines = []
    # Loop over all lines
    for nr, (isclass, content) in enumerate(iterate_linkdef(filename)):
        # And if it's not a class link pragma keep the line unmodified
        if not isclass:
            lines.append(content)
            continue
        # Otherwise check if we need a version and checksum
        column, classname, flag, options = content
        if flag != "-":
            try:
                version, checksum = get_class_version(classname)
                options['version'] = version
                options['checksum'] = hex(checksum)
            except ClassVersionError as e:
                printMessage(filename, nr, column, f"error: {e}")

        # If so convert the options to a suitable comment string
        optionstr = []
        for key, value in sorted(options.items()):
            optionstr.append(key + (f"={value}" if value is not None else ""))
        if optionstr:
            optionstr = " // " + ", ".join(optionstr)
        else:
            optionstr = ""
        # And store a working link class pragme
        lines.append(f"#pragma link C++ class {classname}{flag};{optionstr}\n")

    # And then replace the file
    with open(filename, "w") as newlinkdef:
        newlinkdef.writelines(lines)
