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

class INCDBSCAN {
public:
    INCDBSCAN(double eps, int minPts, KDTree& kdTree, int& clusterID)
        : eps(eps), minPts(minPts), kdTree(kdTree), nextClusterId(clusterID) {}

    void cluster(const std::vector<std::vector<double>>& points) {
        this->points = points;
        // Initialize all points as not visited
        visited.assign(points.size(), false);
        clusters.assign(points.size(), -1);
        for (const auto& point : points) {
            kdTree.insert(point);
        }
        //Call insertPoint for each point with index
        for (int i = 0; i < points.size(); i++) {
            if (!visited[i]) {
                insertPoint(points[i], i);
            }
        }
        //Update the cluster ID of the points in the KDTree
        for(int i = 0; i < points.size(); i++){
            int clusterID = clusters[i];
            kdTree.assignClusterID(points[i], clusterID);
        }
        //Optimize the merge_cluster_pairs by removing the duplicate pairs
        std::sort(merge_cluster_pairs.begin(), merge_cluster_pairs.end());
        merge_cluster_pairs.erase(std::unique(merge_cluster_pairs.begin(), merge_cluster_pairs.end()), merge_cluster_pairs.end());

        for(auto merge_cluster_pair : merge_cluster_pairs){
            // std::cout << "Merging clusters " << merge_cluster_pair.first << " to " << merge_cluster_pair.second << std::endl;
            kdTree.mergeClusters(merge_cluster_pair.first, merge_cluster_pair.second);
        }

        for(int i = 0; i < points.size(); i++){    
            // std::cout << "Point " << i << " is in cluster " << kdTree.getClusterId(points[i]) << std::endl;
            clusters[i] = kdTree.getClusterId(points[i]);
        }

    }

    std::vector<std::vector<double>> findCorePoints(const std::vector<std::vector<double>>& neighbors) {
        std::vector<std::vector<double>> corePoints;
        for(auto neighbor : neighbors){
            //TODO: Done
            auto neighbors_of_neighbor = kdTree.radiusSearch(neighbor, eps);
            if(neighbors_of_neighbor.size() >= minPts){
                corePoints.push_back(neighbor);
            }
        }
        return corePoints;
    }

    void insertPoint(const std::vector<double>& point, int index) {
        
        // Step 1.1: Find neighborhood of current new point
        auto neighbors = kdTree.radiusSearch(point, eps);
        // Debug
        // std::cout << "Found " << neighbors.size() << " neighbors." << " for index " << index << std::endl;
        
        // Step 1.2: Find the core points with in the neighborhood
        auto corePoints = findCorePoints(neighbors);
        // Step 1.3: If no core points is found, assign noise to the current point
        if(corePoints.empty()){
            //Assign noise to the current point
            clusters[index] = -1;
            visited[index] = true;
            return;
        }
        // Step 1.4: Get all the cluster IDs of the core points
        std::set<int> labels_of_core_points;
        for(auto core_point : corePoints){
            int label = getClusterId(core_point);
            //TODO: Done
            if(label != -1){
                labels_of_core_points.insert(label);
            }
        }
        // Debug
        // std::cout << "Labels of core points: " << labels_of_core_points.size() << std::endl;
        // for(auto label : labels_of_core_points){
        //     std::cout << " Label: " << label;
        // }
        // std::cout << std::endl;
        // Step 1.5: If none of the core point is labeled
        if(labels_of_core_points.size() == 0){
            modified_expandCluster(corePoints, index, nextClusterId++);
        }

        // Step 1.6: If all the core points are labeled
        else if(labels_of_core_points.size() == 1){
            modified_expandCluster(corePoints, index, *labels_of_core_points.begin());
        }

        // Step 1.7: If the core points are labeled differently
        else if(labels_of_core_points.size() > 1){
            modified_expandCluster(corePoints, index, nextClusterId++);
        }

    }

    void modified_expandCluster(const std::vector<std::vector<double>>& corePoints, int index, int clusterID){
        std::vector<double> point = corePoints[0];
       
        // Debug
        // std::cout << "Expanding cluster id " << clusterID << " on point index " << index << std::endl;
        if(!visited[index]){
            visited[index] = true;
            clusters[index] = clusterID;
            
            //Get the neighbors of the current point
            auto neighbors = kdTree.radiusSearch(point, eps);
            std::vector<int> neighbor_cluster_ids;
            for(auto neighbor : neighbors){
                //Find the cluster ID of the neighbor
                int neighbor_cluster_id = getClusterId(neighbor);
                int neighbor_index = std::distance(points.begin(), std::find(points.begin(), points.end(), neighbor));
                
                // std::cout << "Neighbor cluster id is " << neighbor_cluster_id << std::endl;
                clusters[neighbor_index] = clusterID;
                visited[neighbor_index] = true;
                    
                //TODO: Done Check Can we get the index of the neighbor
                if(neighbor_cluster_id == -1){
                    kdTree.assignClusterID(point, clusterID);
                    
                    // Debug
                    // std::cout << "Assigned cluster ID " << clusterID << " to point " << index << std::endl;
                    //Check if the neighbor is core point
                    auto neighbor_core_points = findCorePoints(kdTree.radiusSearch(neighbor, eps));
                    if(neighbor_core_points.size() >= minPts){
                        // std::cout << "Expanding again cluster " << clusterID << " with neighbor core point " << neighbor_index << std::endl;
                        modified_expandCluster(neighbor_core_points, neighbor_index, clusterID);
                    }
                }else{
                    //TODO: Done Assign the visited
                    auto neighbor_core_points = findCorePoints(kdTree.radiusSearch(neighbor, eps));
                    // Debug
                    // std::cout << "Neighbor core points size is " << neighbor_core_points.size() << std::endl;
                    if(neighbor_core_points.size() >= minPts){
                        // Debug
                        // std::cout << "Merging clusters " << neighbor_cluster_id << " and " << clusterID << std::endl;
                        merge_cluster_pairs.push_back(std::make_pair(neighbor_cluster_id, clusterID));
                    }
                }
            }
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
    std::vector<std::vector<double>> points;
    std::vector<std::pair<int, int>> merge_cluster_pairs;

};

#endif // INCDBSCAN_H