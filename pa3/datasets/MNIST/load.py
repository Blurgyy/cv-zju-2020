#!/usr/bin/env -S python3 -u

# Load mnist dataset (28x28 single channel images)

import matplotlib.pyplot as plt
import numpy as np
import random
import os
import sys


def loadimg(imgfile: str, idxfile: str, nimages: int):
    size = 28
    # Get images
    with open(imgfile, 'rb') as f:
        image_header = f.read(16)  # useless header bits?
        with open(".image_header", 'wb') as f2:
            f2.write(image_header)
        buf = f.read(size * size * nimages)
        data = np.frombuffer(buf, np.uint8)
    # # 8test8
    # with open("testfile", 'wb') as f:
    #     f.write(image_header)
    #     f.write(data.reshape(nimages, size, size, 1).reshape(-1).tobytes())
    #     # f.write(data.tobytes())
    # sys.exit()
    # # 8test8
    images = data.reshape(nimages, size, size, 1)
    # Get labels
    with open(idxfile, 'rb') as f:
        index_header = f.read(8)
        with open(".index_header", 'wb') as f2:
            f2.write(index_header)
        buf = f.read(nimages)
        data = np.frombuffer(buf, np.uint8)
    labels = data.reshape(-1)
    return images, labels


def dumpimg(imgfile: str, idxfile: str, data: dict):
    with open("./.image_header", 'rb') as f:
        image_header = f.read()
    with open("./.index_header", 'rb') as f:
        index_header = f.read()
    # Write images
    images = [y for x in range(11) for y in data[x]]
    images = np.array(images, dtype=np.uint8)
    # for img in images:
    #     plt.figure()
    #     plt.imshow(img.squeeze())
    #     plt.show()
    images = images.reshape(-1)
    with open(imgfile, 'wb') as f:
        f.write(image_header)
        f.write(images.tobytes())
    # Write labels
    labels = [idx for idx in range(11) for x in range(len(data[idx]))]
    labels = np.array(labels, dtype=np.uint8)
    labels = labels.reshape(-1)
    with open(idxfile, 'wb') as f:
        f.write(index_header)
        f.write(labels.tobytes())


def main():
    train_set_param = {
        'imgfile': "./original/train-images-idx3-ubyte",
        'idxfile': "./original/train-labels-idx1-ubyte",
        'nimages': 60000,
    }
    test_set_param = {
        'imgfile': "./original/t10k-images-idx3-ubyte",
        'idxfile': "./original/t10k-labels-idx1-ubyte",
        "nimages": 10000,
    }

    # Organize training set
    images, labels = loadimg(**train_set_param)
    print(images.shape, labels.shape)
    train_dict = {x: [] for x in range(11)}
    for i in range(train_set_param['nimages']):
        image = images[i]
        label = int(labels[i])
        train_dict[label].append(image)
    assert len(train_dict[10]) == 0
    # Augment training set
    for i in range(6000):
        one_img = random.choice(train_dict[1])
        zero_img = random.choice(train_dict[0])
        joined_img = np.maximum(one_img, zero_img)
        train_dict[10].append(joined_img)

    # Organize testing set
    images, labels = loadimg(**test_set_param)
    print(images.shape, labels.shape)
    test_dict = {x: [] for x in range(11)}
    for i in range(test_set_param['nimages']):
        image = images[i]
        label = int(labels[i])
        test_dict[label].append(image)
    assert len(test_dict[10]) == 0
    # Augment testing set
    for i in range(1000):
        one_img = random.choice(test_dict[1])
        zero_img = random.choice(test_dict[0])
        joined_img = np.maximum(one_img, zero_img)
        test_dict[10].append(joined_img)

    # Write augmented datasets to file
    if not os.path.exists("./raw"):
        os.makedirs("./raw")
    dumpimg(
        os.path.join("raw", "train-images-idx3-ubyte"),
        os.path.join("raw", "train-labels-idx1-ubyte"),
        train_dict,
    )
    dumpimg(
        os.path.join("raw", "t10k-images-idx3-ubyte"),
        os.path.join("raw", "t10k-labels-idx1-ubyte"),
        test_dict,
    )


def test():
    param = {
        'imgfile': "./raw/train-images-idx3-ubyte",
        'idxfile': "./raw/train-labels-idx1-ubyte",
        'nimages': 66000,
    }
    images, labels = loadimg(**param)
    for _ in range(10):
        idx = random.randint(0, len(images) - 1)
        img = images[idx]
        lab = labels[idx]
        print(lab)
        plt.figure()
        plt.imshow(img.squeeze())
        plt.show()


if __name__ == "__main__":
    main()
    # test()

# Author: Blurgy <gy@blurgy.xyz>
# Date:   Jan 15 2021, 15:59 [CST]