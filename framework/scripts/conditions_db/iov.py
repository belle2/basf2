#!/usr/bin/env python3

"""
conditions_db.iov
-----------------

This module contains classes to work with validity intervals. There's a class
for a single interval, `IntervalOfValidity` and a class to manage a set of
validities, `IoVSet`, which can be used to manipulate iov ranges
"""

import math
from itertools import product


class IntervalOfValidity:
    """
    Interval of validity class to support set operations like union and
    intersection.

    An interval of validity is a set of runs for which something is valid. An
    IntervalOfValidity consists of a `first` valid run and a `final` valid run.

    Warning:
        The `final` run is inclusive so the the validity is including the final run.

    Each run is identified by a experiment number and a run number. Accessing
    `first` or `final` will return a tuple ``(experiment, run)`` but the
    elements can also be accessed separately with `first_exp`, `first_exp`,
    `final_exp` and `final_run`.

    For `final` there's a special case where either the run or both, the run and
    the experiment number are infinite. This means the validity extends to all
    values. If only the run number is infinite then it's valid for all further
    runs in this experiment. If both are infinite the validity extends to everything.

    For simplicity ``-1`` can be passed in instead of infinity when creating objects.
    """
    def __init__(self, *iov):
        """Create a new object.

        It can be either instantiated by providing four values or one tuple/list
        with four values for first_exp, first_run, final_exp, final_run
        """
        if len(iov) == 1 and isinstance(iov[0], (list, tuple)):
            iov = iov[0]
        if len(iov) != 4:
            raise ValueError("A iov should have four values")
        #: tuple with the first valid exp, run
        self.__first = tuple(iov[:2])
        #: tuple with the final valid exp, run
        self.__final = tuple(math.inf if x == -1 else x for x in iov[2:])
        if math.isinf(self.__final[0]) and not math.isinf(self.__final[1]):
            raise ValueError(f"Unlimited final experiment but not unlimited run: {self}")
        if self.__first[0] > self.__final[0]:
            raise ValueError(f"First exp larger than final exp: {self}")
        if self.__first[0] == self.__final[0] and self.__first[1] > self.__final[1]:
            raise ValueError(f"First run larger than final run: {self}")
        if self.__first[0] < 0 or self.__first[1] < 0:
            raise ValueError(f"Negative first exp or run: {self}")

    @staticmethod
    def always():
        """Return an iov that is valid everywhere

        >>> IntervalOfValidity.always()
        (0, 0, inf, inf)
        """
        return IntervalOfValidity(0, 0, -1, -1)

    @property
    def first(self):
        """Return the first valid experiment,run"""
        return self.__first

    @property
    def first_exp(self):
        """Return the first valid experiment"""
        return self.__first[0]

    @property
    def first_run(self):
        """Return the first valid run"""
        return self.__first[1]

    @property
    def final(self):
        """Return the final valid experiment,run"""
        return self.__final

    @property
    def final_exp(self):
        """Return the final valid experiment"""
        return self.__final[0]

    @property
    def final_run(self):
        """Return the final valid run"""
        return self.__final[1]

    def __repr__(self):
        """Return a printable representation"""
        return str(self.__first + self.__final)

    def __eq__(self, other):
        """Check for equality"""
        if not isinstance(other, IntervalOfValidity):
            return NotImplemented
        return (self.__first, self.__final) == (other.__first, other.__final)

    def __lt__(self, other):
        """Sort by run values"""
        if not isinstance(other, IntervalOfValidity):
            return NotImplemented
        return (self.__first, self.__final) < (other.__first, other.__final)

    def __and__(self, other):
        """Intersection between iovs. Will return None if the payloads don't overlap"""
        if not isinstance(other, IntervalOfValidity):
            return NotImplemented
        return self.intersect(other)

    def __or__(self, other):
        """Union between iovs. Will return None if the iovs don't overlap or
        connect to each other"""
        if not isinstance(other, IntervalOfValidity):
            return NotImplemented
        return self.union(other, False)

    def __sub__(self, other):
        """Difference between iovs. Will return None if nothing is left over"""
        if not isinstance(other, IntervalOfValidity):
            return NotImplemented
        return self.subtract(other)

    def __hash__(self):
        """Make object hashable"""
        return hash((self.__first, self.__final))

    def subtract(self, other):
        """Return a new iov with the validity of the other removed.
        Will return None if everything is removed.

        Warning:
            If the other iov is in the middle of the validity we will return a
            tuple of two new iovs

                >>> iov1 = IntervalOfValidity(0,0,10,-1)
                >>> iov2 = IntervalOfValidity(5,0,5,-1)
                >>> iov1 - iov2
                ((0, 0, 4, inf), (6, 0, 10, inf))
        """
        if other.first <= self.first and other.final >= self.final:
            # full overlap
            return None
        if other.first > self.first and other.final < self.final:
            # the one we want to remove is in the middle, return a pair of iovs
            # by subtracting two extended once
            iov1 = self.subtract(IntervalOfValidity(other.first + (-1, -1)))
            iov2 = self.subtract(IntervalOfValidity((0, 0) + other.final))
            return (iov1, iov2)
        if other.first <= self.final and other.final >= self.first:
            # one sided overlap, figure out which side and calculate the remainder
            if self.first < other.first:
                end_run = other.first_run - 1
                end_exp = other.first_exp if end_run >= 0 else other.first_exp - 1
                return IntervalOfValidity(self.first + (end_exp, end_run))
            else:
                start_run = other.final_run + 1
                start_exp = other.final_exp
                if math.isinf(other.final_run):
                    start_exp += 1
                    start_run = 0
                return IntervalOfValidity((start_exp, start_run) + self.final)
        # no overlap so return unchanged
        return self

    def intersect(self, other):
        """Intersection with another iov.

        Will return None if the payloads don't overlap

            >>> iov1 = IntervalOfValidity(1,0,2,5)
            >>> iov2 = IntervalOfValidity(2,0,2,-1)
            >>> iov3 = IntervalOfValidity(2,10,5,-1)
            >>> iov1.intersect(iov2)
            (2, 0, 2, 5)
            >>> iov2.intersect(iov3)
            (2, 10, 2, inf)
            >>> iov3.intersect(iov1) is None
            True

        """
        if other.first <= self.final and other.final >= self.first:
            return IntervalOfValidity(*(max(self.first, other.first) + min(self.final, other.final)))
        return None

    def union(self, other, allow_startone=False):
        """
        Return the union with another iov.

            >>> iov1 = IntervalOfValidity(1,0,1,-1)
            >>> iov2 = IntervalOfValidity(2,0,2,-1)
            >>> iov3 = IntervalOfValidity(2,10,5,-1)
            >>> iov1.union(iov2)
            (1, 0, 2, inf)
            >>> iov2.union(iov3)
            (2, 0, 5, inf)
            >>> iov3.union(iov1) is None
            True

        Warning:
           This method will return None if the iovs don't overlap or connect to
           each other as no union can be formed.

        Parameters:
            other (IntervalOfValidity): IoV to calculate the union with
            allow_startone (bool): If True we will consider run 0 and run 1 the
                first run in an experiment. This means that if one of the iovs has
                un unlimited final run it can be joined with the other iov if the
                experiment number increases and the iov starts at run 0 and 1. If
                this is False just run 0 is considered the next run.

                    >>> iov1 = IntervalOfValidity(0,0,0,-1)
                    >>> iov2 = IntervalOfValidity(1,1,1,-1)
                    >>> iov1.union(iov2, False) is None
                    True
                    >>> iov1.union(iov2, True)
                    (0, 0, 1, inf)

        """
        # check the trivial case of overlapping
        if other.first <= self.final and other.final >= self.first:
            return IntervalOfValidity(min(self.first, other.first) + max(self.final, other.final))
        # ok, let's do the less simple case where they don't overlap but join directly
        for i1, i2 in (self, other), (other, self):
            if (i1.first == (i2.final_exp, i2.final_run + 1) or
                (math.isinf(i2.final_run) and (i1.first_exp == i2.final_exp + 1) and
                 (i1.first_run == 0 or allow_startone and i1.first_run == 1))):
                return IntervalOfValidity(i2.first + i1.final)
        # no union possible: not directly connected and not overlapping
        return None

    def contains(self, exp, run):
        """Check if a run is part of the validtiy"""
        return self.first <= (exp, run) <= self.final

    @property
    def is_open(self):
        """Check whether the iov is valid until infinity"""
        #: Doxygen complains without this string.
        return self.final == (math.inf, math.inf)

    @property
    def tuple(self):
        """Return the iov as a tuple with experiment/run numbers replaced with -1

        This is mostly helpful where infinity is not supported and is how the
        intervals are represented in the database.

            >>> a = IntervalOfValidity.always()
            >>> a
            (0, 0, inf, inf)
            >>> a.tuple
            (0, 0, -1, -1)
        """
        return self.__first + tuple(-1 if math.isinf(x) else x for x in self.__final)


class IoVSet:
    """A set of iovs.

    This class allows to combine iovs into a set. New iovs can be added with
    `add()` and will be combined with existing iovs if possible.

    The final, minimal number of iovs can be obtained with the `iovs` property

        >>> a = IoVSet()
        >>> a.add((0,0,0,2))
        >>> a.add((0,3,0,5))
        >>> a.add((0,8,0,9))
        >>> a
        {(0, 0, 0, 5), (0, 8, 0, 9)}
    """
    def __init__(self, iterable=None, *, allow_overlaps=False, allow_startone=False):
        """Create a new set.

            >>> a = IoVSet([IntervalOfValidity(3,6,3,-1), (0,0,3,5)])
            >>> a
            {(0, 0, 3, inf)}

        Parameters:
            iterable: if not None it should be an iterable of IntervalOfValidity
                objects or anything that can be converted to an IntervalOfValidity.
            allow_overlaps (bool): If False adding which overlaps with any
                existing iov in the set will raise a ValueError.
            allow_startone (bool): If True also join iovs if one covers the
                whole experiment and the next one starts at run 1 in the next
                experiment. If False they will only be joined if the next one
                starts at run 0.
        """
        #: The set of iovs
        self.__iovs = set()
        #: Whether or not we raise an error on overlaps
        self.__allow_overlaps = allow_overlaps
        #: Whether or not run 1 will be also considered the first run when
        # combining iovs between experiments
        self.__allow_startone = allow_startone
        if iterable is not None:
            for element in iterable:
                self.add(element)

    def add(self, iov, allow_overlaps=None):
        """
        Add a new iov to the set.

        The new iov be combined with existing iovs if possible. After the
        operation the set will contain the minimal amount of separate iovs
        possible to represent all added iovs

            >>> a = IoVSet()
            >>> a.add((0, 0, 0, 2))
            >>> a.add((0, 3, 0, 5))
            >>> a.add((0, 8, 0, 9))
            >>> a
            {(0, 0, 0, 5), (0, 8, 0, 9)}
            >>> a.add(IoVSet([(10, 0, 10, 1), (10, 2, 10, -1)]))
            >>> a
            {(0, 0, 0, 5), (0, 8, 0, 9), (10, 0, 10, inf)}

        Be aware, by default it's not possible to add overlapping iovs to the set.
        This can be changed either on construction or per `add` call using
        ``allow_overlap``

            >>> a.add((0, 2, 0, 3))
            Traceback (most recent call last):
                ...
            ValueError: Overlap between (0, 0, 0, 5) and (0, 2, 0, 3)
            >>> a.add((0, 2, 0, 3), allow_overlaps=True)
            >>> a
            {(0, 0, 0, 5), (0, 8, 0, 9), (10, 0, 10, inf)}

        Parameters:
            iov (Union[IoVSet, IntervalOfValidity, tuple(int)]): IoV or
                set of IoVs to add to this set
            allow_overlaps (bool): Can be used to override global overlap setting
                of this set to allow/restrict overlaps for a single insertion
                operation

        Warning:
            This method modifies the set in place
        """
        # check whether we override overlap settings
        if allow_overlaps is None:
            allow_overlaps = self.__allow_overlaps
        # we can add a set to a set :D
        if isinstance(iov, IoVSet):
            for element in iov:
                self.add(element, allow_overlaps)
            return
        # make sure it's actually an IoV, this will raise an error on failure
        if not isinstance(iov, IntervalOfValidity):
            iov = IntervalOfValidity(iov)
        # and now check for all existing iovs ... (but use a copy since we modify the set)
        for existing in list(self.__iovs):
            # if there's an overlap to the new iov
            if (not allow_overlaps) and (existing & iov):
                raise ValueError(f"Overlap between {existing} and {iov}")
            # and if they can be combined to a bigger iov
            combined = existing.union(iov, self.__allow_startone)
            # if we now have a combined iov, remove the one that we were able to
            # combine it with from the existing iovs because we now check
            # against the combined one. Since the only way to add a new iov is
            # this loop we know all previous existing iovs we checked before
            # didn't have a union with this new iov or any other existing iovs
            # so if the just check the remaining iovs against the new combined
            # one we can cascade combine all iovs in one go.
            if combined is not None:
                self.__iovs.remove(existing)
                iov = combined
        # done, we now have a new iov which combines all existing iovs it had an
        # overlap with and we removed the existing iovs so nothing else to do
        # but add the iov back in the list
        self.__iovs.add(iov)

    def remove(self, iov):
        """Remove an iov or a set of iovs from this set

        After this operation the set will not be valid for the given iov or set
        of iovs:

            >>> a = IoVSet()
            >>> a.add((0,0,10,-1))
            >>> a.remove((1,0,1,-1))
            >>> a.remove((5,0,8,5))
            >>> a
            {(0, 0, 0, inf), (2, 0, 4, inf), (8, 6, 10, inf)}
            >>> a.remove(IoVSet([(3,0,3,10), (3,11,3,-1)]))
            >>> a
            {(0, 0, 0, inf), (2, 0, 2, inf), (4, 0, 4, inf), (8, 6, 10, inf)}

        Parameters:
            iov (Union[IoVSet, IntervalOfValidity, tuple(int)]): IoV or
                set of IoVs to remove from this set

        Warning:
            This method modifies the set in place
        """
        # we can remove a set from a set :D
        if isinstance(iov, IoVSet):
            for element in iov:
                self.remove(element)
            return
        # make sure it's actually an IoV, this will raise an error on failure
        if not isinstance(iov, IntervalOfValidity):
            iov = IntervalOfValidity(iov)
        # and subtract the iov from all existing iovs
        for existing in list(self.__iovs):
            delta = existing - iov
            if delta != existing:
                self.__iovs.remove(existing)
                if isinstance(delta, tuple):
                    # got two new iovs, apparently we split the old one
                    for new in delta:
                        self.__iovs.add(new)
                elif delta is not None:
                    self.__iovs.add(delta)

    def intersect(self, iov):
        """Intersect this set with another set and return a new set
        which is valid exactly where both sets have been valid before

            >>> a = IoVSet()
            >>> a.add((0,0,10,-1))
            >>> a.intersect((5,0,20,-1))
            {(5, 0, 10, inf)}
            >>> a.intersect(IoVSet([(0,0,3,-1), (9,0,20,-1)]))
            {(0, 0, 3, inf), (9, 0, 10, inf)}

        Parameters:
            iov (Union[IoVSet, IntervalOfValidity, tuple(int)]): IoV or
                set of IoVs to intersect with this set
        """
        if not isinstance(iov, (IoVSet, IntervalOfValidity)):
            iov = IntervalOfValidity(iov)
        if isinstance(iov, IntervalOfValidity):
            iov = IoVSet([iov])

        # ok for all combinations a,b from set1 and set2 check the intersection
        # and if not empty add to the result
        result = IoVSet()
        for a, b in product(self.iovs, iov.iovs):
            c = a & b
            if c:
                result.add(c)
        return result

    def contains(self, iov):
        """
        Check if an iov is fully covered by the set

            >>> a = IoVSet([(0,0,2,-1), (5,0,5,-1)])
            >>> a.contains((0,0,1,-1))
            True
            >>> a.contains(IntervalOfValidity(0,0,3,2))
            False
            >>> a.contains(IoVSet([(0,1,1,23), (5,0,5,23)]))
            True
            >>> a.contains(IoVSet([(0,1,1,23), (5,0,6,23)]))
            False
            >>> a.contains((3,0,4,-1))
            False

        Parameters:
            iov (Union[IoVSet, IntervalOfValidity, tuple(int)]): IoV or
                set of IoVs to be checked

        Returns:
            True if the full iov or all the iovs in the given set are fully
            present in this set
        """
        # check if the whole set is in this set: all iovs need to be in here
        if isinstance(iov, IoVSet):
            return all(e in self for e in iov)
        # make sure it's actually an IoV, this will raise an error on failure
        if not isinstance(iov, IntervalOfValidity):
            iov = IntervalOfValidity(iov)
        # and then check all iovs in the set if they cover it
        for existing in self.__iovs:
            if iov - existing is None:
                return True
        return False

    def overlaps(self, iov):
        """Check if the given iov overlaps with this set.

        In contrast to `contains` this doesn't require the given iov to be fully
        covered. It's enough if the any run covered by the iov is also covered
        by this set.

            >>> a = IoVSet([(0,0,2,-1), (5,0,5,-1)])
            >>> a.overlaps((0,0,1,-1))
            True
            >>> a.overlaps(IntervalOfValidity(0,0,3,2))
            True
            >>> a.overlaps(IoVSet([(0,1,1,23), (5,0,5,23)]))
            True
            >>> a.overlaps(IoVSet([(0,1,1,23), (5,0,6,23)]))
            True
            >>> a.overlaps((3,0,4,-1))
            False

        Parameters:
            iov (Union[IoVSet, IntervalOfValidity, tuple(int)]): IoV or
                set of IoVs to be checked

        Returns:
            True if the iov or any of the iovs in the given set overlap with any
            iov in this set
        """
        if not isinstance(iov, (IoVSet, IntervalOfValidity)):
            iov = IntervalOfValidity(iov)
        if isinstance(iov, IntervalOfValidity):
            iov = IoVSet([iov])

        for a, b in product(self.iovs, iov.iovs):
            c = a & b
            if c:
                return True
        return False

    def copy(self):
        """Return a copy of this set"""
        copy = IoVSet(allow_overlaps=self.__allow_overlaps, allow_startone=self.__allow_startone)
        copy.__iovs = set(self.__iovs)
        return copy

    def clear(self):
        """Clear all iovs from this set"""
        self.__iovs = {}

    @property
    def iovs(self):
        """Return the set of valid iovs"""
        return self.__iovs

    @property
    def first(self):
        """Return the first run covered by this iov set

        >>> a = IoVSet([(3,0,3,10), (10,11,10,23), (0,0,2,-1), (5,0,5,-1)])
        >>> a.first
        (0, 0)
        """
        if not self.__iovs:
            return None
        return min(self.iovs).first

    @property
    def final(self):
        """Return the final run covered by this iov set

        >>> a = IoVSet([(3,0,3,10), (10,11,10,23), (0,0,2,-1), (5,0,5,-1)])
        >>> a.final
        (10, 23)
        """
        if not self.__iovs:
            return None
        return max(self.iovs).final

    @property
    def gaps(self):
        """Return the gaps in the set: Any area not covered between the first
        point of validity and the last

        >>> a = IoVSet([(0,0,2,-1)])
        >>> a.gaps
        {}
        >>> b = IoVSet([(0,0,2,-1), (5,0,5,-1)])
        >>> b.gaps
        {(3, 0, 4, inf)}
        >>> c = IoVSet([(0,0,2,-1), (5,0,5,-1), (10,3,10,6)])
        >>> c.gaps
        {(3, 0, 4, inf), (6, 0, 10, 2)}
        """
        if len(self.__iovs) < 2:
            return IoVSet()

        full_range = IoVSet([self.first + self.final])
        return full_range - self

    def __bool__(self):
        """Return True if the set is not empty


            >>> a = IoVSet()
            >>> a.add((0,0,1,-1))
            >>> bool(a)
            True
            >>> a.clear()
            >>> a
            {}
            >>> bool(a)
            False
        """
        return len(self.__iovs) > 0

    def __contains__(self, iov):
        """Check if an iov is fully covered by the set"""
        return self.contains(iov)

    def __and__(self, other):
        """Return a new set that is the intersection between two sets

            >>> a = IoVSet([(0,0,1,-1)])
            >>> a & (1,0,2,-1)
            {(1, 0, 1, inf)}
        """
        return self.intersect(other)

    def __or__(self, other):
        """
        Return a new set that is the combination of two sets: The new set will
        be valid everywhere any of the two sets were valid.

        No check for overlaps will be performed but the result will inherit the
        settings for further additions from the first set

            >>> a = IoVSet([(0,0,1,-1)])
            >>> a | (1,0,2,-1)
            {(0, 0, 2, inf)}
            >>> a | (3,0,3,-1)
            {(0, 0, 1, inf), (3, 0, 3, inf)}
        """
        copy = self.copy()
        copy.add(other, allow_overlaps=True)
        return copy

    def __sub__(self, other):
        """
        Return a new set which is only valid for where a is valid but not b.

        See `remove` but this will not modify the set in place

            >>> a = IoVSet([(0,0,-1,-1)])
            >>> a - (1,0,2,-1)
            {(0, 0, 0, inf), (3, 0, inf, inf)}
            >>> a - (0,0,3,-1) - (10,0,-1,-1)
            {(4, 0, 9, inf)}
            >>> IoVSet([(0,0,1,-1)]) - (2,0,2,-1)
            {(0, 0, 1, inf)}
        """
        copy = self.copy()
        copy.remove(other)
        return copy

    def __iter__(self):
        """Loop over the set of iovs"""
        return iter(self.__iovs)

    def __len__(self):
        """Return the number of validity intervals in this set"""
        return len(self.__iovs)

    def __repr__(self):
        """Return a printable representation"""
        return '{' + ', '.join(str(e) for e in sorted(self.__iovs)) + '}'
