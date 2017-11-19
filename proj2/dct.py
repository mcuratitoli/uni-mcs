import os
import sys
import numpy as np
from scipy.misc import imread
from scipy.fftpack import dct, idct
import matplotlib.pyplot as plt
import pylab

Q = np.array([[16, 11, 10, 16, 24, 40, 51, 61],
               [12, 12, 14, 19, 26, 58, 60, 55],
               [14, 13, 16, 24, 40, 57, 69, 56],
               [14, 17, 22, 29, 51, 87, 80, 62],
               [18, 22, 37, 56, 68, 109, 103, 77],
               [24, 35, 55, 64, 81, 104, 113, 92],
               [49, 64, 78, 87, 103, 121, 120, 101],
               [72, 92, 95, 98, 112, 100, 103, 99]],
               dtype='float32')

def complete_img(img, bs=8):
    if img.shape[0] % bs != 0:	
        delta_r = bs - (img.shape[0] % bs)
        tmp = img[-1,:]
        for _ in xrange(delta_r - 1):
            tmp = np.vstack((tmp, img[-1,:]))
        img = np.vstack((img, tmp))
    if img.shape[1] % bs != 0:
        delta_c = bs - (img.shape[1] % bs)
        col = img[:,-1]
        col = col[:,np.newaxis]
        tmp = col
        for _ in xrange(delta_c - 1):
            tmp = np.hstack((tmp, col))
        img = np.hstack((img, tmp))
    return img

def read_img(filename):
    if not(os.path.isfile(filename)):
        print "error: %s is not a regular file!" % filename
        sys.exit(1)
    return imread(filename, True)

def apply_dct(img, bs):
    for r in xrange(0, img.shape[0], bs):
        for c in xrange(0, img.shape[1], bs):
            img[r:r+bs, c:c+bs] = dct(dct(img[r:r+bs, c:c+bs].T, norm='ortho').T, norm='ortho')
    return img

def apply_idct(img, bs):
    for r in xrange(0, img.shape[0], bs):
        for c in xrange(0, img.shape[1], bs):
            img[r:r+bs, c:c+bs] = idct(idct(img[r:r+bs, c:c+bs].T, norm='ortho').T, norm='ortho')
    return img

def get_quantization_matrix(q, bs):
    if q <= 0:
        q = 1
    elif q >= 100:
        return np.ones((bs, bs), dtype='float32')
    qf = 0

    if q >= 50:
        qf = (200 - 2 * q) / 100.0
    else:
        qf = (5000.0 / q) / 100

    N = bs / 8

    Qc = np.array(qf * Q)
    cmpl = lambda x, y: Qc[x/N, y/N]
    return np.fromfunction(cmpl, (bs, bs), dtype='int32')

def lossless_cast(val):
    if val < 0:
        return 0
    elif val > 255:
        return 255
    else:
        return val

def compress_img(img, q, bs):
    # conversione dei valori della matrice a float32
    img = np.float32(img)
    # shift dei valori di -128
    img = img - 128
    # eseguo la dct sui blocchi bs di img
    img = apply_dct(img, bs)
    # ottengo la matrice di quantizzazione dati q e bs
    QN = get_quantization_matrix(q, bs)
    # divido gli elementi di ogni blocco (bs, bs) termine a termine per la matrice QN
    for r in xrange(0, img.shape[0], bs):
        for c in xrange(0, img.shape[1], bs):
            blk = img[r:r+bs, c:c+bs]
            div = lambda x, y: blk[x,y] / QN[x,y] 
            img[r:r+bs, c:c+bs] = np.fromfunction(div, (bs, bs), dtype='int32')
    # effettuo l'arrotondamento a intero dei valori della matrice 
    img = np.round(img)
    # moltiplico gli elementi di ogni blocco (bs, bs) termine a termine per la matrice QN
    for r in xrange(0, img.shape[0], bs):
        for c in xrange(0, img.shape[1], bs):
            blk = img[r:r+bs, c:c+bs]
            mul = lambda x, y: blk[x,y] * QN[x,y] 
            img[r:r+bs, c:c+bs] = np.fromfunction(mul, (bs, bs), dtype='int32')
    # eseguo la idct sui blocchi bs di img
    img = apply_idct(img, bs)
    # shift dei valori di +128
    img = img + 128
    # effettuo l'arrotondamento a intero dei valori della matrice 
    img = np.round(img)
    # effettuo il casting dei valori prima della conversione da float32 a
    # uint8
    cast = np.vectorize(lossless_cast)
    img = cast(img)
    # converto da float32 a uint8
    img = np.uint8(img)
    return img
