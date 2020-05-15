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
from basf2 import B2INFO, B2ERROR


class ClassVersionError(Exception):
    """Exception to report class version errors"""
    pass


class ErrorWithExtraVariables(Exception):
    """Exception class with etra keyword arguments to show in log message"""
    def __init__(self, *args, **argk):
        super().__init__(*args)
        self.variables = argk


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
    ``(line, startcolumn, classname, linkoptions, options)`` where

     * ``line`` is the content of the line
     * ``startcolumn`` is the column the content of the comment started in the line.
       Everything before should be kept as is
     * ``classname`` is the class name
     * ``clingflags`` is the link flags requested by either the optional +-! after
       the class or the ``options=`` in front of the class and a set of the value
       "evolution", "nostreamer", and "noinputoper"
     * ``options`` is a dictionary of the options found in the comment. The
       content of the comment is split at commas and the result are the options.
       If a options has a value after ``=`` that will be assigned, otherwise the
       value is None. For example a comment ``// implicit, version=bar`` would
       return ``{'implicit':None, 'version':'bar}``

    If it wasn't  link pragma the second element is just the unmodified line
    """
    # regular expression to find link class pragmas
    class_regex = re.compile(r"^\s*#pragma\s+link\s+C\+\+\s+(?:options=(.*?)\s)?\s*class\s+(.*?)([+\-!]*);(\s*//\s*(.*))?$")
    with open(filename) as linkdef:
        for line in linkdef:
            match = class_regex.match(line)
            # if the line doesn't match return it unchanged
            if not match:
                yield False, line
            else:
                # Otherwise extract the fields
                linkflags, classname, flags, comment, content = match.groups()
                start = match.start(4) if comment else len(line) - 1  # no comment: start of comment is end of line minus newline
                # parse the linkflags
                clingflags = set()
                for char in flags:
                    clingflags.add({"+": "evolution", "-": "nostreamer", "!": "noinputoper"}.get(char))
                if linkflags is not None:
                    for flag in linkflags.split(","):
                        clingflags.add(flag.strip())
                # and parse the comment
                options = {}
                if content is not None:
                    optionlist = (e.strip().split("=", 1) for e in content.split(","))
                    options = {e[0]: e[1] if len(e) > 1 else None for e in optionlist}
                # and return the separate parts
                yield True, (line, start, classname, clingflags, options)


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
        line, column, classname, clingflags, options = content

        # check if we can actually load the class
        try:
            version, checksum = get_class_version(classname)
        except ClassVersionError as e:
            printMessage(filename, nr, column, f"error: {e}")
            errors += 1
            continue

        # no need to check anything else if we don't have storage enabled
        if "nostreamer" in clingflags:
            if "evolution" in clingflags:
                printMessage(filename, nr, column, f"error: {classname} both, "
                             "no streamer and class evolution requested.")
                errors += 1
                # current ROOT code lets "evolution win so lets continue as if
                # nostreamer wouldn't be present, we flagged the error"
            else:
                continue
        # but warn if we use old syle streamer without evolution rules
        elif "evolution" not in clingflags:
            printMessage(filename, nr, column, f"warning: {classname} using old "
                         "ROOT3 streamer format without evolution. Please add + "
                         "or - (for classes not to be written to file) after the classname.")

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
            printMessage(filename, nr, column, f"warning: {classname} has no "
                         "expected version and checksum")
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


def verify_classlink_version(classname, options):
    """This funciton verifies the version and checksum of a class from a given
    linkdef classlink line and returns new values if they can be updated

    Parameters:
        classname (str): The classname as in the linkdef file
        options (dict): The options currently present in the linkdef file.
            Previous version and checksum are taken from here if they exist

    * If there's nothing to do it will return None
    * If there's any problem it will raise an exception
    * Otherwise it will return a dictionary containing the new ``version`` and
      ``checksum`` to be put in the file
    """
    # Try to get the actual values, if that fails it will raise which is handled
    # by caller
    version, checksum = get_class_version(classname)

    # We have the actual values but what do we know about previous
    # values?
    if "version" in options:
        previous_version = int(options['version'], 0)
        # Don't allow decrease of class version
        if previous_version > version:
            raise ErrorWithExtraVariables(
                "actual class version lower than previous version in linkdef. "
                "Class version cannot decrease",
                previous_version=previous_version, actual_version=version,
            )
        # Also please don't just increase by a massive amount unless the previous
        # version number was negative for "no explicit number"
        elif previous_version > -1 and version > (previous_version + 1):
            raise ErrorWithExtraVariables(
                "actual class version increased by more one compared to the "
                "previous version set in linkdef. Please increase ClassDef in single steps",
                previous_version=previous_version, actual_version=version,
            )
        # Otherwise if we have a checksum make sure it is the same
        # if the version didn't change
        if "checksum" in options:
            previous_checksum = int(options['checksum'], 0)
            if version == previous_version and checksum != previous_checksum:
                raise ErrorWithExtraVariables(
                    "checksum changed but class version didn't change. "
                    "Please increase the class version if you modified the class contents",
                    version=version, actual_checksum=hex(checksum),
                    previous_checksum=hex(previous_checksum),
                )
            # but if both are the same just keep the line unchanged
            elif (version, checksum) == (previous_version, previous_checksum):
                return

    # Ok, we have our values, return them
    return {"version": version, "checksum": hex(checksum)}


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
        line, column, classname, clingflags, options = content
        try:
            # only for classes to be streamed ...
            if "nostreamer" not in clingflags:
                # verify the version ...
                version = verify_classlink_version(classname, options)
                if version is not None:
                    options.update(version)
                    B2INFO(f"Updating ROOT class version and checksum for {classname}",
                           linkdef=f"{os.path.relpath(filename)}:{nr}", **version)
                    # convert the options to a comment string
                    optionstr = []
                    for key, value in sorted(options.items()):
                        optionstr.append(key + (f"={value}" if value is not None else ""))
                    comment = " // " + ", ".join(optionstr)
                    lines.append(f"{line[:column]}{comment}\n")
                    continue

        except Exception as e:
            variables = {"classname": classname, "linkdef": f"{os.path.relpath(filename)}:{nr}"}
            if isinstance(e, ErrorWithExtraVariables):
                variables.update(e.variables)
            B2ERROR(str(e), **variables)

        # Finally, keep the line in case of errors or no change
        lines.append(line)

    # And then replace the file
    with open(filename, "w") as newlinkdef:
        newlinkdef.writelines(lines)
