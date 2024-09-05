#ifndef INCDBSCAN_H
#define INCDBSCAN_H

#include "KDTree.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <functional>
#include <algorithm>

class INCDBSCAN {
public:
    INCDBSCAN(double eps, int minPts, KDTree& kdTree, int& clusterID)
        : eps(eps), minPts(minPts), kdTree(kdTree), nextClusterId(clusterID) {}

    void cluster(const std::vector<std::vector<double>>& points) {
        // Initialize all points as not visited
        visited.assign(points.size(), false);
        clusters.assign(points.size(), -1);
        for (const auto& point : points) {
            kdTree.insert(point);
        }
        //Call insertPoint for each point with index
        for (int i = 0; i < points.size(); i++) {
            insertPoint(points[i], i);
        }

    }

    void insertPoint(const std::vector<double>& point, int index) {
        
        // Step 1: Find neighbors using the KD-Tree radius search
        auto neighbors = kdTree.radiusSearch(point, eps);
        std::cout << "Found " << neighbors.size() << " neighbors." << " for index " << index << std::endl;
        
        // Step 2: If neighbors are greater than or equal to minPts
        if (neighbors.size() >= minPts) {
            
            // Step 3: Collect the cluster IDs of the neighbors from the KDTree
            std::unordered_set<int> neighborClusterIds;
            for (const auto& neighbor : neighbors) {
                int neighborClusterId = kdTree.getClusterId(neighbor);
                if (neighborClusterId != -1) {
                    neighborClusterIds.insert(neighborClusterId);
                }
            }
            //Display the cluster IDs of the neighbors
            std::cout << "Neighbor cluster IDs: ";
            for (const auto& clusterId : neighborClusterIds) {
                std::cout << clusterId << " ";
            }
            std::cout << std::endl;
            // Step 4: Decide on a cluster ID for the current point
            int newClusterId;
            if (neighborClusterIds.empty()) {
                //No core points found, with a label
                
            } else if (neighborClusterIds.size() == 1) {
                // All neighbors belong to the same cluster, use that cluster ID
                std::cout << "All neighbors belong to the same cluster." << std::endl;
                newClusterId = *neighborClusterIds.begin();
            } else {
                // Neighbors belong to different clusters, create a new cluster ID
                std::cout << "Neighbors belong to different clusters, merging them." << std::endl;
                newClusterId = nextClusterId++;
                // Merge clusters by reassigning the new cluster ID to all neighbor points
                for (const auto& neighbor : neighbors) {
                    int oldClusterId = kdTree.getClusterId(neighbor);
                    if (oldClusterId != -1 && oldClusterId != newClusterId) {
                        std::cout << "Reassigning cluster ID: " << oldClusterId << " to " << newClusterId << std::endl;
                        kdTree.updateClusterId(neighbor, newClusterId);
                    }
                }
            }

            std::cout << "Final cluster ID: " << newClusterId << " for point " << index << std::endl;
            clusters[index] = newClusterId;
            visited[index] = true;
        } else {
            // Not enough neighbors to form a cluster, mark point as noise
            std::cout << "Not enough neighbors to form a cluster, marking point as noise." << std::endl;
            clusters[index] = -1;
            visited[index] = true;
        }
    }


    int getClusterId(const std::vector<double>& point) const {
        // Retrieve the cluster ID directly from the KDTree
        return kdTree.getClusterId(point);
    }

    void getClustersLabels(std::vector<int>& clusterIds, int& clusterID) {
        clusterIds.clear();
        clusterIds.reserve(clusters.size());
        for (const auto& cluster : clusters) {
            clusterIds.push_back(cluster);
        }
        clusterID = nextClusterId;
    }


private:
    double eps;
    int minPts;
    KDTree& kdTree;
    std::vector<bool> visited;
    std::vector<int> clusters;
    int nextClusterId;

};

#endif // INCDBSCAN_H