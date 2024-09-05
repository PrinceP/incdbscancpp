#include "DBSCAN.h"
#include "INCDBSCAN.h"
#include <iostream>
#include <random>
#include <chrono>
#include <iomanip>
#include <numeric>
#include <cmath>

#include "../vendor/npy.hpp"
#include <filesystem>

namespace fs = std::filesystem;

// Function to convert float data to double data
std::vector<std::vector<double>> convertToDouble(const std::vector<std::vector<float>>& data) {
    std::vector<std::vector<double>> result;
    result.reserve(data.size());
    for (const auto& vec : data) {
        result.emplace_back(vec.begin(), vec.end());
    }
    return result;
}


std::vector<std::vector<float>> load_npy_files(const std::string& base_dir, std::vector<std::string>& labels) {
    std::vector<std::vector<float>> data;

    for (const auto& moviestar_entry : fs::directory_iterator(base_dir)) {
        if (moviestar_entry.is_directory()) {
            std::string moviestar = moviestar_entry.path().filename().string();
            for (const auto& moviename_entry : fs::directory_iterator(moviestar_entry.path())) {
                if (moviename_entry.is_directory()) {
                    for (const auto& npy_file_entry : fs::directory_iterator(moviename_entry.path())) {
                        if (npy_file_entry.path().extension() == ".npy") {
                            std::string npy_path = npy_file_entry.path().string();
                            npy::npy_data d = npy::read_npy<float>(npy_path);  // Correct API call
                            std::vector<float> embedding = d.data;
                            data.push_back(embedding);
                            labels.push_back(moviestar);
                        }
                    }
                }
            }
        }
    }

    return data;
}


std::vector<std::vector<double>> generatePoints(int count, int dimensions) {
    std::vector<std::vector<double>> points;
    std::default_random_engine generator(std::chrono::system_clock::now().time_since_epoch().count());
    
    // Create 3 clusters and some noise
    std::vector<std::vector<double>> centers = {
        std::vector<double>(dimensions, 0.0),
        std::vector<double>(dimensions, 5.0),
        std::vector<double>(dimensions, 10.0)
    };

    std::normal_distribution<double> cluster_dist(0.0, 1.0);
    std::uniform_real_distribution<double> noise_dist(0.0, 15.0);

    for (int i = 0; i < count; ++i) {
        std::vector<double> point(dimensions);
        if (i < count * 0.9) {  // 90% of points in clusters
            int cluster = i % 3;
            for (int j = 0; j < dimensions; ++j) {
                point[j] = centers[cluster][j] + cluster_dist(generator);
            }
        } else {  // 10% noise
            for (int j = 0; j < dimensions; ++j) {
                point[j] = noise_dist(generator);
            }
        }
        points.push_back(point);
    }

    return points;
}

double euclideanDistance(const std::vector<double>& a, const std::vector<double>& b) {
    double sum = 0.0;
    for (size_t i = 0; i < a.size(); ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return std::sqrt(sum);
}

//IncDBSCAN
// void printClusterSummary(const DBSCAN& dbscan, const std::vector<std::vector<double>>& points) {
//     const auto& clusters = dbscan.getClusters();
//     std::cout << "Number of clusters: " << clusters.size() << std::endl;
//     for (const auto& [id, cluster] : clusters) {
//         std::cout << "Cluster " << id << " size: " << cluster.size() << std::endl;
//         if (!cluster.empty()) {
//             double sum = std::accumulate(cluster[0].begin(), cluster[0].end(), 0.0);
//             double mean = sum / cluster[0].size();
//             std::cout << "  First point mean value: " << std::fixed << std::setprecision(2) << mean << std::endl;
//         }
//     }
    
//     // Count noise points
//     int noise_count = 0;
//     for (const auto& point : points) {
//         bool in_cluster = false;
//         for (const auto& [id, cluster] : clusters) {
//             if (std::find(cluster.begin(), cluster.end(), point) != cluster.end()) {
//                 in_cluster = true;
//                 break;
//             }
//         }
//         if (!in_cluster) noise_count++;
//     }
//     std::cout << "Noise points: " << noise_count << std::endl;
//     std::cout << std::endl;
// }

int main() {

    std::string base_dir = "./../python/TESTING_SET/";
    
    std::vector<std::string> labels;
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<std::vector<float>> data = load_npy_files(base_dir, labels);
    std::vector<std::vector<double>> doubleData = convertToDouble(data);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> load_time = end - start;
    std::cout << "Time taken to load data: " << load_time.count() << " seconds." << std::endl;
    //Divide doubleData into 2 parts
    int sliced_index = 1000;
    std::vector<std::vector<double>> doubleData1(doubleData.begin(), doubleData.begin() + sliced_index);
    std::vector<std::vector<double>> doubleData2(doubleData.begin() + sliced_index, doubleData.begin() + 2*sliced_index);
    std::vector<std::vector<double>> doubleData3(doubleData.begin() + 2*sliced_index, doubleData.end());
    //Divide labels
    std::vector<std::string> labels1(labels.begin(), labels.begin() + sliced_index);
    std::vector<std::string> labels2(labels.begin() + sliced_index, labels.begin() + 2*sliced_index);
    std::vector<std::string> labels3(labels.begin() + 2*sliced_index, labels.end());

    //Size of the data
    std::cout << "Size of the data1: " << doubleData1.size() << std::endl;
    std::cout << "Size of the data2: " << doubleData2.size() << std::endl;
    std::cout << "Size of the data3: " << doubleData3.size() << std::endl;
    std::cout << "Size of the labels1: " << labels1.size() << std::endl;
    std::cout << "Size of the labels2: " << labels2.size() << std::endl;
    std::cout << "Size of the labels3: " << labels3.size() << std::endl;

    double eps = 1.0;
    int minPts = 5;
    int dimensions = 512;
    KDTree kdTree(dimensions);
    std::cout << "KDTree size: " << kdTree.size() << std::endl;

    int clusterID = 0;
    DBSCAN dbscan(eps, minPts, kdTree, clusterID);
    std::cout << "DBSCAN declared with eps " << eps << ", minPts " << minPts << ", and " << dimensions << "D vectors" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    dbscan.cluster(doubleData1);
    std::cout << "DBSCAN clustered" << std::endl;
    
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> fit_time = end - start;
    std::cout << "Time taken to fit DBSCAN: " << fit_time.count() << " seconds." << std::endl;


    start = std::chrono::high_resolution_clock::now();
    std::vector<int> clusterLabels;
    dbscan.getClustersLabels(clusterLabels, clusterID);
    // Dump it into a file
    for (size_t i = 0; i < doubleData1.size(); ++i) {
        std::string output_file = "clusters.txt";
        std::ofstream outfile(output_file, std::ios_base::app);
        std::string cluster = "Cluster " + std::to_string(clusterLabels[i]);
        std::string result = labels[i] + "," + cluster;
        outfile << result << std::endl;
        outfile.close();

    }
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> print_time = end - start;
    std::cout << "Time taken to print clusters: " << print_time.count() << " seconds." << std::endl;


    //INCDBSCAN
    eps = 1.1;
    std::cout << "Starting INCDBSCAN with clusterID " << clusterID << std::endl;
    std::cout << "KDTree size: " << kdTree.size() << std::endl;
    INCDBSCAN incdbscan(eps, minPts, kdTree, clusterID);
    std::cout << "INCDBSCAN declared with eps " << eps << ", minPts " << minPts << ", and " << dimensions << "D vectors" << std::endl;

    incdbscan.cluster(doubleData2);
    std::cout << "INCDBSCAN clustered" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    std::vector<int> incclusterLabels;
    int incclusterID;
    incdbscan.getClustersLabels(incclusterLabels, incclusterID);
    std::cout << "INCDBSCAN clusterID: " << incclusterID << std::endl;
    
    // Dump it into a file
    for (size_t i = 0; i < doubleData2.size(); ++i) {
        std::string output_file = "incclusters.txt";
        std::ofstream outfile(output_file, std::ios_base::app);
        std::string cluster = "Cluster " + std::to_string(incclusterLabels[i]);
        std::string result = labels2[i] + "," + cluster;
        outfile << result << std::endl;
        outfile.close();

    }
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> incprint_time = end - start;
    std::cout << "Time taken to print clusters: " << incprint_time.count() << " seconds." << std::endl;



    /*
    std::vector<std::vector<double>> points = generatePoints(15, dimensions);

    // Print some diagnostic information
    std::cout << "Diagnostic Information:" << std::endl;
    std::cout << "Number of points: " << points.size() << std::endl;
    
    // Calculate and print the distance between the first two points
    if (points.size() >= 2) {
        double distance = euclideanDistance(points[0], points[1]);
        std::cout << "Distance between first two points: " << distance << std::endl;
    }

    // Count how many points are within eps distance of the first point
    int nearby_points = 0;
    for (size_t i = 1; i < points.size(); ++i) {
        if (euclideanDistance(points[0], points[i]) <= eps) {
            nearby_points++;
        }
    }
    std::cout << "Points within eps distance of the first point: " << nearby_points << std::endl;

    std::cout << "\nInitial Clustering:" << std::endl;
    // Perform initial clustering
    dbscan.cluster(points);
    for (const auto& [id, cluster] : dbscan.getClusters()) {
        std::cout << "Cluster " << id << ": ";
        for (const auto& point : cluster) {
            std::cout << "(" << point[0] << " -- " << point[511] << ") ";
        }
        std::cout << std::endl;
    }
    printClusterSummary(dbscan, points);
    std::cout << std::endl;
    
    // Insert a new point
    std::vector<double> newPoint(dimensions, 2.5);  // A point with all values set to 2.5
    dbscan.insertPoint(newPoint);
    points.push_back(newPoint);
    std::cout << "\nClustering after inserting a new point:" << std::endl;
    std::cout << "(" << points[points.size()-1][0] << " -- " << points[points.size()-1][511] << ") \n";
    for (const auto& [id, cluster] : dbscan.getClusters()) {
        std::cout << "Cluster " << id << ": ";
        for (const auto& point : cluster) {
            std::cout << "(" << point[0] << " -- " << point[511] << ") ";
        }
        std::cout << std::endl;
    }
    printClusterSummary(dbscan, points);
    std::cout << std::endl;

    // Remove a point
    std::cout << "\nClustering after removing point:" << std::endl;
    dbscan.removePoint(points[0]);
    std::cout << "(" << points[0][0] << " -- " << points[0][511] << ") \n";
    
    points.erase(points.begin());
    for (const auto& [id, cluster] : dbscan.getClusters()) {
        std::cout << "Cluster " << id << ": ";
        for (const auto& point : cluster) {
            std::cout << "(" << point[0] << " -- " << point[511] << ") ";
        }
        std::cout << std::endl;
    }
    printClusterSummary(dbscan, points);
    std::cout << std::endl;
    */   
    return 0;
}