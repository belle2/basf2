#!/usr/bin/env python
# coding: utf8

import sys
import os

# Get Belle2 environment variables
try:
    LOCAL_DIR = os.environ["BELLE2_LOCAL_DIR"]
    VIRTUAL_ENV = os.environ["VIRTUAL_ENV"]
except KeyError:
    print "Belle II software not correctly set up, aborting"
    sys.exit(1)

# this script only works with python > 2.7,
# but git can prepend /usr/bin to PATH!
if sys.hexversion < 0x02070600:
    # but we know where our python version should reside, so let's call
    # ourselves with the newer interpreter
    from subprocess import call
    args = [VIRTUAL_ENV + '/bin/python'] + sys.argv
    ret = call(args)
    sys.exit(ret)

import re
from subprocess import check_output, call, CalledProcessError


def parse_external(line):
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
        raise ValueError("svn::externals line could not be parsed")
    return match.groups()


def find_svnrevision(commit="HEAD"):
    """Find the svn revision the current branch is based on.
    git-svn history should be very linear but we need to now on which svn
    revision the current branch is based. To do this, we traverse the parents
    of the current commit until git svn finds a corresponding svn revision

    return svn revision or None if none could be found
    """
    revlist = check_output(["git", "rev-list", "--parents", commit])
    for line in revlist.splitlines():
        parents = line.split()
        revision = check_output(["git", "svn", "find-rev", parents[0]])
        try:
            revision = int(revision)
            return revision
        except ValueError:
            pass

    return None


# Checking all folders takes far to long, lets provide a list of dirs to check
# as environment variable
externals = os.environ.get("BELLE2_SVN_EXTERNALS", "genfit2").split()

# Check arguments: See if we are called as post-rewrite or post checkout and if
# so determine if we need to do anything
basename = os.path.basename(sys.argv[0])
if basename == "post-checkout":
    prev_head, new_head, branch_switch = sys.argv[1:]
    if not branch_switch or prev_head == new_head:
        # No branch real switch, nothing to do
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
    print "Cannot determine SVN revision, cannot check for externals"
    sys.exit(1)

# Let's change into the release dir
os.chdir(LOCAL_DIR)

# See if there is already an gitignore file, if so read it
gitignore = []
if os.path.exists(".gitignore"):
    f = open(".gitignore")
    gitignore = f.readlines()
    f.close()


# Get the latest revision we checked out. This might not be the revision the
# current branch is based on but finding that revision is more difficult.
current_revision = find_svnrevision()
print "Checking svn:externals for r%s" % current_revision

for e in externals:
    print "directory '%s'" % e
    try:
        prop = check_output(["git", "svn", "propget",
                             "-r%s" % current_revision, "svn:externals", e])
    except CalledProcessError:
        # Apparently no externals here, move on
        continue

    # Fetch all externals
    for line in prop.split("\n"):
        if not line.strip():
            continue

        dirname, revision, url = parse_external(line)
        abspath = os.path.join(LOCAL_DIR, e, dirname)
        # Build command line for svn
        args = ["svn"]
        if os.path.exists(abspath):
            # Seems to exist already, so update to specified revision
            print "Updating", os.path.join(e, dirname),
            args.append("update")
        else:
            # Not there yet, checkout using svn
            print "Fetching", os.path.join(e, dirname),
            print "from", url,
            args += ["checkout", url]

        # If revision is specified, add it after the svn command
        if revision is not None:
            print "r" + revision,
            args.insert(2, "-r" + revision)

        # finish line
        print
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
