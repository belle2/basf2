#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import math
import os
import glob
import sys
import time


def create_image_matrix(imidzes, package, size):
    """!
    A function that creates the image matrix as .png image
    """

    from PIL import Image

    # prepare an empty list of images
    images = []

    # from our form, we get image width in %,
    # so we need to convert the number to integer
    n = math.floor(100 / size)

    # initialize all images in a package
    imgs = sorted(glob.glob("./plots/" + package + "/*.png"))

    # this is extremely useful for debugging, but not as useful as
    # it could be for running the script, so it is commented out
    if imidzes == "everything":
        imidzes = []
        for image in imgs:
            imidzes.append(image.split("/")[-1][:-4])

    # error handling
    if not imgs:
        print("Something went wrong. Exitting.")
        return 0
    if not imidzes:
        print("Error: No images input.")
        return 0
    if not package:
        print("Error: No package input.")
        return 0
    if not size:
        print("Error: No size input.")
        return 0
    if not os.path.isdir("./plots/" + package):
        print("Error: The requested directory does not exist.")
        return 0

    # initialize variables
    im_xs = []
    im_ys = []
    sum_y = 0
    element_size = []

    # select only images we need
    for imidz in sorted(imgs):
        item = imidz.split("/")[-1].strip()
        if item[:-4] in imidzes:
            imidz = Image.open(imidz)
            images.append(imidz)
            im_xs.append(imidz.size[0])
            im_ys.append(imidz.size[1])

    # get the size of images, we need min of width and min and max of height
    element_size_y = set(im_ys)
    element_size_x = set(im_xs)
    min_x = min(im_xs)
    max_y = max(element_size_y)
    min_y = min(element_size_y)
    sizes = (im_xs, im_ys)

    # first, make sure we are not leaving any empty space in x-coord
    if n > len(images):
        N = len(images)
    else:
        N = n

    # First, the x-coordinate. This is actually easy, because we want
    # every image to have the same width, so we don't care if they have
    # different heights, if we put a minimum width to paste() and the
    # image is of a bigger width, paste() should scale it down in x-coord
    element_size.append(N * min_x)
    es = [min_x, max_y]
    k = int(math.ceil(len(images) / N))
    # But, obviously, we care about image height.
    if len(element_size_y) >= 2:

        element_size.append(int(k * math.ceil(1.01 * max_y) +
                                math.ceil(0.01 * es[1])))
        # Add a little border to the x-coordinate
        element_size[0] = int(math.ceil(1.01 * element_size[0]) +
                              math.ceil(0.01 * es[0]))
    else:

        # get the final width and height of the image
        element_size.append(k * max_y)

        # add a little border(1%)
        for i in [0, 1]:
            element_size[i] = int(math.ceil(1.01 * element_size[i]) +
                                  math.ceil(0.01 * es[i]))

    # create a new white image, our canvas
    img = Image.new('RGB', element_size, "white")

    # loop over all images and paste them to our canvas
    border = (int(math.ceil(min_x * 0.01)), int(math.ceil(min_y * 0.01)))
    if len(element_size_y) >= 2:

        # If we do have images of diferent height, we will split the image
        # into boxes, first we define elementary box width and height
        im_sx = int(math.ceil(min_x * 1.01))
        im_sy = int(math.ceil(min_y * 1.01))
        vacant_spaces = []
        # Then we split the image
        for i in range(int(element_size[1] / im_sy)):
            for j in range(int(N)):
                vacant_spaces.append((j * im_sx,
                                      i * im_sy,
                                      (j + 1) * im_sx,
                                      (i + 1) * im_sy))

        # We want them sorted according to y-coordinate(in rows)
        vacant_spaces = sorted(vacant_spaces, key=lambda x: x[1])

        # create a dictionary of removed elementary boxes
        removed = {}

        # Now let's loop over all boxes and put images into them.
        for i, vacant_space in enumerate(vacant_spaces):
            # We have more boxes than images, so everything is in try-except
            try:
                # We define our image box, the area of an actual image we
                # want to paste it in
                box = (vacant_space[0] + border[0],
                       vacant_space[1] + border[1],
                       vacant_space[0] + border[0] + min_x,
                       vacant_space[1] + border[1] + images[i].size[1])
                # If the image height is bigger than one elementary box,
                # loop over all vacant boxes and delete boxes that are
                # occupied by this image and save the height of all removed
                # elementary boxes to dictionary "removed"
                removed[box[3]] = []
                if box[3] > vacant_space[3]:
                    for space in vacant_spaces:
                        if (box[3] >= space[3] and
                                vacant_space[0] == space[0] and
                                vacant_space[1] < space[1]):
                            removed[box[3]].append(space[3])
                            vacant_spaces.remove(space)
                if not removed[box[3]]:
                    removed[box[3]].append(vacant_space[3])
            except IndexError:
                continue
        # And now loop over all images and paste them, also we need the
        # true height of an image, so we can crop it so we don't leave any
        # empty space at the end of the image. The true height is in w variable
        height = []
        box = []
        for i, vacant_space in enumerate(vacant_spaces):
            try:
                box = [vacant_space[0] + border[0],
                       vacant_space[1] + border[1],
                       vacant_space[0] + border[0] + min_x,
                       vacant_space[1] + border[1] + images[i].size[1]]
                if box[3] > vacant_space[3]:
                    box[3] = max(removed[box[3]])
                resize = (box[2] - box[0], box[3] - box[1])

                # if we have a different size of an image than the box size
                # we want to paste it in, resize it
                if images[i].size != resize:
                    images[i] = images[i].resize(resize)
                img.paste(images[i], box)
                height.append(box[3])
            except IndexError:
                continue
        w = max(height)
    else:
        x = int(math.ceil(es[0] * 0.01))
        y = int(math.ceil(es[1] * 0.01))
        for i, element in enumerate(images):
            # get the x coordinate of an image
            # if we are dealing with the first row, as x coordinate,
            # we want the border only
            if i % n == 0:
                x = int(math.ceil(es[0] * 0.01))
            else:
                # if not, just add the new image width to the x coordinate
                x += es[0] + int(math.ceil(es[0] * 0.01))

            # now this is a little aesthetic workaround, so the first column
            # is aligned with the other columns and it is not "stepped down"
            if not i or not i % n:
                j = i + 1
            else:
                j = i

            # get the y coordinate of the image
            y = int(math.floor(i / n) * es[1] + math.ceil(j / n) * es[1] * 0.01)

            # since Image.paste gets a 4-tuple(it could accept a 2-tuple,
            # but it is safer to hand it a 4-tuple),
            # we need also coordinates of lower corner
            # and right corner of the image
            v = x + int(es[0])
            w = y + int(es[1])
            # If we have different widths, resize images
            if len(element_size_x) > 1:
                element = element.resize((v - x, w - y))

            # finally, paste the image onto our canvas
            img.paste(element, (x, y, v, w))

    # Crop the created image to fit pasted images
    crop_coords = (0, 0, element_size[0], w)
    img = img.crop(crop_coords)

    # if the temp file exists, delete it
    if os.path.exists("./plots/matrix.png"):
        os.remove("./plots/matrix.png")

    # save image to a temp file
    img.save("./plots/matrix.png", "PNG")
    print("Image successfully saved.")

    # return true as a sign of successfull image creation
    return 1
# trial debug line
# create_image_matrix("everything", "current_reference/test1", 10)


def merge_multiple_plots(package, pdfs):
    """!
    A function that merges plots into a single .pdf file
    """

    # Crucial import
    from PyPDF2 import PdfFileReader, PdfFileWriter

    # Set the folder where all of the PDFs are
    folder = "./plots/" + package + "/*.pdf"

    # Get all plots in PDF
    files = sorted(glob.glob(folder))

    # Output file
    new = "./plots/merged_plots.pdf"

    # Try to merge them
    try:
        output = PdfFileWriter()
        output.setPageLayout("/TwoColumnLeft")
        for element in files:
            pdf = element.split("/")[-1].strip()
            pdf = pdf.split(".")[0]
            if pdf in pdfs:
                element = PdfFileReader(file(element, "rb"))
                output.addPage(element.getPage(0))
        output.write(file(new, "wb"))
        return 1

    # If we can't merge them, return false
    except BaseException:
        return 0
