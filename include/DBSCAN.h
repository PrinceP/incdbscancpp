// DBSCAN.h
#ifndef DBSCAN_H
#define DBSCAN_H

#include "KDTree.h"
#include <vector>
#include <unordered_map>
#include <set>
#include <chrono>

class DBSCAN {
public:
    DBSCAN(double eps, int minPts, KDTree& kdTree, int& clusterID)
        : eps(eps), minPts(minPts), kdTree(kdTree), clusterID(clusterID) {}

    void cluster(const std::vector<std::vector<double>>& points) {
        // Initialize all points as not visited
        visited.assign(points.size(), false);
        clusters.assign(points.size(), -1);
        auto start = std::chrono::high_resolution_clock::now();
        // Insert points into KD-Tree
        for (size_t i = 0; i < points.size(); ++i) {
            kdTree.insert(points[i], i);
        }
        std::cout << "KDTree size: " << kdTree.size() << std::endl;
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        auto durationInSeconds = duration / 1e6; // Convert microseconds to seconds
        std::cout << "Time taken to insert all the points into the KDTree: " << durationInSeconds << " seconds" << std::endl;

        // Process each point
        start = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < points.size(); ++i) {
            if (!visited[i]) {
                if (expandCluster(points, i)) {
                    clusterID++;
                }
            }
        }
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        durationInSeconds = duration / 1e6; // Convert microseconds to seconds
        std::cout << "Time taken to cluster: " << durationInSeconds << " seconds" << std::endl;
    }

    void getClustersLabels(std::vector<int>& clusterIds, int& clusterID) {
        clusterIds.clear();
        clusterIds.reserve(clusters.size());
        for (int cluster : clusters) {
            clusterIds.push_back(cluster);
        }
        clusterID = this->clusterID;
    }

    void getLastClusterId(int& clusterId){
        clusterId = this->clusterID;
    }

private:
    double eps;
    int minPts;
    int clusterID;
    KDTree& kdTree;
    std::vector<bool> visited;
    std::vector<int> clusters;

    bool expandCluster(const std::vector<std::vector<double>>& points, size_t index) {
        auto start = std::chrono::high_resolution_clock::now();
        auto neighbors_of_index = kdTree.radiusSearch(points[index], eps);
        std::vector<size_t> neighbors;
        for (const auto& neighbor : neighbors_of_index) {
            neighbors.push_back(kdTree.getIndex(neighbor));
        }
        // auto end = std::chrono::high_resolution_clock::now();
        // auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        // auto durationInSeconds = duration / 1e6; // Convert microseconds to seconds
        // std::cout << "Time taken to find neighbors for index " << index << " : " << durationInSeconds << " seconds" << std::endl;
        if (neighbors.size() < minPts) {
            clusters[index] = -1; // Mark as noise
            return false;
        } else {
            kdTree.assignClusterID(points[index], clusterID);
            int currentClusterID = clusterID;
            std::set<size_t> seeds(neighbors.begin(), neighbors.end());
            seeds.erase(index);
            clusters[index] = currentClusterID;
            auto start = std::chrono::high_resolution_clock::now();
            while (!seeds.empty()) {
                size_t currentPoint = *seeds.begin();
                seeds.erase(seeds.begin());
                if (!visited[currentPoint]) {
                    visited[currentPoint] = true;
                    // auto start = std::chrono::high_resolution_clock::now();
                    auto neighbors = kdTree.radiusSearchUsingCache(points[currentPoint], eps);
                    // std::cout << "Found " << neighbors.size() << " neighbors for index " << currentPoint << std::endl;
                    std::vector<size_t> currentNeighbors;
                    for (const auto& neighbor : neighbors) {
                        currentNeighbors.push_back(kdTree.getIndex(neighbor));
                    }
                    // auto end = std::chrono::high_resolution_clock::now();
                    // auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                    // auto durationInSeconds = duration / 1e6; // Convert microseconds to seconds
                    // std::cout << "Time taken to find neighbors for index " << currentPoint << " : " << durationInSeconds << " seconds" << std::endl;
                    if (currentNeighbors.size() >= minPts) {
                        seeds.insert(currentNeighbors.begin(), currentNeighbors.end());
                    }
                    clusters[currentPoint] = currentClusterID;
                    kdTree.assignClusterID(points[currentPoint], clusterID); 
                }
            }
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            auto durationInSeconds = duration / 1e6; // Convert microseconds to seconds
            std::cout << "Time taken to expand cluster: " << durationInSeconds << " seconds" << std::endl;
            return true;
        }
    }

};

#endif
