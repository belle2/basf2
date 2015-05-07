# -*- coding: utf-8 -*-

import math
import os
import glob
import sys


def create_image_matrix(imidzes, package, size):

    """!
    A function that creates the image matrix as .png image
    """

    from PIL import Image

    # prepare an empty list of images
    images = []

    # from our form, we get image width in %,
    # so we need to convert the number to integer
    n = math.floor(100/size)

    # initialize all images in a package
    imgs = sorted(glob.glob("./plots/" + package + "/*.png"))

    # this is extremely useful for debugging, but not as useful as
    # it could be for running the script, so it is commented out
    """
    if imidzes == "everything":
        imidzes = []
        for image in imgs:
            imidzes.append(image.split("/")[-1][:-4])
    """

    # error handling
    if not imgs:
        print "Something went wrong. Exitting."
        return 0
    if not imidzes:
        print "Error: No images input."
        return 0
    if not package:
        print "Error: No package input."
        return 0
    if not size:
        print "Error: No size input."
        return 0
    if not os.path.isdir("./plots/"+package):
        print "Error: The requested directory does not exist."
        return 0

    # initialize variables
    element_size_x = set()
    element_size_y = set()
    size_y_arr = []
    sum_y = 0
    element_size = []
    i = 0

    # select only images we need
    for imidz in imgs:
        item = imidz.split("/")[-1].strip()
        if item[:-4] in imidzes:
            imidz = Image.open(imidz)
            images.append(imidz)
            element_size_x.add(imidz.size[0])
            element_size_y.add(imidz.size[1])
            size_y_arr.append(imidz.size[1])
            i += 1

    # first, make sure we are not leaving any empty space in x-coord
    if n > len(images):
        N = len(images)
    else:
        N = n

    # First, the x-coordinate. This is actually easy, because we want
    # every image to have the same width, so we don't care if they have
    # different heights, if we put a minimum width to paste() and the
    # image is of a bigger width, paste() should scale it down in x-coord
    element_size.append(N*min(element_size_x))
    es = [min(element_size_x), max(element_size_y)]

    # But, obviously, we care about image height.
    if len(element_size_y) >= 2:
        rowheight = []
        imageheight = 0
        row = set()
        i = 0

        # Biggest Image In The Row _ X-coordinate
        # Does not contain the precise x-coordinate, but rather a number
        # that tells us, in which column the image is situated
        # the BIITR is defined as an image that is as big or bigger than
        # 2 ordinary images
        biitr_x = []

        # Get the image height
        # We need to look at every image in the row and compare them to
        # see which one is the biggest one
        for size_y in size_y_arr:
            row.add(size_y)

            # First, get the BIIITR number
            if size_y < max(row) and not (i % N) == (N - 1):
                biitr_x.append(i % N)
            elif (i % N) == (N - 1):
                biitr_x.append(0)

            # Then, get a height of the row
            if (i % N) == (N - 1) and i + 1 == len(images):
                rowheight.append(int(math.ceil(max(row)*1.02)))
            elif i + 1 == len(imidzes):
                rowheight.append(int(math.ceil(max(row)*1.02)))
            elif (i % N) == (N - 1):
                rowheight.append(int(math.ceil(max(row)*1.01)))
                row = set()
            i += 1
        # Finally, get the image height as a sum of all row heights
        for y in rowheight:
            imageheight += y
        # Now we can append the image height to the list we are putting
        # into image.create()
        element_size.append(int(imageheight))

        # Add a little border to the x-coordinate
        element_size[0] = int(math.ceil(1.01*element_size[0]) +
                              math.ceil(0.01*es[0]))

        # And log what is a normal image height
        normal_element_y = int(min(element_size_y))
    else:

        # get the final width and height of the image
        k = int(math.ceil(len(images)/n))
        element_size.append(k*max(element_size_y))

        # add a little border(1%)
        for i in [0, 1]:
            element_size[i] = int(math.ceil(1.01*element_size[i]) +
                                  math.ceil(0.01*es[i]))

    # create a new white image, our canvas
    img = Image.new('RGB', element_size, "white")

    i = 0
    rowno = 0
    x = int(math.ceil(es[0]*0.01))
    y = int(math.ceil(es[1]*0.01))
    row_start = int(math.ceil(es[0]*0.1))
    # loop over all images and paste them to our canvas
    for element in images:

        # If we are dealing with images with different heights
        if len(element_size_y) >= 2:
            # If the image we are trying to put there is in the end of the row
            if (x + int(math.ceil(es[0]*1.01))) >= element_size[0]:

                # if we do have the biggest image in the row, save its x-coord
                # because we want to start the next row there
                # I don't suppose there will be bigger images than 3 ordinary
                # images, I guess that if we had bigger images, we could not
                # orient in what they are showing anyway
                if rowheight[rowno] >= 2*min(rowheight) and biitr_x[rowno]:
                    row_start = int((biitr_x[rowno])*int(math.ceil(es[0]*1.01)))
                else:
                    row_start = int(math.ceil(es[0]*0.01))
                y += int(math.ceil(images[i-1].size[1]*1.01))
                rowno += 1

                # Reset the x-coord
                x = row_start
            # Just a fancy way to write else
            elif i:
                x += int(math.ceil(es[0]*1.01))

            # And create our 4-tuple
            v = x+int(es[0])
            w = y+int(element.size[1])

        else:
            # get the x coordinate of an image
            # if we are dealing with the first row, as x coordinate,
            # we want the border only
            if i % n == 0:
                x = int(math.ceil(es[0]*0.01))
            else:
                # if not, just add the new image width to the x coordinate
                x += es[0]+int(math.ceil(es[0]*0.01))

            # now this is a little aesthetic workaround, so the first column
            # is aligned with the other columns and it is not "stepped down"
            if not i or not i % n:
                j = i+1
            else:
                j = i

            # get the y coordinate of the image
            y = int(math.floor(i/n)*es[1]+math.ceil(j/n)*es[1]*0.01)

            # since Image.paste gets a 4-tuple(it could accept a 2-tuple,
            # but it is safer to hand it a 4-tuple),
            # we need also coordinates of lower corner and right corner of the image
            v = x+int(es[0])
            w = y+int(es[1])

        # don't forget to increment the i variable
        i += 1

        # finally, paste the image onto our canvas
        img.paste(element, (x, y, v, w))

    # Crop the created image to fit pasted images
    crop_coords = (0, 0, element_size[0], w)
    img = img.crop(crop_coords)

    # trial debug line. Image.show() prints the image into temp folder and opens it
    # with a system image viewer right away, does not save the image.
    # img.show()

    # if the temp file exists, delete it
    if os.path.exists("./plots/matrix.png"):
        os.remove("./plots/matrix.png")

    # save image to a temp file
    img.save("./plots/matrix.png", "PNG")
    print "Image successfully saved."

    # return true as a sign of successfull image creation
    return 1
# trial debug line
# create_image_matrix("everything", "current_reference/test", 20)


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
    except:
        return 0
