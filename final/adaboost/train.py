import matplotlib.pyplot as plt

from sklearn.ensemble import AdaBoostClassifier
from sklearn.metrics import accuracy_score
from sklearn.tree import DecisionTreeClassifier
from sklearn.externals import joblib

import numpy as np

from mnist import read


def adaboost():
    X_train, y_train = read('train')
    X_test, y_test = read('test')
    X_train = X_train.reshape((X_train.shape[0], -1))
    # X_train[X_train != 0] = 255
    # X_test[X_test != 0] = 255
    X_test = X_test.reshape((X_test.shape[0], -1))
    
    bdt_discrete = AdaBoostClassifier(
        DecisionTreeClassifier(
            max_depth=40, min_samples_split=50, min_samples_leaf=50),
        n_estimators=1000,
        learning_rate=0.05,
        algorithm='SAMME')

    bdt_discrete.fit(X_train, y_train)

    discrete_test_errors = []

    for discrete_train_predict in bdt_discrete.staged_predict(X_test):
        discrete_test_errors.append(
            1. - accuracy_score(discrete_train_predict, y_test))

    return bdt_discrete, discrete_test_errors


def showinPLT(bdt_discrete, discrete_test_errors):
    n_trees_discrete = len(bdt_discrete)
    discrete_estimator_errors = bdt_discrete.estimator_errors_[
        :n_trees_discrete]
    discrete_estimator_weights = bdt_discrete.estimator_weights_[
        :n_trees_discrete]

    plt.figure(figsize=(15, 5))

    plt.subplot(131)
    plt.plot(range(1, n_trees_discrete + 1),
             discrete_test_errors, c='black', label='SAMME')
    plt.annotate('%.3f' % discrete_test_errors[-1],
                 xy=(n_trees_discrete, discrete_test_errors[-1]))

    plt.legend()
    plt.ylim(0, 1)
    plt.ylabel('Test Error')
    plt.xlabel('Number of Trees')

    plt.subplot(132)
    plt.plot(range(1, n_trees_discrete + 1), discrete_estimator_errors,
             "b", label='SAMME', alpha=.5)

    plt.legend()
    plt.ylabel('Error')
    plt.xlabel('Number of Trees')
    plt.ylim((0, discrete_estimator_errors.max() * 1.2))
    plt.xlim((-20, len(bdt_discrete) + 20))

    plt.subplot(133)
    plt.plot(range(1, n_trees_discrete + 1), discrete_estimator_weights,
             "b", label='SAMME')

    plt.legend()
    plt.ylabel('Weight')
    plt.xlabel('Number of Trees')
    plt.ylim((0, discrete_estimator_weights.max() * 1.2))
    plt.xlim((-20, n_trees_discrete + 20))

    # prevent overlapping y-axis labels
    plt.subplots_adjust(wspace=0.25)
    plt.show()


if __name__ == '__main__':
    bdt, test_error = adaboost()
    showinPLT(bdt, test_error)
    joblib.dump(bdt, 'model/model')
