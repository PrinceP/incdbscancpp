#ifndef INCDBSCAN_H
#define INCDBSCAN_H

#include "KDTree.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <functional>
#include <algorithm>
#include <stack>
#include <chrono>
class INCDBSCAN {
public:
    INCDBSCAN(double eps, int minPts, KDTree& kdTree)
        : eps(eps), minPts(minPts), kdTree(kdTree) {}

    void cluster(const std::vector<std::vector<double>>& points, int nextClusterId, int startingIndex) {
        this->nextClusterId = nextClusterId;
        this->startingIndex = startingIndex;
        std::cout << "Starting INCDBSCAN with clusterID " << nextClusterId << " startingIndex " << startingIndex << std::endl;
        // Initialize all points as not visited
        visited.assign(points.size() + startingIndex, false);
        

        //Benchmark the time taken to insert all the points into the KDTree
        auto start = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < points.size(); ++i) {
            kdTree.insert(points[i], i+startingIndex);
        }
        std::cout << "KDTree size: " << kdTree.size() << std::endl;
    
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        double durationInSeconds = duration / 1e6; // Convert microseconds to seconds
        std::cout << "Time taken to insert all the points into the KDTree: " << durationInSeconds << " seconds" << std::endl;
        
        //Call insertPoint for each point with index
        start = std::chrono::high_resolution_clock::now();
        std::cout << "Newly added points size : " << points.size() << std::endl;
        for (int i = 0; i < points.size(); i++) {
            if (!visited[i + startingIndex]) {
                insertPoint(points[i], i + startingIndex);
            }
        }
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        durationInSeconds = duration / 1e6; // Convert microseconds to seconds
        std::cout << "Time taken to call insertPoint for each point: " << durationInSeconds << " seconds" << std::endl;
        
        // //Update the cluster ID of the points in the KDTree
        // for(int i = 0; i < points.size(); i++){
        //     int clusterID = clusters[i + startingIndex];
        //     kdTree.assignClusterID(points[i], clusterID);
        // }
        
        //Sort the pairs of merge_cluster_pairs
        std::sort(merge_cluster_pairs.begin(), merge_cluster_pairs.end());
        //Optimize the merge_cluster_pairs by removing the duplicate pairs
        std::unordered_map<int, int> cleaned_merges;

        // Iterate through merge pairs and keep track of the last conversion
        for (const auto& merge_cluster_pair : merge_cluster_pairs) {
            cleaned_merges[merge_cluster_pair.first] = merge_cluster_pair.second;
        }
        // merge_cluster_pairs.erase(std::unique(merge_cluster_pairs.begin(), merge_cluster_pairs.end()), merge_cluster_pairs.end());
        //Benchmark the time taken to merge the clusters
        start = std::chrono::high_resolution_clock::now();
        for (const auto& pair : cleaned_merges) {
            std::cout << "Merging clusters " << pair.first << " to " << pair.second << std::endl;
            kdTree.mergeClusters(pair.first, pair.second);
        }
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        durationInSeconds = duration / 1e6; // Convert microseconds to seconds
        std::cout << "Time taken to merge the clusters: " << durationInSeconds << " seconds" << std::endl;
        
        
    }

    
    void insertPoint(const std::vector<double>& point, int index) {
        
        // Step 1.1: Find neighborhood of current new point
        auto start = std::chrono::high_resolution_clock::now();
        auto neighbors = kdTree.radiusSearchUsingCache(point, eps);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        double durationInSeconds = duration / 1e6; // Convert microseconds to seconds
        // std::cout << "Time taken to find neighbors for index " << index << " : " << durationInSeconds << " seconds" << std::endl;

        // Debug
        // std::cout << "Found " << neighbors.size() << " neighbors." << " for index " << index << std::endl;
        
        // Step 1.2: Check current point is core point or not
        if(neighbors.size() < minPts){
            //Assign noise to the current point
            kdTree.assignClusterID(point, -1);
            visited[index] = true;
            return;
        }
        // Step 1.3: Get all the cluster IDs of each of the neighbors
        // start = std::chrono::high_resolution_clock::now();
        std::set<int> labels_of_neighbors;
        for(auto neighbor : neighbors){
            int label = kdTree.getClusterId(neighbor);
            //TODO: Done
            if(label != -1){
                labels_of_neighbors.insert(label);
            }
        }
        // end = std::chrono::high_resolution_clock::now();
        // duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        // durationInSeconds = duration / 1e6; // Convert microseconds to seconds
        // std::cout << "Time taken to get labels of core points: " << durationInSeconds << " seconds" << std::endl;
        // Debug
        // std::cout << "Labels of core points: " << labels_of_neighbors.size() << std::endl;
        // for(auto label : labels_of_neighbors){
        //     std::cout << " Label: " << label;
        // }
        // std::cout << std::endl;
        // Step 1.4: If none of the core point is labeled
        if(labels_of_neighbors.size() == 0){
            // Debug
            // std::cout << "No labeled core points found, creating a new cluster" << std::endl;
            start = std::chrono::high_resolution_clock::now();
            modified_expandCluster(point, index, nextClusterId++);
            end = std::chrono::high_resolution_clock::now();
            duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            durationInSeconds = duration / 1e6; // Convert microseconds to seconds
            std::cout << "Time taken to expand cluster CASE A: " << durationInSeconds << " seconds" << std::endl;
        }

        // Step 1.5: If all the core points are labeled
        else if(labels_of_neighbors.size() == 1){
            // Debug
            // std::cout << "All labeled core points found, assigning to the existing cluster" << std::endl;
            start = std::chrono::high_resolution_clock::now();
            modified_expandCluster(point, index, *labels_of_neighbors.begin());
            end = std::chrono::high_resolution_clock::now();
            duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            durationInSeconds = duration / 1e6; // Convert microseconds to seconds
            std::cout << "Time taken to expand cluster CASE B: " << durationInSeconds << " seconds" << std::endl;
        }

        // Step 1.6: If the core points are labeled differently
        else if(labels_of_neighbors.size() > 1){
            // Debug
            // std::cout << "Multiple labeled core points found, merging the clusters" << std::endl;
            start = std::chrono::high_resolution_clock::now();
            modified_expandCluster(point, index, nextClusterId++);
            end = std::chrono::high_resolution_clock::now();
            duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            durationInSeconds = duration / 1e6; // Convert microseconds to seconds
            std::cout << "Time taken to expand cluster CASE C: " << durationInSeconds << " seconds" << std::endl;
        }

    }

    void modified_expandCluster(const std::vector<double>& currentPoint, int index, int clusterID) {
        std::stack<std::pair<std::vector<double>, int>> dfsStack;
        std::vector<std::pair<std::vector<double>, int>> dfsPath;
        std::unordered_set<int> visiteddfsPoints;
        std::unordered_set<int> uniqueLabels;
        
        dfsStack.push({currentPoint, index});

        while (!dfsStack.empty()) {
            auto [point, currentIndex] = dfsStack.top();
            dfsStack.pop();
            if (!visited[currentIndex]) {
                visited[currentIndex] = true;
                
                auto neighbors = kdTree.radiusSearchUsingCache(point, eps);
                
                if (neighbors.size() >= minPts) {
                    dfsPath.push_back({currentPoint,currentIndex});
                    // Current point is a core point
                    for (const auto& neighbor : neighbors) {
                        //TODO: Same call is needed for below
                        int neighborIndex = kdTree.getIndex(neighbor);
                        int neighborClusterID = kdTree.getClusterId(neighbor);
                        if(neighborClusterID != -1){
                            uniqueLabels.insert(neighborClusterID);
                        }
                        else{
                            if(!visited[neighborIndex]){
                                if(visiteddfsPoints.find(neighborIndex) == visiteddfsPoints.end()){
                                    dfsStack.push({neighbor, neighborIndex});
                                    dfsPath.push_back({neighbor, neighborIndex});
                                    visiteddfsPoints.insert(neighborIndex);
                                }
                            }
                        }
                    
                    }
                }
                else{
                    // Current point is a border point
                    // Do nothing
                }
                
            }
        }

        //Debug
        if(uniqueLabels.size() != 0){
            std::cout << "Unique labels for index "  << index <<" and incoming clusterid " << clusterID << " : " << uniqueLabels.size() << std::endl;
            for(auto label : uniqueLabels){
                std::cout << " Label: " << label;
            }
            std::cout << std::endl;
        }
        
        // Determine the cluster ID to assign
        int assignClusterID;
        if (uniqueLabels.empty()) {
            // Case 3: No labeled points encountered, create a new cluster
            assignClusterID = clusterID;
        } else if (uniqueLabels.size() == 1) {
            // Case 1: Only one unique label encountered
            assignClusterID = *uniqueLabels.begin();
        } else {
            // Case 2: Multiple labels encountered, all labels should merge to the new cluster
            assignClusterID = clusterID;
            for (auto it = uniqueLabels.begin(); it != uniqueLabels.end(); ++it) {
                merge_cluster_pairs.emplace_back(*it, assignClusterID);
            }
        }
        
        // Assign the determined cluster ID to all points in the DFS path
        //TODO: Need to be O(1)
        for (auto path : dfsPath) {
            kdTree.updateClusterId(path.first, assignClusterID);
        }
    }

    void getLastClusterId(int& clusterId){
        clusterId = nextClusterId;
    }

    void getClustersLabels(std::vector<int>& clusterIds, int& clusterID) {
        clusterIds.clear();
        clusterIds.reserve(clusters.size());
        //Take values from startIndex
        for (int i = 0/*startingIndex*/; i < clusters.size(); ++i) {
            clusterIds.push_back(clusters[i]);
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
    int startingIndex;
    std::vector<std::pair<int, int>> merge_cluster_pairs;
    

};

#endif // INCDBSCAN_H