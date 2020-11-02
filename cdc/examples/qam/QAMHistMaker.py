#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
import argparse
import glob

b2.reset_database()
b2.use_database_chain()
b2.use_central_database("GT_gen_data_003.04_gcr2017-08", b2.LogLevel.WARNING)


def ana(exp=1, run=3118, magneticField=True, prefix='', dest='.'):

    # Seach dst files.
    #    files = glob.glob(prefix + '/output_cosmic.{0:0>4}.{1:0>5}'.format(exp, run) + '*.root')
    files = glob.glob(prefix + '/e{0:0>4}/cosmics/r{1:0>5}/all/dst/sub00/dst.cosmic.{0:0>4}.{1:0>5}'.format(exp, run) + '*.root')
    #    files = glob.glob(prefix + 'dst.cosmic.{0:0>4}.{1:0>5}'.format(exp, run) + '*.root')
    #    files = glob.glob(prefix + '/dst_run{0}'.format(run) + '.root')

    # create path
    main = b2.create_path()
    # Input (ROOT file).
    main.add_module('RootInput',
                    inputFileNames=files)

    gearbox = b2.register_module('Gearbox',
                                 fileName="/geometry/GCR_Summer2017.xml",
                                 override=[
                                  ("/Global/length", "8.", "m"),
                                  ("/Global/width", "8.", "m"),
                                  ("/Global/height", "8.", "m"),
                                     ])
    main.add_module(gearbox)
    #    main.add_module('Geometry')
    main.add_module('Geometry',
                    excludedComponents=['EKLM'])

    output = "/".join([dest, 'qam.{0:0>4}.{1:0>5}.root'.format(exp, run)])
    main.add_module('CDCCosmicAnalysis',
                    noBFit=not magneticField,
                    Output=output,
                    treeName='treeTrk',
                    phi0InRad=False,
                    qam=True)

    main.add_module('Progress')
    # process events and print call statistics
    b2.process(main)
    print(b2.statistics)


if __name__ == "__main__":

    parser = argparse.ArgumentParser()

    parser.add_argument('exp', help='Experimental number')
    parser.add_argument('run', help='Run number')
    args = parser.parse_args()
    ana(exp=args.exp, run=args.run, magneticField=True,
        #        prefix='/ghi/fs01/belle2/bdata/users/karim/data/GCR1/build-2017-08-21',
        #        prefix='/home/belle/muchida/basf2/release/cdc/examples/performance/output',
        prefix='/hsm/belle2/bdata/Data/release-00-09-01/DB00000266/GCR1/prod00000001',
        #        prefix='/ghi/fs01/belle2/bdata/users/karim/MC/GCR1/release-00-09-02',
        #        prefix='/ghi/fs01/belle2/bdata/users/karim/MC/GCR1/prerelease-00-09-02',
        #        prefix='/ghi/fs01/belle2/bdata/users/karim/data/GCR1/release-00-09-00_new/',
        # dest='/ghi/fs01/belle2/bdata/group/detector/CDC/qam/GCR1/test'
        dest='.'  # Store current directory.
        )
