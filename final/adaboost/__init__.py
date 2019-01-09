from sklearn.externals import joblib
import os

def classifier():
    return joblib.load(os.path.dirname(os.path.abspath(__file__)) + '/model/model')
