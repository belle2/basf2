import numpy
import matplotlib
import matplotlib.figure
import matplotlib.gridspec
import pandas
import seaborn
import scipy
import scipy.stats


def binom_error(n_sig, n_tot):
    """
    for an efficiency = nSig/nTrueSig or purity = nSig / (nSig + nBckgrd), this function calculates the
    standard deviation according to http://arxiv.org/abs/physics/0701199 .
    """
    variance = numpy.where(n_tot > 0, (n_sig + 1) * (n_sig + 2) / ((n_tot + 2) * (n_tot + 3))
                           - (n_sig + 1) ** 2 / ((n_tot + 2) ** 2), 0)
    return numpy.sqrt(variance)


def poisson_error(n_tot):
    """
    use poisson error, except for 0 we use an 68% CL upper limit
    """
    return numpy.where(n_tot > 0, numpy.sqrt(n_tot), numpy.log(1.0/(1-0.6827)))


class BasePlotter(object):

    quantity = 'Classifier Output'

    def datapoint_plot(self, axis, x, y, xerr=None, yerr=None, label=None):
        p, = axis.plot(x, y, label=label)
        color = p.get_color()
        darker_color = map(lambda x: 0.5*x, p.get_color())
        axis.errorbar(x, y, xerr=xerr, yerr=yerr, fmt='.', color=color, ecolor=darker_color, elinewidth=0.5, alpha=0.5)
        axis.fill_between(x, y-yerr, y+yerr, alpha=0.5, color=color)
        return p


class Plotter(BasePlotter):
    def __init__(self, data, columns=['SignalProbability'], target='isSignal', weight=None, binning={}):
        self.columns = columns
        self.signal = {}
        self.bckgrd = {}
        self.bin_patches = {}
        self.bin_centers = {}
        self.bin_widths = {}
        self.xmax = float('-inf')
        self.xmin = float('inf')
        self.ymax = float('-inf')
        self.ymin = float('inf')
        for column in columns:
            # TODO Optional handle chunk wise dataframes from root_pandas
            if column not in binning:
                # Create histograms for signal and background with the same binning
                _, patches = numpy.histogram(data[column], bins=100, weights=None if weight is None else data[weight])
            else:
                patches = binning[column]
            signal_data = data[data[target] == 1]
            bckgrd_data = data[data[target] == 0]
            signal_hist, _ = numpy.histogram(signal_data[column], bins=patches,
                                             weights=None if weight is None else signal_data[weight])
            bckgrd_hist, _ = numpy.histogram(bckgrd_data[column], bins=patches,
                                             weights=None if weight is None else bckgrd_data[weight])
            self.signal[column] = signal_hist.astype('float')
            self.bckgrd[column] = bckgrd_hist.astype('float')
            # Save binning
            self.bin_patches[column] = patches
            self.bin_centers[column] = (numpy.roll(patches, 1) + patches)[1:] / 2.0
            self.bin_widths[column] = (numpy.roll(patches, 1) - patches)[1:] / 2.0
            # Save maximum and minimum for x and y dimension
            self.xmax = max(data[column].max(), self.xmax)
            self.xmin = min(data[column].min(), self.xmin)
            self.ymax = max(signal_hist.max(), bckgrd_hist.max(), self.ymax)
            self.ymin = min(signal_hist.min(), bckgrd_hist.min(), self.ymin)

    def roc_plot(self, axis):
        plots = []
        for column in self.columns:
            signal = self.signal[column]
            bckgrd = self.bckgrd[column]

            cumsignal = (signal.sum() - signal.cumsum()).astype('float')
            cumbckgrd = (bckgrd.sum() - bckgrd.cumsum()).astype('float')

            efficiency = cumsignal / signal.sum()
            efficiency_error = binom_error(cumsignal, signal.sum())
            purity = cumsignal / (cumsignal + cumbckgrd)
            purity_error = binom_error(cumsignal, cumsignal + cumbckgrd)

            p = self.datapoint_plot(axis, efficiency, purity, xerr=efficiency_error, yerr=purity_error, label=column)
            plots.append(p)

        axis.set_xlim((0, 1))
        axis.set_ylim((0, 1))
        axis.get_xaxis().set_label_text('Efficiency')
        axis.get_yaxis().set_label_text('Purity')
        axis.set_title('ROC Plot')
        axis.legend(handles=plots,  loc=3)
        return plots

    def diag_plot(self, axis, line=None):
        plots = []
        for column in self.columns:
            signal = self.signal[column]
            bckgrd = self.bckgrd[column]
            bin_centers = self.bin_centers[column]
            bin_widths = self.bin_widths[column]

            purity = signal / (signal + bckgrd)
            purity_error = binom_error(signal, signal + bckgrd)

            p = self.datapoint_plot(axis, bin_centers, purity, xerr=bin_widths, yerr=purity_error, label=column)
            plots.append(p)

        axis.plot((self.xmin, self.xmax) if line is None else line, (0, 1), color='black')
        axis.set_xlim((self.xmin, self.xmax))
        axis.set_ylim((0, 1))
        axis.get_xaxis().set_label_text(self.quantity)
        axis.get_yaxis().set_label_text('Purity')
        axis.set_title('Diagonal Plot')
        axis.legend(handles=plots, loc=4)
        return plots

    def dist_plot(self, axis):
        plots = []
        ymax = float('-inf')
        for column in self.columns:
            signal = self.signal[column]
            bckgrd = self.bckgrd[column]
            bin_centers = self.bin_centers[column]
            bin_widths = self.bin_widths[column]

            signal_error = poisson_error(signal)
            bckgrd_error = poisson_error(bckgrd)

            ymax = max(signal.max() + signal_error.max(), bckgrd.max() + bckgrd_error.max(), ymax)

            p = self.datapoint_plot(axis, bin_centers, signal, xerr=bin_widths, yerr=signal_error, label=column + ' Signal')
            plots.append(p)
            p = self.datapoint_plot(axis, bin_centers, bckgrd, xerr=bin_widths, yerr=bckgrd_error, label=column + ' Background')
            plots.append(p)

        axis.set_xlim((self.xmin, self.xmax))
        axis.set_ylim((0, ymax))
        axis.get_xaxis().set_label_text(self.quantity)
        axis.get_yaxis().set_label_text('N')
        axis.set_title('Distribution Plot')
        axis.legend(handles=plots,  loc=4)
        return plots


class ComparisonPlotter(BasePlotter):
    def __init__(self, data_a, data_b, column_a='Train', column_b='Test', target='isSignal', weight=None):

        # Create a common binning for data_a and data_b
        _, patches = numpy.histogram(numpy.r_[data_a[column_a], data_b[column_b]], bins=100,
                                     weights=None if weight is None else numpy.r_[data_a[weight], data_b[weight]])

        self.plotter_a = Plotter(data_a, columns=[column_a], target=target, weight=weight, binning={column_a: patches})
        self.plotter_b = Plotter(data_b, columns=[column_b], target=target, weight=weight, binning={column_b: patches})

        self.column_a = column_a
        self.column_b = column_b
        self.bin_patches = self.plotter_a.bin_patches[column_a]
        self.bin_centers = self.plotter_a.bin_centers[column_a]
        self.bin_widths = self.plotter_a.bin_widths[column_a]
        self.xmax = max(self.plotter_a.xmax, self.plotter_b.xmax)
        self.xmin = min(self.plotter_a.xmin, self.plotter_b.xmin)
        self.ymax = max(self.plotter_a.ymax, self.plotter_b.ymax)
        self.ymin = min(self.plotter_a.ymin, self.plotter_b.ymin)
        self.signal_a = self.plotter_a.signal[column_a]
        self.signal_b = self.plotter_b.signal[column_b]
        self.bckgrd_a = self.plotter_a.bckgrd[column_a]
        self.bckgrd_b = self.plotter_b.bckgrd[column_b]

        # Kolmogorov-Smirnov test on the data itself, not the bins (TODO better way to use binned data and its errors?)
        self.ks_sig = scipy.stats.ks_2samp(data_a[data_a[target] == 1][column_a], data_b[data_b[target] == 1][column_b])
        self.ks_bck = scipy.stats.ks_2samp(data_a[data_a[target] == 0][column_a], data_b[data_b[target] == 0][column_b])

    def dist_plot(self, axis):
        plots = self.plotter_a.dist_plot(axis) + self.plotter_b.dist_plot(axis)
        axis.set_xlim((self.xmin, self.xmax))
        axis.set_ylim((0, self.ymax))
        axis.legend(handles=plots, loc=3,
                    # bbox_to_anchor=(0., -0.175, 1., .102), ncol=2, mode="expand", borderaxespad=0.
                    )
        axis.text(0.02, 0.96, "Kolmogorov-Smirnov test: signal (background) probability = {sig:.3f} ({bck:.3f})".format(
                  sig=self.ks_sig[1], bck=self.ks_bck[1]),
                  size=28, transform=axis.transAxes, bbox=dict(boxstyle="square,pad=0.3", fc="white", ec="w", lw=0, alpha=0))
        return plots

    def diff_plot(self, axis_signal, axis_bckgrd):
        signal = self.signal_a - self.signal_b
        signal_error = poisson_error(self.signal_a + self.signal_b)
        bckgrd = self.bckgrd_a - self.bckgrd_b
        bckgrd_error = poisson_error(self.bckgrd_a + self.bckgrd_b)

        ymin = min((signal-signal_error).min(), (bckgrd-bckgrd_error).min())
        ymax = max((signal+signal_error).max(), (bckgrd+bckgrd_error).max())

        p_signal = self.datapoint_plot(axis_signal, self.bin_centers, signal, xerr=self.bin_widths, yerr=signal_error,
                                       label='(' + self.column_a + ' - ' + self.column_b + ') Signal')
        axis_signal.set_xlim((self.xmin, self.xmax))
        axis_signal.set_ylim(((signal-signal_error).min(), (signal+signal_error).max()))
        axis_signal.get_xaxis().set_label_text(self.quantity)
        axis_signal.get_yaxis().set_label_text('Difference')
        axis_signal.legend(handles=[p_signal],  loc=4)
        axis_signal.set_title('Signal Difference Plot')

        p_bckgrd = self.datapoint_plot(axis_bckgrd, self.bin_centers, bckgrd, xerr=self.bin_widths, yerr=bckgrd_error,
                                       label='(' + self.column_a + ' - ' + self.column_b + ') Background')
        axis_bckgrd.set_xlim((self.xmin, self.xmax))
        axis_bckgrd.set_ylim(((bckgrd-bckgrd_error).min(), (bckgrd+bckgrd_error).max()))
        axis_bckgrd.get_xaxis().set_label_text(self.quantity)
        axis_bckgrd.get_yaxis().set_label_text('Difference')
        axis_bckgrd.legend(handles=[p_bckgrd],  loc=4)
        axis_bckgrd.set_title('Background Difference Plot')
        return (p_signal, p_bckgrd)


def get_data(N, columns):
    n = len(columns) - 1
    xs = numpy.random.normal(0, size=(N, n))
    xb = numpy.random.normal(1, size=(N, n))
    ys = numpy.zeros(N)
    yb = numpy.ones(N)
    data = pandas.DataFrame(numpy.c_[numpy.r_[xs, xb], numpy.r_[ys, yb]], columns=columns)
    return data.reindex(numpy.random.permutation(data.index))


def write(figure, filename):
    from matplotlib.backends.backend_agg import FigureCanvasAgg as FigureCanvas
    canvas = FigureCanvas(figure)
    canvas.print_figure(filename, dpi=100)


if __name__ == '__main__':

    # Set nice searborn settings
    seaborn.set(font_scale=3)
    seaborn.set_style('whitegrid')

    # Do standard plots using plotter
    figure = matplotlib.figure.Figure(figsize=(20, 20))
    figure.set_tight_layout(True)
    gs = matplotlib.gridspec.GridSpec(3, 1)

    data = get_data(10000, columns=['FastBDT', 'NeuroBayes', 'isSignal'])
    plotter = Plotter(data, columns=['FastBDT', 'NeuroBayes'], target='isSignal')
    plotter.roc_plot(figure.add_subplot(gs[0, :]))
    plotter.diag_plot(figure.add_subplot(gs[1, :]))
    plotter.dist_plot(figure.add_subplot(gs[2, :]))

    write(figure, "comparison.png")

    # Do overtraining plot using comparison plotter
    figure = matplotlib.figure.Figure(figsize=(20, 20))
    figure.set_tight_layout(True)
    gs = matplotlib.gridspec.GridSpec(5, 4)

    train_data = get_data(5000, columns=['Train', 'isSignal'])
    test_data = get_data(5000, columns=['Test', 'isSignal'])

    comparison_plotter = ComparisonPlotter(train_data, test_data)
    comparison_plotter.dist_plot(figure.add_subplot(gs[:3, :]))
    comparison_plotter.diff_plot(figure.add_subplot(gs[3, :]), figure.add_subplot(gs[4, :]))

    write(figure, "overtraining.png")
