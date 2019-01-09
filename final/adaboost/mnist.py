import os
import struct
import numpy as np
import cv2
from tempfile import TemporaryFile

TRAIN_IMG = 'train-images.idx3-ubyte'
TRAIN_LABEL = 'train-labels.idx1-ubyte'
TEST_IMG = 't10k-images.idx3-ubyte'
TEST_LABEL = 't10k-labels.idx1-ubyte'

def read(path):
    if path is 'train':
        lblpath = TRAIN_LABEL
        imgpath = TRAIN_IMG
    else:
        lblpath = TEST_LABEL
        imgpath = TEST_IMG

    with open(os.path.join('.', path, lblpath), 'rb') as flbl:
        flbl.read(8)
        lbl = np.fromfile(flbl, dtype=np.int8)

    with open(os.path.join('.', path, imgpath), 'rb') as fimg:
        rows, cols = struct.unpack(">IIII", fimg.read(16))[2:]
        img = np.fromfile(fimg, dtype=np.uint8).reshape(len(lbl), rows, cols)

    return img, lbl

if __name__ == '__main__':
    img, lbl = read('train')
    print(img[0])