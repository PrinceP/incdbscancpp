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
        for (const auto& point : points) {
            insertPoint(point);
        }

    }

    void insertPoint(const std::vector<double>& point) {
        std::cout << "Inserting point: " << point[0] << ", " << point[1] << std::endl;
        
        // Step 1: Find neighbors using the KD-Tree radius search
        auto neighbors = kdTree.radiusSearch(point, eps);
        std::cout << "Found " << neighbors.size() << " neighbors." << std::endl;
        
        // Step 2: If neighbors are greater than or equal to minPts, form or expand a cluster
        if (neighbors.size() >= minPts) {
            std::cout << "Forming a cluster with " << neighbors.size() << " points." << std::endl;
            
            // Step 3: Collect the cluster IDs of the neighbors from the KDTree
            std::unordered_set<int> neighborClusterIds;
            for (const auto& neighbor : neighbors) {
                int neighborClusterId = kdTree.getClusterId(neighbor);
                std::cout << "Neighbor cluster ID: " << neighborClusterId << std::endl;
                if (neighborClusterId != -1) {
                    neighborClusterIds.insert(neighborClusterId);
                    std::cout << "Adding neighbor cluster ID: " << neighborClusterId << std::endl;
                }
            }

            // Step 4: Decide on a cluster ID for the current point
            int newClusterId;
            if (neighborClusterIds.size() == 1) {
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

            std::cout << "New cluster ID: " << newClusterId << std::endl;
            
            // Step 5: Expand the cluster with the new or existing cluster ID
            expandCluster(point, neighbors, newClusterId);
        } else {
            // Not enough neighbors to form a cluster, mark point as noise
        }
    }

    void expandCluster(const std::vector<double>& point, const std::vector<std::vector<double>>& neighbors, int clusterId) {
        std::cout << "Expanding cluster " << clusterId << " with point: " << point[0] << ", " << point[1] << std::endl;
        
        // Add point to the cluster in both the KDTree and the local clusters map
        clusters.push_back(clusterId);
        kdTree.updateClusterId(point, clusterId);  // Update cluster ID in KDTree
        

        std::cout << "Expanding cluster " << clusterId << " with neighbor: " << neighbors.size() << std::endl;
        for (const auto& neighbor : neighbors) {
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