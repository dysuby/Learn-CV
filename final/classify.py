from adaboost import classifier
import cv2
from os import listdir, path
import csv
import numpy as np

TARGET = 'digits/good samples/'
LABEL = 'test/labels.CSV'

if __name__ == '__main__':
    def np_to_str(arr): return ''.join(map(str, arr))
    with open('res/good samples.csv', 'w', newline='') as f, open(LABEL) as f_labels:
        f_csv = csv.writer(f)
        f_csv.writerow(['', 'angle1', 'angle2', 'angle3',
                        'angle4', 'sid', 'phone', 'identity', 'error_count', 'correct_rate'])

        labels_reader = csv.reader(f_labels)
        lbl = {}
        for row in labels_reader:
            lbl[row[0]] = row[0] + row[1] + row[2]    # 正常
            # lbl[row[0]] = row[0] + row[2] + row[1]      # 身份证先

        cf = classifier()
        for p in listdir(TARGET):
            digits_dir = TARGET + p
            if path.isdir(digits_dir):
                with open(digits_dir + '/angles.txt') as angle_file:
                    raw_angles = angle_file.read().split()
                    angles = []
                    for i in range(4):
                        angles.append('({}, {})'.format(
                            raw_angles[i * 2], raw_angles[i * 2 + 1]))
                imgs = []
                for i in range(len(listdir(digits_dir)) - 1):
                    img = cv2.imread(
                        '{}/{}.bmp'.format(digits_dir, i), cv2.IMREAD_GRAYSCALE)
                    imgs.append(img.reshape(784, ))

                result = cf.predict(np.array(imgs))[:37]

                error_count = 0
                single_lbl = lbl[p]
                # 正常
                sid = np_to_str(result[:8])
                phone = np_to_str(result[8:19])
                identity = np_to_str(result[19:37])
                # 身份证先
                # sid = np_to_str(result[:8])
                # identity = np_to_str(result[8:26])
                # phone = np_to_str(result[26:37])
                for i in range(len(result)):
                    error_count += str(result[i]) != single_lbl[i]
                correct_rate = (
                    (len(result)) - error_count) / len(result)

                f_csv.writerow(
                    ['{}.bmp'.format(p), *angles, sid + '#', phone + '#', identity + '#', error_count, str(correct_rate) * 100 + '%'])
