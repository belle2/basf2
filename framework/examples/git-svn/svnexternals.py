#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import pickle
import sys
import os
import re
from subprocess import check_output, call, CalledProcessError

# @cond internal_test

# Get Belle2 environment variables
try:
    LOCAL_DIR = os.environ["BELLE2_LOCAL_DIR"]
except KeyError:
    print("Belle II software not correctly set up, aborting")
    sys.exit(1)


class SvnExternalsCache(object):
    """Class to obtain svn:externals information and cache it.
    This class will get the svn:externals property on a given path with a given
    revision. It will cache this information in a persistent file to speed up
    lookups

    This class is a context manager so use as

    >>> with SvnExternals() as cache:
    >>>     for dirname, revision, url in cache.get(global_revision, path):
    >>>         if cache.check_revision(dirname, revision):
    >>>            continue
    >>>         ...
    """

    def __init__(self):
        """Create the instance and read the cache file"""
        self._filename = os.path.join(LOCAL_DIR, ".git/svn_external_cache.pkl")

    def __enter__(self):
        """Open the cache file and get the contents"""
        try:
            with open(self._filename, "rb") as f:
                self._externalcache = pickle.load(f)
                self._revisioncache = pickle.load(f)
        except (IOError, EOFError) as e:
            # problem reading file cache, start with empty file
            self._externalcache = {}
            self._revisioncache = {}

        return self

    def __exit__(self, exc_type, exc_value, traceback):
        """Clean up and save current cache contents"""
        try:
            with open(self._filename, "wb") as f:
                pickle.dump(self._externalcache, f)
                pickle.dump(self._revisioncache, f)
        except IOError:
            pass

    def parse_external(self, line):
        """Parse svn:externals property.

        assume that the externals are either defined as
        <folder> [-r<revision>] <url>
        <folder>[@<revision>] <url>

        return folder, revision, url
        revision might be None
        """
        match = re.match("""
            ^(?P<folder>[^@ \t]*)                # match the folder
            (?:(?:@|\s+-r)(?P<revision>[0-9]*))? # optionally, match the revision
            \s*(?P<url>.*)$                      # finally, match the rest as url
            """, line.strip(), re.VERBOSE)
        if not match:
            raise ValueError("svn::externals '%s' line could not be parsed" % line)
        return match.groups()

    def get(self, revision, path):
        """Get the svn:externals for a path in a given revision.
        Returns a list of (dirname, revision, url) tuples, one for each entry
        in the svn:externals property. If property is not set an empty list is
        returned."""
        key = (revision, path)
        if key in self._externalcache:
            return self._externalcache[key]

        data = []
        try:
            prop = check_output(["git", "svn", "propget", "-r%s" % current_revision, "svn:externals", path])

            for line in prop.decode().splitlines():
                if not line.strip():
                    continue
                else:
                    try:
                        data.append(self.parse_external(line))
                    except ValueError as e:
                        print("Error parsing externals %s for r%s: %s" % (external, revision, path))
        except CalledProcessError:
            # Apparently no externals here, move on
            pass

        self._externalcache[key] = data
        return data

    def check_revision(self, path, revision):
        """Check if the requested revision for path is already checkout out.
        Return False if revision is None, not in the cache or different from
        the one in cache."""
        current_revision = self._revisioncache.get(path, None)
        self._revisioncache[path] = revision
        if revision is None:
            return False
        return current_revision == revision


def find_svnrevision(commit="HEAD"):
    """ Finds the last svn commit by greping through the log list of git and
    searching for an git-svn-id. The hash of the first element resturned is
    used to get the svn revision number """
    last_svn_git_hash = check_output(["git", "log", "-n1", "--format=%H",
                                      '--grep=^\s*git-svn-id', commit]).splitlines()[0]
    if len(last_svn_git_hash) == 0:
        return None
    last_svn_hash = check_output(["git", "svn", "find-rev", last_svn_git_hash]).splitlines()[0]

    return int(last_svn_hash)


# Checking all folders takes far to long, lets provide a list of dirs to check
# as environment variable
externals = os.environ.get("BELLE2_SVN_EXTERNALS", "genfit2").split()

# Check arguments: See if we are called as post-rewrite or post checkout and if
# so determine if we need to do anything
basename = os.path.basename(sys.argv[0])
if basename == "post-checkout":
    prev_head, new_head, branch_switch = sys.argv[1:]
    if not branch_switch or prev_head == new_head:
        # No real branch switch, nothing to do
        sys.exit(0)

    prev_revision = find_svnrevision(prev_head)
    current_revision = find_svnrevision(new_head)
    if prev_revision == current_revision:
        # Nothing to do, both branches are based on the same svn revision
        sys.exit(0)
else:
    if basename == "post-rewrite" and sys.argv[1] != "rebase":
        # Nothing to do
        sys.exit(0)

    # Find svn revision current head is based on
    current_revision = find_svnrevision()

if current_revision is None:
    print("Cannot determine SVN revision, cannot check for externals")
    sys.exit(1)

# Let's change into the release dir
os.chdir(LOCAL_DIR)

# See if there is already an gitignore file, if so read it
gitignore = []
if os.path.exists(".gitignore"):
    f = open(".gitignore")
    gitignore = f.readlines()
    f.close()

print("Checking svn:externals for r%s" % current_revision)

with SvnExternalsCache() as cache:
    for e in externals:
        print("directory '%s'" % e)

        for dirname, revision, url in cache.get(current_revision, e):
            path = os.path.join(e, dirname)
            abspath = os.path.join(LOCAL_DIR, e, dirname)
            if cache.check_revision(abspath, revision):
                print("%s already up to date" % path)
                continue
            # Build command line for svn
            args = ["svn"]
            if os.path.exists(abspath):
                # Seems to exist already, so update to specified revision
                print("Updating", path, end=' ')
                args.append("update")
            else:
                # Not there yet, checkout using svn
                print("Fetching", path, end=' ')
                print("from", url, end=' ')
                args += ["checkout", url]

            # If revision is specified, add it after the svn command
            if revision is not None:
                print("r" + revision, end=' ')
                args.insert(2, "-r" + revision)

            # finish line
            print()
            call(args + [abspath])

            # Check if it is already excluded
            exclude = "/%s/" % os.path.join(e, dirname).strip("/")
            if not any(e.strip() == exclude for e in gitignore):
                # If not, add it to the gitignore
                gitignore += ["# ignore directory containing svn:externals\n",
                              exclude + "\n"]

# Write gitignore file
f = open(".gitignore", "w")
f.writelines(gitignore)
f.close()

# @endcond
