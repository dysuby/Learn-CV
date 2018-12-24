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


def calFeature(img):
    m, n = img.shape
    ret = []

    # 笔划密度
    for i in range(0, m, 4):
        ret.append(np.count_nonzero(img[i, :]))
    for j in range(0, n, 4):
        ret.append(np.count_nonzero(img[:, j]))

    # 投影特征
    window = [[0, m // 2, 0, n // 2], [0, m // 2, n // 2, n],
              [m // 2, m, 0, n // 2], [m // 2, m, n // 2, n]]
    for index in window:
        region = img[index[0]:index[1], index[2]:index[3]]
        arg = np.nonzero(region)
        for i in range(2):
            if arg[i].shape[0]:
                ret.append(arg[i].max() - arg[i].min())
            else:
                ret.append(0)

    # 重心特征
    x, y = np.where(img == img)
    ret.append(np.sum(img * x.reshape((m, n))) / np.sum(img))
    ret.append(np.sum(img * y.reshape((m, n))) / np.sum(img))

    # 八点特征
    direction = [[1, 0], [1, -1], [0, -1], [-1, -1], [-1, 0], [-1, 1], [0, 1], [1, 1]]
    xstart = [m // 2, 0, m - 1]
    ystart = [n // 2, 0, n - 1]

    for d in direction:
        x = xstart[d[0]]
        y = ystart[d[1]]
        while 0 <= x < m and 0 <= y < n:
            if img[x, y]:
                ret.append(x)
                ret.append(y)
                break
            x += d[0]
            y += d[1]
        if not (0 <= x < m and 0 <= y < n):
            ret.append(m // 2)
            ret.append(n // 2)
    
    # # 傅里叶变换，要求为方阵
    ff = np.angle(np.fft.fft2(img))
    ret = ret + ff[0].tolist()

    return ret


def preprocess(data):
    feature = []
    for i in range(data.shape[0]):
        print('Preprocessing {}'.format(i))
        # img = img[(img != 0).sum(axis=0) != 0][:, (img != 0).sum(axis=1) != 0]
        # print(img.shape)
        # cv2.imshow('1806', img[i])
        # cv2.waitKey(0)
        # cv2.destroyAllWindows()
        feature.append(calFeature(img[i]))
    return np.array(feature).astype(np.int32)

def get(dataset):
    lbl = np.fromfile('{}/label.npy'.format(dataset), dtype=np.uint8)
    feature = np.fromfile('{}/feature.npy'.format(dataset), dtype=np.int32).reshape((lbl.shape[0], -1))
    return lbl, feature

if __name__ == '__main__':
    for ds in ['train', 'test']:
        img, label = read(ds)
        features = preprocess(img)
        label.tofile('{}/{}.npy'.format(ds, 'label'))
        features.tofile('{}/{}.npy'.format(ds, 'feature'))
