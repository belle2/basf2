from shape_utils import *


class FullDigitalShapeClassifierTrainer(object):
    """Class for training of FullDigitalShapeClassifier instances on track data"""

    def __init__(self, mintracks=2000, coveragecut=0.99, maxdivisions=5):
        # Minimum number of tracks for computing hits
        self.mintracks = mintracks
        # Stop adding hits to shape classifier once cut reached
        self.coveragecut = coveragecut
        # Maximum number hits per digital label
        self.maxdivisions = maxdivisions

    def createShapeClassifier(self, Data, thetaU, thetaV):
        """Returns a new FullDigitalShapeClassifier trained on data from tracks in angle bin at theatU and thetaV"""

        # Instantiate a new classifier
        shape_classifier = FullDigitalShapeClassifier()

        # Total number of tracks for training
        alltracks = Data.shape[0]

        # Missing training data, return empty shape classifier
        if alltracks == 0:
            print('Added fulldigital shape classifier with coverage {:.2f}% on training data sample.'.format(0.0))
            return shape_classifier

        print("Start training shape classifier using {:d} tracks ...".format(alltracks))

        # Reduce clusters to shapes
        shapefunc = np.vectorize(get_shape)
        shapes = shapefunc(Data['f2'])

        # Find unique digital labels in shapes
        dlabelfunc = np.vectorize(get_digital_label)
        dlabels, counts = np.unique(dlabelfunc(shapes), return_counts=True)

        # Sort digital labels to decreasing frequency
        order = counts.argsort()[::-1]
        dlabels = dlabels[order]
        counts = counts[order]
        dprobs = counts.astype(np.float) / alltracks

        # Coverage of classifer on training data
        coverage = 0.0

        # Main loop over digital labels
        for i, dlabel in enumerate(dlabels):

            # Turn dlabel into proper string
            dlabel = str(dlabel)

            # Stop once number tracks to small
            if counts[i] < self.mintracks:
                break

            # Stop once target coverage is reached
            if coverage > self.coveragecut:
                break

            # Update the coverage
            coverage += dprobs[i]

            # Find subset of data for given digital label
            it = np.where(dlabelfunc(shapes) == dlabel)
            DData = Data[it]

            # Now, we try to find a best subdivision of the digital label
            # into smaller labels with improved spatial resolution. This
            # is done by computing a feature from cluster signals and
            # feeding it into a classifier. The classifier is just a
            # binning based on K percentiles computed from the sample of
            # features.

            # Compute percentiles and hitmap
            percentiles, hitmap = self.subdivide_data(DData, dlabel, thetaU, thetaV, alltracks)

            # Store results in shape classifier
            shape_classifier.setPercentiles(dlabel, percentiles)
            shape_classifier.setHitMap(dlabel, hitmap)

        print('Added fulldigital shape classifier with coverage {:.2f}% on training data sample.'.format(100 * coverage))

        return shape_classifier

    def subdivide_data(self, DData, dlabel, thetaU, thetaV, alltracks):
        """Returns hitmap and percentile array from sorted training data """

        # Number of tracks for training
        ntracks = DData.shape[0]

        # F is a feature array.
        F = np.zeros(ntracks, dtype=np.float)

        # This is only temporarily used helper
        tmp_classifier = FullDigitalShapeClassifier()

        for j in range(ntracks):
            # Get shape string
            shape = get_shape(DData[j]['f2'])
            # Compute feature
            F[j] = tmp_classifier.computeFeature(shape, thetaU, thetaV)

        # Train feature classifier
        percentiles, unique_indices, indices = self.train_features(F)

        # Compute array with true track intersections
        intersects = np.vstack((DData['f3'], DData['f4'])).transpose()

        # Dictionary with hits
        hitmap = {}

        for index in unique_indices:
            # Find all intersections of tracks for this label
            label_subset = np.where(indices == index)
            label_intersects = intersects[label_subset, :][0].transpose()
            label_prob = prob = float(label_intersects.shape[1]) / alltracks
            hitmap[index] = label_prob

        return percentiles, hitmap

    def train_features(self, F):
        """Train feature classifier to subdivide features array F
           Returns tuple of percentile array, unique feature indices labels and feature indices
        """

        # Number of features for training
        ntracks = F.shape[0]

        # Maximum number of labels is limited by statistics
        maxlabel = max(int(ntracks / self.mintracks), 1)

        # and limited by the user
        maxlabel = min(maxlabel, self.maxdivisions)

        # Try subdivision of digital label into K smaller labels
        for K in range(maxlabel, 0, -1):

            # Compute array of q values
            qValues = np.arange(0., 100.0, 100.0 / K)

            # Compute percentiles for q values
            percentiles = np.percentile(F, qValues)
            inds = np.digitize(F, percentiles)

            # Count tracks per label
            unique_inds, tracksPerLabel = np.unique(inds, return_counts=True)

            if np.min(tracksPerLabel) >= self.mintracks:
                return percentiles, unique_inds, inds


class FullDigitalShapeClassifier(object):
    """
    The FullDigitalShapeClassifier classifies shapes taking into account
    the full sequence of digits, called the digital shape.
    All shapes mapped to the same label are guaranteed to have the same
    digital shape.
    """

    def __init__(self):
        self.hits_dict = {}
        self.percentile_dict = {}

    def computeFeature(self, shape, thetaU, thetaV):
        """Returns feature scalar """
        feature = 0.0
        entry, exit = get_entry_exit_index(shape, thetaU, thetaV)

        if not entry == exit:
            # Feature is the eta value from entry/exit signals
            picked_signals = pick_signals(shape, indexset=[entry, exit])
            feature = picked_signals[0] / (picked_signals[0] + picked_signals[1])
        else:
            # Single digit shape. Feature is the digit charge
            picked_signals = pick_signals(shape, indexset=[entry])
            feature = picked_signals[0]

        return feature

    def getLabel(self, shape, thetaU, thetaV):
        """Returns label tuple for shape at given angles """
        dlabel = get_digital_label(shape)
        if dlabel in self.getDigitalLabels():
            # Compute feature
            feature = self.computeFeature(shape, thetaU, thetaV)
            # Compute the index
            index = int(np.digitize(feature, self.getPercentiles(dlabel)))
            # Return label
            return dlabel, index
        else:
            return None

    def getLabelString(self, shape, thetaU, thetaV):
        return str(self.getLabel(shape, thetaU, thetaV))

    def getHit(self, label):
        """Returns hit for label """
        return self.hits_dict[label[0]][label[1]]

    def hasHit(self, label):
        """ Returns True if there are valid position corrections available """
        if label is None:
            return False
        if not label[0] in self.hits_dict:
            return False
        if not label[1] in self.getHitMap(label[0]):
            return False
        return True

    def getLabels(self):
        """Returns list of labels """
        labels = []
        for dlabel in self.getDigitalLabels():
            for index in self.getHitMap(dlabel):
                labels.append((dlabel, index))
        return labels

    def getDigitalProb(self, dlabel):
        """ Returns probability of digital label in training sample"""
        prob = 0.0
        for index, hit in self.getHitMap(dlabel).items():
            prob += hit[2]
        return prob

    def setPercentiles(self, dlabel, percentiles):
        """ Set array with percentila bins for digital label"""
        self.percentile_dict[dlabel] = percentiles

    def getPercentiles(self, dlabel):
        """ Returns array with percentila bins for digital label"""
        return self.percentile_dict[dlabel]

    def setHitMap(self, dlabel, hitmap):
        """ Set hits for digital label """
        self.hits_dict[dlabel] = hitmap

    def getHitMap(self, dlabel):
        """Returns  hits for digital labels """
        return self.hits_dict[dlabel]

    def getDigitalLabels(self):
        """Returns list of digital labels in shape classifier"""
        return self.hits_dict.keys()

    def getNumberOfLabels(self, dlabel):
        """Returns number of (sub) labels for given digital label"""
        return self.percentile_dict[dlabel].shape[0]
