#!/usr/bin/env -S python3 -u
# Do not enable conda's virtualenv for this script

import cv2 as cv
import numpy as np
import sys


# :param prog: has possible values 0, 1, 2, 3, indicates drawing's
#              completeness
def draw(prog: int, width: int = 1920, height: int = 1080) -> np.ndarray:
    # Use white background
    frame = 255 * np.ones((height, width, 3), dtype=np.uint8)
    print(frame.shape)

    if prog <= 1:
        pass

    return frame


# :param prog: has possible values 0, 1.  0 for zju logo, 1 for personal info
def opening(prog: int, width: int = 1920, height: int = 1080) -> np.ndarray:
    logofile = "./stuff/zjulogo.jpg"
    logo = cv.imread(logofile)
    logo_width = logo.shape[0]
    logo_height = logo.shape[1]
    # Use white background
    frame = 255 * np.ones((height, width, 3), dtype=np.uint8)

    hor_begin = (width - logo_width) // 2
    ver_begin = (height - logo_height) // 2
    print(frame.shape)
    frame[hor_begin:hor_begin + logo_width,
          ver_begin:ver_begin + logo_height] = logo

    return frame


def ending(width: int = 1920, height: int = 1080) -> np.ndarray:
    # Use white background
    frame = 255 * np.ones((height, width, 3), dtype=np.uint8)
    return frame


def consume_key(key: int):
    if key == ord(' '):
        key = cv.waitKey()
    if key == ord('q'):
        cv.destroyAllWindows()
        sys.exit(0)


if __name__ == '__main__':
    # Global configs
    width = 1920
    height = 1080
    vidlen = 10
    fps = 24
    name = "kidsdrawing"

    # save file configs
    savfile = name + ".mp4"
    fourcc = cv.VideoWriter_fourcc(*"mp4v")
    # Video writer
    out = cv.VideoWriter(savfile, fourcc, fps, (width, height))

    ############## Play by frame and save as video file ##############
    # Opening
    for fid in range(int(1.5 * fps)):
        showimg = opening(0)
        cv.imshow(name, showimg)
        out.write(showimg)
        key = cv.waitKey(int(1000 / fps))
        consume_key(key)
    # Kid's drawing
    for fid in range(int(vidlen * fps)):
        if fid:
            showimg = draw(0)
        cv.imshow(name, showimg)
        out.write(showimg)
        key = cv.waitKey(int(1000 / fps))
        consume_key(key)

    # Ending
    for fid in range(1.5 * fps):
        showimg = ending()
        cv.imshow(name, showimg)
        out.write(showimg)
        key = cv.waitKey(int(1000 / fps))
        consume_key(key)

# Author: Blurgy <gy@blurgy.xyz>
# Date:   Nov 30 2020, 17:54 [CST]