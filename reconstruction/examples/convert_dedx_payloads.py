##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import argparse

import basf2


def argument_parser():
    '''
    A simple argument parser.
    '''
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('-e', '--exp',
                        type=int,
                        required=True,
                        help='experiment number',
                        metavar='EXP')
    parser.add_argument('-r', '--run',
                        type=int,
                        required=True,
                        help='run number',
                        metavar='RUN')
    parser.add_argument('-g', '--globaltags',
                        type=str,
                        nargs='*',
                        required=True,
                        help='chain of globaltags to be checked',
                        metavar='GT')
    return parser


def set_experiment_and_run(exp_number, run_number):
    '''
    Set the experiment and run numbers.
    '''
    from ROOT import Belle2  # noqa
    event_meta_data = Belle2.PyStoreObj('EventMetaData')
    Belle2.DataStore.Instance().setInitializeActive(True)
    event_meta_data.registerInDataStore()
    event_meta_data.create(False)
    event_meta_data.setEvent(0)
    event_meta_data.setRun(run_number)
    event_meta_data.setExperiment(exp_number)
    Belle2.DataStore.Instance().setInitializeActive(False)


def set_database(globaltags, exp_number, run_number):
    '''
    Set the database.
    '''
    from ROOT import Belle2  # noqa
    Belle2.DBStore.Instance().reset()
    basf2.conditions.override_globaltags()
    for globaltag in globaltags:
        basf2.conditions.append_globaltag(globaltag)
    set_experiment_and_run(exp_number=int(exp_number),
                           run_number=int(run_number))


if __name__ == "__main__":

    args = argument_parser().parse_args()
    exp_number = args.exp
    run_number = args.run
    globaltags = args.globaltags

    # Load the objects from the database
    from ROOT import Belle2  # noqa
    set_database(globaltags, exp_number, run_number)
    oldpdfs = Belle2.PyDBObj('DedxPDFs')
    cdcpdfs = Belle2.CDCdEdxPDFs()
    pxdpdfs = Belle2.PXDdEdxPDFs()
    svdpdfs = Belle2.SVDdEdxPDFs()

    for i in range(6):  # shortcut for looping over the 6 hypotheses
        cdcpdf = oldpdfs.getCDCPDF(i, False)
        cdcpdfs.setPDF(cdcpdf, i, False)
        cdcpdf_truncated = oldpdfs.getCDCPDF(i, True)
        cdcpdfs.setPDF(cdcpdf_truncated, i, True)

        pxdpdf = oldpdfs.getPXDPDF(i, False)
        pxdpdfs.setPDF(pxdpdf, i, False)
        pxdpdf_truncated = oldpdfs.getPXDPDF(i, True)
        pxdpdfs.setPDF(pxdpdf_truncated, i, True)

        svdpdf = oldpdfs.getSVDPDF(i, False)
        svdpdfs.setPDF(svdpdf, i, False)
        svdpdf_truncated = oldpdfs.getSVDPDF(i, True)
        svdpdfs.setPDF(svdpdf_truncated, i, True)

    db = Belle2.Database.Instance()
    iov = Belle2.IntervalOfValidity(exp_number, run_number, exp_number, run_number)
    db.storeData(str(cdcpdfs.ClassName()).replace('Belle2::', ''), cdcpdfs, iov)
    db.storeData(str(pxdpdfs.ClassName()).replace('Belle2::', ''), pxdpdfs, iov)
    db.storeData(str(svdpdfs.ClassName()).replace('Belle2::', ''), svdpdfs, iov)
