#!/usr/env python3

import cv2
import numpy as np


def cv2rgb565(cv_image):
    """
    convert from opencv image to rgb565 format
    """
    rgb_image = cv2.cvtColor(cv_image, cv2.COLOR_BGR2RGB)
    red_5 = (rgb_image[..., 0] >> 3).astype(np.uint16) << 11
    green_6 = (rgb_image[..., 1] >> 2).astype(np.uint16) << 5
    blue_5 = (rgb_image[..., 2] >> 3).astype(np.uint16)
    rgb565_image = red_5 | green_6 | blue_5
    return rgb565_image


def save_as_hex(filename, array2d):
    """
    save rgb565 numpy array as hex format string
    """
    with open(filename, "w") as text_file:
        for i in array2d:
            for j in i:
                text_file.write(format(j, "#04x") + ", ")
            text_file.write("\n")


if __name__ == "__main__":
    DATA_ROOT = "../datas/"
    image = cv2.imread(DATA_ROOT + "logo.png")
    rgb565 = cv2rgb565(image)
    save_as_hex(DATA_ROOT + "logo.txt", rgb565)
