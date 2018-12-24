import cv2
import numpy as np
import matplotlib.pyplot as plt


from mnist import get


class Adaboost:

    def __init__(self, data, label):
        self.data = data
        self.label = label
        self.weak_classify = []
        self.m = self.data.shape[0]
        self.H = np.zeros((self.m, ))
        self.D = np.ones((self.m, )) / self.m

    def stump_classify(self, data, dim, thresh, thresh_ineq):
        """
        基于单层决策树的弱分类器
        """
        res = np.ones((data.shape[0], ))
        if thresh_ineq == 'lt':
            res[data[:, dim] <= thresh] = -1.0
        else:
            res[data[:, dim] > thresh] = -1.0
        return res

    def build_stump(self):
        """
        找出当前 D 下最优的分类器
        """
        m, n = self.data.shape

        step = 5
        min_err = np.inf
        best_stump = {}

        for dim in range(n):
            min_val, max_val = self.data[:, dim].min(), self.data[:, dim].max()
            for thresh in range(min_val - 1, max_val + 1, step):
                for ineq in ['lt', 'gt']:
                    res = self.stump_classify(self.data, dim, thresh, ineq)
                    err = np.ones((m, ))

                    err[res == self.label] = 0
                    err_sum = min(np.sum(self.D * err), 1 - 1e-16)  # 避免浮点数标准导致大于 1

                    if err_sum < min_err:
                        best_stump['dim'] = dim
                        best_stump['thresh'] = thresh
                        best_stump['ineq'] = ineq
                        best_stump['res'] = res
                        min_err = err_sum

        return best_stump, min_err

    def adaboost(self, T):
        """
        Adaboost 算法
        """
        self.errs = np.empty((T, ))

        for t in range(T):
            stump, err = self.build_stump()

            alpha = 0.5 * np.log((1 - err) / max(err, 1e-16))

            e = np.exp(-alpha * self.label * stump['res'])
            self.D = self.D * e / np.sum(self.D)

            print('D: ', self.D)

            self.H += alpha * stump['res']

            stump['alpha'] = alpha

            self.weak_classify.append(stump)
            self.errs[t] = err

            print("t: {} alpha: {} err: {}".format(t, alpha, err))

        print('Final err rate: {}'.format(
            np.count_nonzero(np.sign(self.H) != self.label) / self.m))

        return self.H

    def test(self, test_data, test_label):
        """
        测试
        """
        m = test_data.shape[0]
        T = len(self.weak_classify)
        aggClassify = np.zeros((m, ))
        self.test_errs = np.empty((T, ))

        for t in range(T):
            # 预测
            res = self.stump_classify(
                test_data, self.weak_classify[t]['dim'], self.weak_classify[t]['thresh'], self.weak_classify[t]['ineq'])
            predict = res * self.weak_classify[t]['alpha']
            aggClassify += predict

            # 计算当前错误率
            self.test_errs[t] = np.count_nonzero(
                np.sign(aggClassify) != test_label) / m

        # 计算最终错误率
        print("test err rate: {}".format(self.test_errs[-1]))
        return self.test_errs

    def showinPLT(self, title):
        """
        显示错误率
        """
        n_trees_discrete = len(self.weak_classify)

        plt.figure(figsize=(10, 5))
        plt.title(title)

        plt.subplot(1, 2, 1)
        plt.plot(range(1, n_trees_discrete + 1),
                 self.test_errs, c='r', label='test')

        plt.legend()
        plt.ylim(0, 1)
        plt.yticks(np.arange(0, 1, 0.1))
        plt.annotate('%.3f'%self.test_errs[-1], xy=(n_trees_discrete, self.test_errs[-1]))
        plt.ylabel('Test Error Rate')
        plt.xlabel('T')

        plt.subplot(1, 2, 2)
        plt.plot(range(1, n_trees_discrete + 1),
                 self.errs, c='b', label='train')
        plt.plot()

        plt.legend()
        plt.ylim((0, 1))
        plt.yticks(np.arange(0, 1, 0.1))
        plt.annotate('%.3f'%self.errs[-1], xy=(n_trees_discrete, self.errs[-1]))
        plt.ylabel('Train Error Rate')
        plt.xlabel('T')

        plt.subplots_adjust(wspace=0.4)
        plt.show()


if __name__ == '__main__':
    label, features = get('train')
    test_lbl, test_feature = get('train')

    T = 200
    for i in range(10):
        # 训练
        lbl = np.where(label == i, 1, -1)
        ab = Adaboost(features, lbl)
        ab.adaboost(T)

        # 测试
        lbl = np.where(test_lbl == i, 1, -1)
        ab.test(test_feature, lbl)
        ab.showinPLT('Digit: {}'.format(i))
        print('Classfier {} done'.format(i))

