# python3.9 tester.py ../clusters_500.txt 
import pandas as pd
from sklearn import metrics
import sys
import os

# Read the text file
file_path = sys.argv[1]
df = pd.read_csv(file_path, header=None, names=['track_id', 'cluster'])

# Map track_id and cluster to numerical labels
df['track_id'] = pd.factorize(df['track_id'])[0]
df['cluster'] = pd.factorize(df['cluster'])[0]

# Calculate the metrics
hgs5, cps5, v_meas5 = metrics.homogeneity_completeness_v_measure(labels_true=df['track_id'], labels_pred=df['cluster'], beta=1)

# Print the results
print(f'GID: Homogeneity, Completeness, V-Measure, #clusters: {hgs5*100:.2f} %, {cps5*100:.2f} %, {v_meas5*100:.2f} %, {len(df.cluster.unique())}')
print('# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #')

# Ground truth cluster: 99

# C++
# GID: Homogeneity, Completeness, V-Measure, #clusters: 94.05 %, 98.61 %, 96.27 %, 132
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #

# Python
# GID: Homogeneity, Completeness, V-Measure, #clusters: 94.05 %, 98.39 %, 96.17 %, 165
# # ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
