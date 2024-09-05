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
    INCDBSCAN(double eps, int minPts, int dimensions, KDTree& kdTree)
        : eps(eps), minPts(minPts), kdTree(kdTree), nextClusterId(0) {}

    void cluster(const std::vector<std::vector<double>>& points) {
        clusters.clear();
        noise.clear();
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
                        reassignClusterId(neighbor, oldClusterId, newClusterId);
                    }
                }
            }

            std::cout << "New cluster ID: " << newClusterId << std::endl;
            
            // Step 5: Expand the cluster with the new or existing cluster ID
            expandCluster(point, neighbors, newClusterId);
        } else {
            // Not enough neighbors to form a cluster, mark point as noise
            noise.push_back(point);
            kdTree.updateClusterId(point, -1);  // Mark as noise in KDTree
        }
    }

    void expandCluster(const std::vector<double>& point, const std::vector<std::vector<double>>& neighbors, int clusterId) {
        std::cout << "Expanding cluster " << clusterId << " with point: " << point[0] << ", " << point[1] << std::endl;
        
        // Add point to the cluster in both the KDTree and the local clusters map
        clusters[clusterId].push_back(point);
        kdTree.updateClusterId(point, clusterId);  // Update cluster ID in KDTree
        noise.erase(std::remove(noise.begin(), noise.end(), point), noise.end());


        for (const auto& neighbor : neighbors) {
            if (isNoise(neighbor)) {
                // Neighbor was previously marked as noise, add to the cluster
                clusters[clusterId].push_back(neighbor);
                kdTree.updateClusterId(neighbor, clusterId);
                noise.erase(std::remove(noise.begin(), noise.end(), neighbor), noise.end());
            } else if (!isAssignedToCluster(neighbor)) {
                // Neighbor is not yet assigned to any cluster, add it and expand
                clusters[clusterId].push_back(neighbor);
                kdTree.updateClusterId(neighbor, clusterId);
                auto newNeighbors = kdTree.radiusSearch(neighbor, eps);
                if (newNeighbors.size() >= minPts) {
                    expandCluster(neighbor, newNeighbors, clusterId);
                }
            }
        }
    }

    int getClusterId(const std::vector<double>& point) const {
        // Retrieve the cluster ID directly from the KDTree
        return kdTree.getClusterId(point);
    }

    void reassignClusterId(const std::vector<double>& point, int oldClusterId, int newClusterId) {
        // Remove point from old cluster and reassign to new cluster
        auto& oldCluster = clusters[oldClusterId];
        auto it = std::find(oldCluster.begin(), oldCluster.end(), point);
        if (it != oldCluster.end()) {
            oldCluster.erase(it);
            clusters[newClusterId].push_back(point);
            kdTree.updateClusterId(point, newClusterId);  // Update in KDTree
        }

        if (oldCluster.empty()) {
            clusters.erase(oldClusterId);
        }
    }


    const std::unordered_map<int, std::vector<std::vector<double>>>& getClusters() const {
        return clusters;
    }

    void getClustersLabels(std::vector<int>& clusterIds) {
        clusterIds.clear();
        clusterIds.reserve(clusters.size());
        for (const auto& [id, cluster] : clusters) {
            clusterIds.push_back(id);
        }
    }


private:
    double eps;
    int minPts;
    KDTree& kdTree;
    std::unordered_map<int, std::vector<std::vector<double>>> clusters;
    std::vector<std::vector<double>> noise;

    int nextClusterId;

    bool isNoise(const std::vector<double>& point) const {
        std::cout << "Checking if point is noise: " << point[0] << ", " << point[1] << std::endl;
        return std::find(noise.begin(), noise.end(), point) != noise.end();
    }

    bool isAssignedToCluster(const std::vector<double>& point) const {
        std::cout << "Checking if point is assigned to a cluster: " << point[0] << ", " << point[1] << std::endl;
        std::cout << "Clusters size: " << clusters.size() << std::endl;
        for (const auto& [id, cluster] : clusters) {
            std::cout << "Cluster ID: " << id << std::endl;
            if (std::find(cluster.begin(), cluster.end(), point) != cluster.end()) {
                return true;
            }
        }
        std::cout << "Point is not assigned to any cluster." << std::endl;
        return false;
    }
};

#endif // INCDBSCAN_H