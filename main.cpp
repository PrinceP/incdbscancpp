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

// Function to shuffle data and labels
void shuffleDataAndLabels(std::vector<std::vector<double>>& data, std::vector<std::string>& labels) {
    // Create a vector of indices
    std::vector<size_t> indices(data.size());
    std::iota(indices.begin(), indices.end(), 0);

    // Shuffle the indices
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(indices.begin(), indices.end(), g);

    // Create new vectors to store shuffled data
    std::vector<std::vector<double>> shuffledData(data.size());
    std::vector<std::string> shuffledLabels(labels.size());

    // Use the shuffled indices to reorder both data and labels
    for (size_t i = 0; i < indices.size(); ++i) {
        shuffledData[i] = data[indices[i]];
        shuffledLabels[i] = labels[indices[i]];
    }

    // Swap the original data and labels with the shuffled ones
    data.swap(shuffledData);
    labels.swap(shuffledLabels);
}

// Function to split data into parts
std::vector<std::vector<std::vector<double>>> splitData(const std::vector<std::vector<double>>& data, size_t partSize) {
    std::vector<std::vector<std::vector<double>>> parts;
    for (size_t i = 0; i < data.size(); i += partSize) {
        parts.push_back(std::vector<std::vector<double>>(data.begin() + i, data.begin() + std::min(i + partSize, data.size())));
    }
    return parts;
}

// Function to split labels into parts
std::vector<std::vector<std::string>> splitLabels(const std::vector<std::string>& labels, size_t partSize) {
    std::vector<std::vector<std::string>> parts;
    for (size_t i = 0; i < labels.size(); i += partSize) {
        parts.push_back(std::vector<std::string>(labels.begin() + i, labels.begin() + std::min(i + partSize, labels.size())));
    }
    return parts;
}

// Function to dump cluster results into a file
void dumpClustersToFile(const std::string& filename, const std::vector<std::vector<double>>& data, const std::vector<std::string>& labels, KDTree& kdTree) {
    std::ofstream outfile(filename, std::ios_base::app);
    for (size_t i = 0; i < data.size(); ++i) {
        std::string cluster = std::to_string(kdTree.getClusterId(data[i]));
        std::string result = labels[i] + "," + cluster;
        outfile << result << std::endl;
    }
    outfile.close();
}

int main() {

    std::string base_dir = "./../python/TESTING_SET/";
    
    std::vector<std::string> labels;
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<std::vector<float>> data = load_npy_files(base_dir, labels);
    std::vector<std::vector<double>> doubleData = convertToDouble(data);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> load_time = end - start;
    std::cout << "Time taken to load data: " << load_time.count() << " seconds." << std::endl;

    // Shuffle the data and labels
    shuffleDataAndLabels(doubleData, labels);

    // Split the data and labels into parts
    size_t partSize = 1000;
    auto dataParts = splitData(doubleData, partSize);
    auto labelParts = splitLabels(labels, partSize);

    // Log the sizes of the data and labels parts
    for (size_t i = 0; i < dataParts.size(); ++i) {
        std::cout << "Size of the data part " << i + 1 << ": " << dataParts[i].size() << std::endl;
        std::cout << "Size of the labels part " << i + 1 << ": " << labelParts[i].size() << std::endl;
    }

    double eps = 1.0;
    int minPts = 5;
    int dimensions = 512;
    KDTree kdTree(dimensions);
    std::cout << "KDTree size: " << kdTree.size() << std::endl;

    int clusterID = 0;
    DBSCAN dbscan(eps, minPts, kdTree, clusterID);
    std::cout << "DBSCAN declared with eps " << eps << ", minPts " << minPts << ", and " << dimensions << "D vectors" << std::endl;

    // Cluster the first part of the data using DBSCAN
    start = std::chrono::high_resolution_clock::now();
    dbscan.cluster(dataParts[0]);
    std::cout << "DBSCAN clustered" << std::endl;
    
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> fit_time = end - start;
    std::cout << "Time taken to fit DBSCAN: " << fit_time.count() << " seconds." << std::endl;
    
    dbscan.getLastClusterId(clusterID);
    // Dump the first set of cluster results into a file
    dumpClustersToFile("clusters.txt", dataParts[0], labelParts[0], kdTree);

    // INCDBSCAN
    INCDBSCAN incdbscan(eps, minPts, kdTree);
    std::cout << "INCDBSCAN declared with eps " << eps << ", minPts " << minPts << ", and " << dimensions << "D vectors" << std::endl;

    // Cluster the remaining parts of the data using INCDBSCAN
    for (size_t i = 1; i < dataParts.size(); ++i) {
        start = std::chrono::high_resolution_clock::now();
        incdbscan.cluster(dataParts[i], clusterID, i * partSize);
        std::cout << "INCDBSCAN clustered part " << i + 1 << std::endl;
        
        int lastClusterID;
        incdbscan.getLastClusterId(lastClusterID);
        clusterID = lastClusterID;

        // Dump the incremental cluster results into a file
        dumpClustersToFile("incclusters" + std::to_string(i) + ".txt", dataParts[i], labelParts[i], kdTree);

        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> incprint_time = end - start;
        std::cout << "Time taken to print clusters for part " << i + 1 << ": " << incprint_time.count() << " seconds." << std::endl;
    }

    // Dump combined results
    for (size_t i = 0; i < dataParts.size(); ++i) {
        dumpClustersToFile("combinedclusters.txt", dataParts[i], labelParts[i], kdTree);
    }

    return 0;
}