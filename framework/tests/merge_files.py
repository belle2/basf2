#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import subprocess
import tempfile
import ROOT
from contextlib import contextmanager
# we don't really need basf2 but it fixes the print buffering problem
import basf2


@contextmanager
def clean_working_directory():
    """Context manager to create a temporary directory and directly us it as
    current working directory"""
    dirname = os.getcwd()
    try:
        with tempfile.TemporaryDirectory() as tempdir:
            os.chdir(tempdir)
            yield tempdir
    finally:
        os.chdir(dirname)


def create_testfile(name, steering, release=None, exp=0, run=0, events=100, branchNames=[], **argk):
    """Create a test file from a steering string"""
    env = dict(os.environ)
    if release is None and "BELLE2_RELEASE" in env:
        del env["BELLE2_RELEASE"]
    else:
        env["BELLE2_RELEASE"] = release

    env.update(argk)

    steering_file = "steering-{0}.py".format(name)
    with open(steering_file, "w") as f:
        f.write(steering)

    subprocess.call(["basf2", "-o", name, "--experiment", str(exp), "--run", str(run),
                    "-n", str(events), steering_file] + branchNames, env=env)


def merge_files(*args, output="output.root"):
    """run the merging tool on all passed files"""
    return subprocess.call(["merge_basf2_files", "-q", output] + list(args))


#: Minimal steering file to create an output root file we can merge
steering = """
import os
import sys
import basf2
basf2.set_log_level(basf2.LogLevel.ERROR)
if "BELLE2_GLOBALTAG" in os.environ:
    basf2.reset_database()
    basf2.use_central_database(os.environ["BELLE2_GLOBALTAG"])
main = basf2.create_path()
main.add_module("EventInfoSetter")
main.add_module("ParticleGun")
main.add_module("RootOutput", branchNames=sys.argv[1:])
basf2.process(main)
"""


def check_01_existing():
    """Check that merging a non exsiting file fails"""
    create_testfile("test2.root", steering)
    return merge_files("test1.root") != 0 and merge_files("test2.root") == 0


def check_02_nonroot():
    """Check that merging fails on non-root input files"""
    with open("test1.root", "w") as f:
        f.write("This is not a ROOT file")
    return merge_files("test1.root") != 0


def check_03_overwrite():
    """Check that overwriting fails if -f is missing"""
    create_testfile("test1.root", steering)
    with open("output.root", "w") as f:
        f.write("stuff")
    return merge_files("test1.root") != 0 and merge_files("-f", "test1.root") == 0


def check_04_access():
    """Check that it fails if we cannot create output file"""
    create_testfile("test1.root", steering)
    return merge_files("test1.root", output="nosuchdir/foo") != 0


def check_05_release():
    """Check that it fails if the releases are different"""
    create_testfile("test1.root", steering, "SomeRelease")
    create_testfile("test2.root", steering, "SomeOtherRelease")
    return merge_files("test1.root", "test2.root") != 0


def check_06_steering():
    """Check that merging fails if the steering file is different"""
    create_testfile("test1.root", steering)
    create_testfile("test2.root", steering + " # add commment")
    return merge_files("test1.root", "test2.root") != 0


def check_07_globaltag():
    """Check that merging fails if the global tag is different"""
    create_testfile("test1.root", steering, BELLE2_GLOBALTAG="GlobalTag")
    create_testfile("test2.root", steering, BELLE2_GLOBALTAG="OtherGlobalTag")
    return merge_files("test1.root", "test2.root") != 0


def check_08_branches():
    """Check that merging fails if the branches in the event tree are different"""
    create_testfile("test1.root", steering)
    create_testfile("test2.root", steering, branchNames=["EventMetaData"])
    return merge_files("test1.root", "test2.root") != 0


def check_09_hadded():
    """Check that merging fails if the file has more then one entry in the persistent tree"""
    create_testfile("test1.root", steering)
    subprocess.call(["hadd", "test11.root", "test1.root", "test1.root"])
    return merge_files("test11.root") != 0


def check_10_nopersistent():
    """Check that merging fails without persistent tree"""
    f = ROOT.TFile("test1.root", "RECREATE")
    t = ROOT.TTree("tree", "tree")
    t.Write()
    f.Close()
    return merge_files("test1.root") != 0


def check_12_noeventtree():
    """Check that merging fails without event tree"""
    f = ROOT.TFile("test1.root", "RECREATE")
    t = ROOT.TTree("persistent", "persistent")
    meta = ROOT.Belle2.FileMetaData()
    t.Branch("FileMetaData", meta)
    t.Fill()
    t.Write()
    f.Close()
    return merge_files("test1.root") != 0


def check_13_noeventbranches():
    """Check that merging fails without event tree"""
    f = ROOT.TFile("test1.root", "RECREATE")
    t = ROOT.TTree("persistent", "persistent")
    meta = ROOT.Belle2.FileMetaData()
    t.Branch("FileMetaData", meta)
    t.Fill()
    t.Write()
    t = ROOT.TTree("tree", "tree")
    t.Write()
    f.Close()
    return merge_files("test1.root") != 0


def check_14_nonmergeable():
    """Check that merging fails without persistent tree"""
    f = ROOT.TFile("test1.root", "RECREATE")
    t = ROOT.TTree("persistent", "persistent")
    meta = ROOT.Belle2.FileMetaData()
    t.Branch("FileMetaData", meta)
    t.Branch("AnotherMetaData", meta)
    t.Fill()
    t.Write()
    t = ROOT.TTree("tree", "tree")
    t.Branch("EventMetaData", meta)
    t.Fill()
    t.Write()
    f.Close()
    return merge_files("test1.root") != 0


def check_XX_filemetaversion():
    """Check that the Version of the FileMetaData hasn't changed.
    If this check fails please check that the changes to FileMetaData don't
    affect merge_basf2_files and adapt the correct version number here."""
    return ROOT.Belle2.FileMetaData.Class().GetClassVersion() == 8


if __name__ == "__main__":
    failures = 0
    existing = [(name, fcn) for (name, fcn) in sorted(globals().items()) if name.startswith("check_")]
    for name, fcn in existing:
        print("running {0}: {1}".format(name, fcn.__doc__))
        with clean_working_directory():
            if not fcn():
                print("{0} failed".format(name))
                failures += 1
            else:
                print("{0} passed".format(name))

    sys.exit(failures)
