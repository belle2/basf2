#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from pathlib import Path

import pandas as pd
import seaborn as sns
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

from prompt import ValidationSettings
import svd.validation_utils as vu

import ROOT as r
r.PyConfig.IgnoreCommandLineOptions = True

matplotlib.use('Agg')
plt.style.use("belle2")

#: Tells the automated system some details of this script
settings = ValidationSettings(name="caf_svd_time",
                              description=__doc__,
                              download_files=[],
                              expert_config=None)


def run_validation(job_path, input_data_path=None, **kwargs):
    '''job_path will be replaced with path/to/calibration_results
    input_data_path will be replaced with path/to/data_path used for calibration
    e.g. /group/belle2/dataprod/Data/PromptSkim/'''

    collector_output_dir = Path(job_path) / 'SVDTimeValidation/0/collector_output/default/'
    output_dir = Path(kwargs.get('output_dir', 'SVDTimeValidation_output'))
    plots_per_run = output_dir / 'runs'

    plots_per_run.mkdir(parents=True, exist_ok=True)

    files = list(collector_output_dir.glob('**/CollectorOutput.root'))

    agreements = {algo: {} for algo in vu.time_algorithms}
    precisions = {algo: {} for algo in vu.time_algorithms}
    discriminations = {algo: {} for algo in vu.time_algorithms}
    shift_agreements = {algo: {} for algo in vu.time_algorithms}
    entries_onTracks = {algo: {} for algo in vu.time_algorithms}
    entries_eventT0 = {algo: {} for algo in vu.time_algorithms}

    roc_U = {algo: {} for algo in vu.time_algorithms}
    roc_V = {algo: {} for algo in vu.time_algorithms}

    CollectorHistograms = vu.get_merged_collector_histograms(files)

    max_total_run = 0
    total_item = 0
    for algo in CollectorHistograms:
        for exp in CollectorHistograms[algo]:
            nRun = len(CollectorHistograms[algo][exp])
            total_item += nRun
            if nRun > max_total_run:
                max_total_run = nRun
    total_length = max_total_run * len(vu.time_algorithms)

    print(f'Looping over {total_item} items')
    count = 0
    vu.progress(0, total_item)

    shift_histos = {}

    for algo in CollectorHistograms:
        shift_histos[algo] = {}
        for exp in CollectorHistograms[algo]:
            for run in CollectorHistograms[algo][exp]:
                # print(f"working with : algo {algo} exp {exp} run {run}")

                histos = vu.get_histos(CollectorHistograms[algo][exp][run])

                if histos is None:
                    print(f'Skipping file algo {algo} exp {exp} run {run}')
                    continue

                # if some histogram is empty (too little stat) do not crash but skip that file for that calibration
                try:
                    entries_eventT0_ = histos['eventT0'].GetEntries()
                    if run not in entries_eventT0[algo] or entries_eventT0_ > entries_eventT0[algo][run]:
                        agreements[algo][run] = {key: vu.get_agreament(histos['eventT0'], h_diff)
                                                 for key, h_diff in histos['diff'].items()}
                        precisions[algo][run] = {key: vu.get_precision(h_diff)
                                                 for key, h_diff in histos['diff'].items()}
                        discriminations[algo][run] = {key: vu.get_roc_auc(histos['onTracks'][key], histos['offTracks'][key])
                                                      for key in histos['onTracks']}
                        shift_agreements[algo][run] = {key: vu.get_shift_agreament(hShift)
                                                       for key, hShift in histos['timeShifter'].items()}
                        entries_onTracks[algo][run] = {key: val.GetEntries() for key, val in histos['onTracks'].items()}
                        entries_eventT0[algo][run] = entries_eventT0_

                        for key, hShift in histos['timeShifter'].items():
                            if key in shift_histos[algo]:
                                shift_histos[algo][key].Add(hShift)
                            else:
                                shift_histos[algo][key] = hShift

                        vu.make_combined_plot('*U', histos,
                                              title=f'exp {exp} run {run} U {algo}')
                        plt.savefig(plots_per_run / f'{exp}_{run}_U_{algo}.pdf')
                        plt.close()

                        vu.make_combined_plot('*V', histos,
                                              title=f'exp {exp} run {run} V {algo}')
                        plt.savefig(plots_per_run / f'{exp}_{run}_V_{algo}.pdf')
                        plt.close()

                        roc_U[algo][run] = vu.make_roc(vu.get_combined(histos['onTracks'], '*U'),
                                                       vu.get_combined(histos['offTracks'], '*U'))
                        roc_V[algo][run] = vu.make_roc(vu.get_combined(histos['onTracks'], '*V'),
                                                       vu.get_combined(histos['offTracks'], '*V'))
                except AttributeError:
                    print(f'Skipping file algo {algo} exp {exp} run {run}')
                    continue
                vu.progress(count + 1, total_item)
                count += 1

    print()

    for algo, KeyHisto in shift_histos.items():
        c1 = r.TCanvas("c1", "c1", 640, 480)
        outPDF = f"{output_dir}/shift_histograms_{algo}.pdf"
        c1.Print(outPDF + "[")
        topPad = r.TPad("topPad", "topPad", 0, 0.5, 1, 1)
        btmPad = r.TPad("btmPad", "btmPad", 0, 0, 1, 0.5)
        topPad.SetMargin(0.1, 0.1, 0, 0.149)
        btmPad.SetMargin(0.1, 0.1, 0.303, 0)
        topPad.SetNumber(1)
        btmPad.SetNumber(2)
        topPad.Draw()
        btmPad.Draw()
        isOdd = True
        for key, hShift in KeyHisto.items():
            hShift.SetStats(0)
            for yn in range(hShift.GetNbinsY()):
                norm = (hShift.ProjectionX("tmp", yn + 1, yn + 1, "")).GetMaximum()
                if norm <= 0:
                    continue
                for xn in range(hShift.GetNbinsX()):
                    hShift.SetBinContent(xn + 1, yn + 1, hShift.GetBinContent(xn + 1, yn + 1) / norm)
            if isOdd:
                # c1.Clear()
                # c1.cd(1)
                topPad.cd()
                hShift.Draw("colz")
            else:
                # c1.cd(2)
                btmPad.cd()
                hShift.Draw("colz")
                c1.Print(outPDF, "Title:" + hShift.GetName())
            isOdd = not isOdd
        c1.Print(outPDF + "]")

    dd = {}
    runs = sorted(agreements[vu.time_algorithms[0]])
    dd['run'] = sum([[i]*len(vu.names_sides) for i in runs], [])
    dd['name'] = vu.names_sides*len(runs)
    dd['side'] = [i[-1] for i in dd['name']]

    for algo in vu.time_algorithms:
        dd[f'agreement_{algo}'] = [agreements[algo][run][side] for run, side in zip(dd['run'], dd['name'])]
        dd[f'precision_{algo}'] = [precisions[algo][run][side] for run, side in zip(dd['run'], dd['name'])]
        dd[f'discrimination_{algo}'] = [discriminations[algo][run][side] for run, side in zip(dd['run'], dd['name'])]
        dd[f'shift_agreement_{algo}'] = [shift_agreements[algo][run][side] for run, side in zip(dd['run'], dd['name'])]
        dd[f'entries_onTracks_{algo}'] = [entries_onTracks[algo][run][side] for run, side in zip(dd['run'], dd['name'])]
        dd[f'entries_eventT0_{algo}'] = [entries_eventT0[algo][run] for run, side in zip(dd['run'], dd['name'])]

    # Make ROC plots
    for run in runs:
        plt.figure()
        plt.plot(*roc_U['CoG6'][run], 'k-', label='CoG6 U')
        plt.plot(*roc_V['CoG6'][run], 'k:', label='CoG6 V')
        plt.plot(*roc_U['CoG3'][run], 'b-', label='CoG3 U')
        plt.plot(*roc_V['CoG3'][run], 'b:', label='CoG3 V')
        plt.plot(*roc_U['ELS3'][run], 'r-', label='ELS3 U')
        plt.plot(*roc_V['ELS3'][run], 'r:', label='ELS3 V')
        plt.legend(loc='lower left')
        plt.xlabel('sgn efficiency')
        plt.ylabel('bkg rejection')
        plt.title(f'ROC run {run}')
        plt.xlim((0, 1))
        plt.ylim((0, 1))
        plt.tight_layout()
        plt.savefig(plots_per_run / f'ROC_{run}.pdf')
        plt.close()

    df = pd.DataFrame(dd)
    df.to_pickle(output_dir / 'df.pkl')

    # df = pd.read_pickle('df.pkl')

    print('Making combined plots')

    for algo in vu.time_algorithms:
        plt.figure(figsize=(6.4*max(2, total_length/30), 4.8*2))
        ax = sns.violinplot(x='run', y=f'agreement_{algo}', hue='side', data=df, split=True)
        ax.set_ylim([-2, 2])
        ax.xaxis.set_minor_locator(ticker.NullLocator())
        plt.axhline(0, color='black', linestyle='--')
        plt.axhline(0.5, color='black', linestyle=':')
        plt.axhline(-0.5, color='black', linestyle=':')
        plt.setp(ax.get_xticklabels(), rotation=90)
        plt.tight_layout()
        plt.savefig(output_dir / f'agreement_{algo}.pdf')
        plt.close()

        plt.figure(figsize=(6.4*max(2, total_length/30), 4.8*2))
        ax = sns.violinplot(x='run', y=f'precision_{algo}', hue='side', data=df, split=True)
        ax.set_ylim([0, 50])
        ax.xaxis.set_minor_locator(ticker.NullLocator())
        plt.axhline(10, color='black', linestyle=':')
        plt.axhline(20, color='black', linestyle=':')
        plt.setp(ax.get_xticklabels(), rotation=90)
        plt.tight_layout()
        plt.savefig(output_dir / f'precision_{algo}.pdf')
        plt.close()

        plt.figure(figsize=(6.4*max(2, total_length/30), 4.8*2))
        ax = sns.violinplot(x='run', y=f'discrimination_{algo}', hue='side', data=df, split=True)
        ax.set_ylim([0.5, 1])
        ax.xaxis.set_minor_locator(ticker.NullLocator())
        plt.axhline(0.8, color='black', linestyle=':')
        plt.axhline(0.9, color='black', linestyle=':')
        plt.setp(ax.get_xticklabels(), rotation=90)
        plt.tight_layout()
        plt.savefig(output_dir / f'discrimination_{algo}.pdf')
        plt.close()

        plt.figure(figsize=(6.4*max(2, total_length/30), 4.8*2))
        ax = sns.violinplot(x='run', y=f'shift_agreement_{algo}', hue='side', data=df, split=True, cut=0)
        ax.xaxis.set_minor_locator(ticker.NullLocator())
        ax.set_ylim([0.0, 3.5])
        plt.axhline(0, color='black', linestyle='--')
        plt.axhline(0.5, color='black', linestyle=':')
        plt.axhline(1.0, color='black', linestyle=':')
        plt.axhline(2.0, color='black', linestyle=':')
        plt.setp(ax.get_xticklabels(), rotation=90)
        plt.tight_layout()
        plt.savefig(output_dir / f'shift_agreement_{algo}.pdf')
        plt.close()

        plt.figure(figsize=(6.4*max(2, total_length/30), 4.8*2))
        ax = sns.violinplot(x='run', y=f'entries_onTracks_{algo}', hue='side', data=df, split=True, cut=0)
        ax.xaxis.set_minor_locator(ticker.NullLocator())
        plt.setp(ax.get_xticklabels(), rotation=90)
        plt.tight_layout()
        plt.savefig(output_dir / f'entries_onTracks_{algo}.pdf')
        plt.close()

        plt.figure(figsize=(6.4*max(2, total_length/30), 4.8*2))
        ax = sns.violinplot(x='run', y=f'entries_eventT0_{algo}', hue='side', data=df, split=True)
        ax.xaxis.set_minor_locator(ticker.NullLocator())
        plt.setp(ax.get_xticklabels(), rotation=90)
        plt.tight_layout()
        plt.savefig(output_dir / f'entries_eventT0_{algo}.pdf')
        plt.close()


if __name__ == '__main__':

    import argparse
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawTextHelpFormatter)

    # b2val-prompt-run wants to pass to the script also input_data_path
    # and requested_iov. As they are not required by this validation I just accept
    # them together with calibration_results_dir and then ignore them
    parser.add_argument('calibration_results_dir',
                        help='The directory that contains the collector outputs',
                        nargs='+')

    parser.add_argument('-o', '--output_dir',
                        help='The directory where all the output will be saved',
                        default='SVDTimeValidation_output')
    args = parser.parse_args()

    run_validation(args.calibration_results_dir[0], output_dir=args.output_dir)
