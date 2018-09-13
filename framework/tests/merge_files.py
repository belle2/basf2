#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import re
import subprocess
import itertools
import ROOT
from ROOT.Belle2 import FileMetaData, EventMetaData
# we don't really need basf2 but it fixes the print buffering problem
import basf2
from b2test_utils import clean_working_directory


def create_testfile(name, release=None, exp=0, run=0, events=100, branchNames=[], **argk):
    """Create a test file from a steering string"""
    global testfile_steering
    env = dict(os.environ)
    env.update(argk)

    steering_file = "steering-{0}.py".format(name)
    with open(steering_file, "w") as f:
        f.write(testfile_steering)

    subprocess.call(["basf2", "-o", name, "--experiment", str(exp), "--run", str(run),
                    "-n", str(events), steering_file] + branchNames, env=env)


def create_testfile_direct(name, metadata=None, release="test_release", user="test_user", seed=None,
                           site="test_site", global_tag="test_globaltag", steering="test_steering"):
    """similar to create_testfile but does it manually without running basf2 for
    full control over the FileMetaData"""
    if metadata is None:
        metadata = FileMetaData()
        if seed is None:
            seed = name + "-seed"

    if seed is not None:
        metadata.setRandomSeed(seed)
    metadata.setCreationData("the most auspicious of days for testing", site, user, release)
    metadata.setDatabaseGlobalTag(global_tag)
    metadata.setSteering(steering)
    f = ROOT.TFile(name, "RECREATE")
    t = ROOT.TTree("persistent", "persistent")
    t.Branch("FileMetaData", metadata)
    t.Fill()
    t.Write()
    t = ROOT.TTree("tree", "tree")
    event_meta = ROOT.Belle2.EventMetaData()
    t.Branch("EventMetaData", event_meta)
    t.Fill()
    t.Write()
    f.Close()


def get_metadata(name="output.root"):
    """Get the metadata out of a root file"""
    out = ROOT.TFile(name)
    t = out.Get("persistent")
    t.GetEntry(0)
    return FileMetaData(t.FileMetaData)


def merge_files(*args, output="output.root", filter_modified=False):
    """run the merging tool on all passed files

    Parameters:
      output: name of the output file
      filter_modified: if True omit warnings that the release is modified and
          consistency cannot be checked
    """
    process = subprocess.run(["merge_basf2_files", "-q", output] + list(args), stdout=subprocess.PIPE)
    # do we want to filter the modified release warning?
    if filter_modified:
        # if so replace them using regular expression
        process.stdout = re.sub(b"^\[WARNING\] File \"(.*?)\" created with modified software ([a-zA-Z0-9\-+]*?): "
                                b"cannot verify that files are compatible\\n", b"", process.stdout, flags=re.MULTILINE)

    # in any case print output
    sys.stdout.buffer.write(process.stdout)
    sys.stdout.buffer.flush()
    # and return exitcode
    return process.returncode


#: Minimal steering file to create an output root file we can merge
testfile_steering = """
import os
import sys
import basf2
basf2.set_log_level(basf2.LogLevel.ERROR)
if "BELLE2_GLOBALTAG" in os.environ:
    basf2.reset_database()
    basf2.use_central_database(os.environ["BELLE2_GLOBALTAG"])
if "BELLE2_SEED" in os.environ:
    basf2.set_random_seed(os.environ["BELLE2_SEED"])
main = basf2.create_path()
main.add_module("EventInfoSetter")
main.add_module("ParticleGun")
main.add_module("RootOutput", branchNames=sys.argv[1:])
basf2.process(main)
"""


def check_01_existing():
    """Check that merging a non exsiting file fails"""
    create_testfile_direct("test2.root")
    return merge_files("test1.root") != 0 and merge_files("test2.root") == 0


def check_02_nonroot():
    """Check that merging fails on non-root input files"""
    with open("test1.root", "w") as f:
        f.write("This is not a ROOT file")
    return merge_files("test1.root") != 0


def check_03_overwrite():
    """Check that overwriting fails if -f is missing"""
    create_testfile_direct("test1.root")
    with open("output.root", "w") as f:
        f.write("stuff")
    return merge_files("test1.root") != 0 and merge_files("-f", "test1.root") == 0


def check_04_access():
    """Check that it fails if we cannot create output file"""
    create_testfile_direct("test1.root")
    return merge_files("test1.root", output="nosuchdir/foo") != 0


def check_05_release():
    """Check that it fails if the releases are different"""
    create_testfile_direct("test1.root")
    create_testfile_direct("test2.root", release="other_release")
    return merge_files("test1.root", "test2.root") != 0


def check_06_empty_release():
    """Check that merging fails with empty release valuses"""
    create_testfile_direct("test1.root")
    create_testfile_direct("test2.root", release="")
    return merge_files("test1.root", "test2.root") != 0


def check_07_modified_release():
    """Check that merging modified release gives warning about that but merging should work"""
    create_testfile_direct("test1.root", release="test_release")
    create_testfile_direct("test2.root", release="test_release-modified")
    return merge_files("test1.root", "test2.root") == 0


def check_08_duplicate_seed():
    """Check that we get a warning for identical seeds but merging should work"""
    create_testfile_direct("test1.root", seed="seed1")
    create_testfile_direct("test2.root", seed="seed1")
    return merge_files("test1.root", "test2.root") == 0


def check_09_different_steering():
    """Check that merging fails if the steering file is different"""
    create_testfile_direct("test1.root",)
    create_testfile_direct("test2.root", steering="my other steering")
    return merge_files("test1.root", "test2.root") != 0


def check_10_different_globaltag():
    """Check that merging fails if the global tag is different"""
    create_testfile_direct("test1.root")
    create_testfile_direct("test2.root", global_tag="other_globaltag")
    return merge_files("test1.root", "test2.root") != 0


def check_11_branches():
    """Check that merging fails if the branches in the event tree are different"""
    create_testfile("test1.root")
    create_testfile("test2.root", branchNames=["EventMetaData"])
    return merge_files("test1.root", "test2.root", filter_modified=True) != 0


def check_12_hadded():
    """Check that merging fails if the file has more then one entry in the persistent tree"""
    create_testfile_direct("test1.root")
    subprocess.call(["hadd", "test11.root", "test1.root", "test1.root"])
    return merge_files("test11.root") != 0


def check_13_nopersistent():
    """Check that merging fails without persistent tree"""
    f = ROOT.TFile("test1.root", "RECREATE")
    t = ROOT.TTree("tree", "tree")
    t.Write()
    f.Close()
    return merge_files("test1.root") != 0


def check_14_noeventtree():
    """Check that merging fails without event tree"""
    f = ROOT.TFile("test1.root", "RECREATE")
    t = ROOT.TTree("persistent", "persistent")
    meta = FileMetaData()
    t.Branch("FileMetaData", meta)
    t.Fill()
    t.Write()
    f.Close()
    return merge_files("test1.root") != 0


def check_15_noeventbranches():
    """Check that merging fails without event tree"""
    f = ROOT.TFile("test1.root", "RECREATE")
    t = ROOT.TTree("persistent", "persistent")
    meta = FileMetaData()
    meta.setCreationData("date", "site", "user", "release")
    t.Branch("FileMetaData", meta)
    t.Fill()
    t.Write()
    t = ROOT.TTree("tree", "tree")
    t.Write()
    f.Close()
    return merge_files("test1.root") != 0


def check_16_nonmergeable():
    """Check that merging fails it there a ron mergeable persistent trees"""
    f = ROOT.TFile("test1.root", "RECREATE")
    t = ROOT.TTree("persistent", "persistent")
    meta = FileMetaData()
    meta.setCreationData("date", "site", "user", "release")
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


def check_17_checkparentLFN():
    """Check that parent LFN get merged correctly"""
    parents = [("a", "b", "c"), ("a", "c", "d")]
    m1 = FileMetaData()
    m2 = FileMetaData()
    lfn1 = ROOT.std.vector("std::string")()
    lfn2 = ROOT.std.vector("std::string")()
    for e in parents[0]:
        lfn1.push_back(e)
    for e in parents[1]:
        lfn2.push_back(e)
    m1.setParents(lfn1)
    m2.setParents(lfn2)
    m1.setRandomSeed("1")
    m2.setRandomSeed("2")
    create_testfile_direct("test1.root", m1)
    create_testfile_direct("test2.root", m2)
    merge_files("test1.root", "test2.root")
    meta = get_metadata()
    should_be = [e for e in sorted(set(parents[0] + parents[1]))]
    is_actual = [meta.getParent(i) for i in range(meta.getNParents())]
    return should_be == is_actual


def check_18_checkEventNr():
    """Check that event and mc numbers are summed correctly"""
    evtNr = [10, 1243, 232, 1272, 25]
    mcNr = [120, 821, 23, 923, 1]
    files = []
    for i, (e, m) in enumerate(zip(evtNr, mcNr)):
        meta = FileMetaData()
        meta.setNEvents(e)
        meta.setMcEvents(m)
        meta.setRandomSeed(str(i))
        files.append("test%d.root" % i)
        create_testfile_direct(files[-1], meta)
    merge_files(*files)
    meta = get_metadata()
    return sum(evtNr) == meta.getNEvents() and sum(mcNr) == meta.getMcEvents()


def check_19_lowhigh():
    """Check that the low/high event numbers are merged correctly"""
    lowhigh = [
        (-1, -1, 0),
        (0, 0, 0),
        (0, 0, 1),
        (0, 1, 0),
        (1, 0, 0),
        (1, 1, 1),
    ]
    files = []
    for i, e in enumerate(lowhigh):
        meta = FileMetaData()
        meta.setNEvents(0 if e == (-1, -1, 0) else 1)
        meta.setRandomSeed(str(i))
        meta.setLow(e[0], e[1], e[2])
        meta.setHigh(e[0], e[1], e[2])
        files.append("test%d.root" % i)
        create_testfile_direct(files[-1], meta)

    # test all possible combinations taking 2 elements from the list plus the
    # full list in one go
    indices = range(len(files))
    tests = list(itertools.permutations(indices, 2)) + [indices]
    for indices in tests:
        low = min(lowhigh[i] for i in indices if lowhigh[i] != (-1, -1, 0))
        high = max(lowhigh[i] for i in indices if lowhigh[i] != (-1, -1, 0))
        if merge_files("-f", "--no-catalog", *(files[i] for i in indices)) != 0:
            return False
        meta = get_metadata()
        if meta.getExperimentLow() != low[0] or meta.getRunLow() != low[1] or meta.getEventLow() != low[2]:
            print("low event should be", low)
            meta.Print()
            return False
        if meta.getExperimentHigh() != high[0] or meta.getRunHigh() != high[1] or meta.getEventHigh() != high[2]:
            print("high event should be", high)
            meta.Print()
            return False
    return True


def check_20_test_file():
    """Check that a merged file passes the check_basf2_file program"""
    create_testfile("test1.root", events=1111)
    create_testfile("test2.root", events=123)
    merge_files("test1.root", "test2.root", filter_modified=True)
    return subprocess.call(["check_basf2_file", "-n", "1234", "--mcevents", "1234",
                            "output.root", "EventMetaData", "MCParticles"]) == 0


def check_21_eventmetadata():
    """Check that merged files has all the correct even infos"""
    create_testfile("test1.root", run=0, events=100, BELLE2_SEED="test1", BELLE2_USER="user1")
    create_testfile("test2.root", run=1, events=100, BELLE2_SEED="test2", BELLE2_USER="user2")
    merge_files("test1.root", "test2.root", "test1.root", filter_modified=True)
    out = ROOT.TFile("output.root")
    events = out.Get("tree")
    entries = events.GetEntriesFast()
    if entries != 300:
        return False
    # we expect to see the events from run 0 twice and the ones from run 1 once.
    # So create a dictionary which contains the expected counts
    eventcount = {(0, 0, i+1): 2 for i in range(100)}
    eventcount.update({(0, 1, i+1): 1 for i in range(100)})
    for i in range(entries):
        events.GetEntry(i)
        e = events.EventMetaData
        eventcount[(e.getExperiment(), e.getRun(), e.getEvent())] -= 1
    return max(eventcount.values()) == 0 and min(eventcount.values()) == 0


def check_XX_filemetaversion():
    """Check that the Version of the FileMetaData hasn't changed.
    If this check fails please check that the changes to FileMetaData don't
    affect merge_basf2_files and adapt the correct version number here."""
    return FileMetaData.Class().GetClassVersion() == 9


if __name__ == "__main__":
    failures = 0
    existing = [e for e in sorted(globals().items()) if e[0].startswith("check_")]
    for name, fcn in existing:
        print("running {0}: {1}".format(name, fcn.__doc__))
        with clean_working_directory():
            if not fcn():
                print("{0} failed".format(name))
                failures += 1
            else:
                print("{0} passed".format(name))

    sys.exit(failures)
