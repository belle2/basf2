# basic imports
import math
import os
import glob
import sys


def create_image_matrix(imidzes, package, size):

    from PIL import Image
    # prepare an empty list of images
    images = []
    # from our form, we get image width in %, so we need to convert the number to integer
    n = math.floor(100/size)
    # initialize all images in a package
    imgs = sorted(glob.glob("./plots/" + package + "/*.png"))
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
            i += 1

    # and all images are exactly the same
    if len(element_size_x) >= 2 or len(element_size_y) >= 2:
        print "Error: Images selected have different dimensions."
        return 0
    # get the final width and height of the image
    k = int(math.ceil(len(images)/n))
    # but first, make sure we are not leaving any empty space
    if n > len(images):
        N = len(images)
    else:
        N = n
    element_size.append(N*max(element_size_x))
    element_size.append(k*max(element_size_y))
    es = [max(element_size_x), max(element_size_y)]
    # add a little border(1%)
    for i in [0, 1]:
        element_size[i] = int(math.ceil(1.01*element_size[i])+math.ceil(0.01*es[i]))
    i = 0
    # create a new white image, our canvas
    img = Image.new('RGB', element_size, "white")
    # loop over all images and paste them to our canvas
    for element in images:
        # get the x coordinate of an image
        # if we are dealing with the first row, as x coordinate, we want the border only
        if i % n == 0:
            x = int(math.ceil(max(element_size_x)*0.01))
        else:
            # if not, just add the new image width to the x coordinate
            x += max(element_size_x)+int(math.ceil(max(element_size_x)*0.01))
        # now this is a little aesthetic workaround, so the first column
        # is aligned with the other columns and it is not "stepped down"
        if not i or not i % n:
            j = i+1
        else:
            j = i
        # get the y coordinate of the image
        y = int(math.floor(i/n)*max(element_size_y)+math.ceil(j/n)*max(element_size_y)*0.01)
        # since Image.paste gets a 4-tuple(it could accept a 2-tuple, but it is safer to hand it a 4-tuple),
        # we need also coordinates of lower corner and right corner of the image
        v = x+int(max(element_size_x))
        w = y+int(max(element_size_y))
        # don't forget to increment the i variable
        i += 1
        # finally, paste the image onto our canvas
        img.paste(element, (x, y, v, w))
    # trial debug line. Image.show() prints the image into temp folder and opens it
    # with a system image viewer right away, does not save the image.
    # img.show()
    # if the temp file exists, delete it
    if os.path.exists("./plots/matrix.png"):
        os.remove("./plots/matrix.png")
    # save image to a temp file
    img.save("./plots/matrix.png", "PNG")
    # print "Image successfully saved."
    # return true as a sign of successfull image creation
    return 1
# trial debug line
# create_image_matrix(['h1', 'h0', 'h5', 'h3', 'h4', 'h6', 'h2', 'h7'], "current_reference/background", 20)


def merge_multiple_plots(package, pdfs):
    from PyPDF2 import PdfFileReader, PdfFileWriter
    folder = "./plots/" + package + "/*.pdf"
    files = sorted(glob.glob(folder))
    new = "./plots/merged_plots.pdf"
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
    except:
        return 0
