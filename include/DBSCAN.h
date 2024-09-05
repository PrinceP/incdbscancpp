// DBSCAN.h
#ifndef DBSCAN_H
#define DBSCAN_H

#include "KDTree.h"
#include <vector>
#include <unordered_map>
#include <set>

class DBSCAN {
public:
    DBSCAN(double eps, int minPts, KDTree& kdTree)
        : eps(eps), minPts(minPts), kdTree(kdTree), clusterID(0) {}

    void cluster(const std::vector<std::vector<double>>& points) {
        // Initialize all points as not visited
        visited.assign(points.size(), false);
        clusters.assign(points.size(), -1);

        // Insert points into KD-Tree
        for (const auto& point : points) {
            kdTree.insert(point);
        }

        // Process each point
        for (size_t i = 0; i < points.size(); ++i) {
            if (!visited[i]) {
                if (expandCluster(points, i)) {
                    clusterID++;
                }
            }
        }
    }

    void getClustersLabels(std::vector<int>& clusterIds) {
        clusterIds.clear();
        clusterIds.reserve(clusters.size());
        for (int cluster : clusters) {
            clusterIds.push_back(cluster);
        }
    }

private:
    double eps;
    int minPts;
    int clusterID;
    KDTree& kdTree;
    std::vector<bool> visited;
    std::vector<int> clusters;

    bool expandCluster(const std::vector<std::vector<double>>& points, size_t index) {
        std::vector<size_t> neighbors = regionQuery(points, points[index]);
        if (neighbors.size() < minPts) {
            clusters[index] = -1; // Mark as noise
            return false;
        } else {
            kdTree.assignClusterID(points[index], clusterID);
            int currentClusterID = clusterID;
            std::set<size_t> seeds(neighbors.begin(), neighbors.end());
            seeds.erase(index);
            clusters[index] = currentClusterID;
            while (!seeds.empty()) {
                size_t currentPoint = *seeds.begin();
                seeds.erase(seeds.begin());
                if (!visited[currentPoint]) {
                    visited[currentPoint] = true;
                    std::vector<size_t> currentNeighbors = regionQuery(points, points[currentPoint]);
                    if (currentNeighbors.size() >= minPts) {
                        seeds.insert(currentNeighbors.begin(), currentNeighbors.end());
                    }
                    clusters[currentPoint] = currentClusterID;
                    kdTree.assignClusterID(points[currentPoint], clusterID); 
                }
            }
            return true;
        }
    }

    std::vector<size_t> regionQuery(const std::vector<std::vector<double>>& points, const std::vector<double>& point) {
        std::vector<std::vector<double>> neighbors = kdTree.radiusSearch(point, eps);
        std::vector<size_t> indices;
        for (size_t i = 0; i < points.size(); ++i) {
            for (const auto& neighbor : neighbors) {
                if (points[i] == neighbor) {
                    indices.push_back(i);
                    break;
                }
            }
        }
        return indices;
    }
};

#endif
