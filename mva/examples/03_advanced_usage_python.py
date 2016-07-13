#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2016

import basf2_mva

if __name__ == "__main__":

    # Perform an sPlot training
    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector("train.root")
    general_options.m_weightfile = "MVAFull"
    general_options.m_treename = "tree"
    general_options.m_variables = basf2_mva.vector('M', 'p', 'pz', 'daughter(0, Kid)')
    general_options.m_target_variable = "isSignal"

    fastbdt_options = basf2_mva.FastBDTOptions()
    basf2_mva.teacher(general_options, fastbdt_options)

    general_options.m_weightfile = "MVAOrdinary"
    general_options.m_variables = basf2_mva.vector('p', 'pz', 'daughter(0, Kid)')
    basf2_mva.teacher(general_options, fastbdt_options)

    meta_options = basf2_mva.MetaOptions()
    meta_options.m_use_splot = True
    meta_options.m_splot_variable = "M"
    # SPlot training assumes that the datafile given to the general options contains only data
    # It requires an additional file with MC information from which it can extract the distribution
    # of the discriminating variable (in this case M).
    # Here we use the same file
    meta_options.m_splot_mc_files = basf2_mva.vector("train.root")

    # First we do an ordinary sPlot training
    general_options.m_weightfile = "MVASPlot"
    meta_options.m_splot_combined = False
    meta_options.m_splot_boosted = False
    basf2_mva.teacher(general_options, fastbdt_options, meta_options)

    # Now we combine the sPlot training with a PDF classifier for M, in one step
    general_options.m_weightfile = "MVASPlotCombined"
    meta_options.m_splot_combined = True
    meta_options.m_splot_boosted = False
    basf2_mva.teacher(general_options, fastbdt_options, meta_options)

    # Now we use a bossted sPlot training
    general_options.m_weightfile = "MVASPlotBoosted"
    meta_options.m_splot_combined = False
    meta_options.m_splot_boosted = True
    basf2_mva.teacher(general_options, fastbdt_options, meta_options)

    # And finally a boosted and combined training
    general_options.m_weightfile = "MVASPlotCombinedBoosted"
    meta_options.m_splot_combined = True
    meta_options.m_splot_boosted = True
    basf2_mva.teacher(general_options, fastbdt_options, meta_options)

    # Also do a training of only the pdf classifier
    pdf_options = basf2_mva.PDFOptions()
    general_options.m_method = 'PDF'
    general_options.m_weightfile = "MVAPdf"
    general_options.m_variables = basf2_mva.vector('M')
    basf2_mva.teacher(general_options, pdf_options)

    # Apply the trained methods on data
    basf2_mva.expert(basf2_mva.vector('MVAPdf', 'MVAFull', 'MVAOrdinary', 'MVASPlot',
                                      'MVASPlotCombined', 'MVASPlotBoosted', 'MVASPlotCombinedBoosted'),
                     basf2_mva.vector('train.root'), 'tree', 'expert.root')
