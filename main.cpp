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
    std::vector<std::vector<double>> doubleData3(doubleData.begin() + 2*sliced_index, doubleData.begin() + 3*sliced_index);
    std::vector<std::vector<double>> doubleData4(doubleData.begin() + 3*sliced_index, doubleData.end());
    //Divide labels
    std::vector<std::string> labels1(labels.begin(), labels.begin() + sliced_index);
    std::vector<std::string> labels2(labels.begin() + sliced_index, labels.begin() + 2*sliced_index);
    std::vector<std::string> labels3(labels.begin() + 2*sliced_index, labels.begin() + 3*sliced_index);
    std::vector<std::string> labels4(labels.begin() + 3*sliced_index, labels.end());

    //Size of the data
    std::cout << "Size of the data1: " << doubleData1.size() << std::endl;
    std::cout << "Size of the data2: " << doubleData2.size() << std::endl;
    std::cout << "Size of the data3: " << doubleData3.size() << std::endl;
    std::cout << "Size of the data4: " << doubleData4.size() << std::endl;  
    std::cout << "Size of the labels1: " << labels1.size() << std::endl;
    std::cout << "Size of the labels2: " << labels2.size() << std::endl;
    std::cout << "Size of the labels3: " << labels3.size() << std::endl;
    std::cout << "Size of the labels4: " << labels4.size() << std::endl;
    // Randomize the data and labels for doubleData1 and labels1
    // Create a vector of indices
    std::vector<size_t> indices(doubleData1.size());
    std::iota(indices.begin(), indices.end(), 0);

    // Shuffle the indices
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(indices.begin(), indices.end(), g);

    // Create new vectors to store shuffled data
    std::vector<std::vector<double>> shuffled_doubleData1(doubleData1.size());
    std::vector<std::string> shuffled_labels1(labels1.size());

    // Use the shuffled indices to reorder both doubleData1 and labels1
    for (size_t i = 0; i < indices.size(); ++i) {
        shuffled_doubleData1[i] = doubleData1[indices[i]];
        shuffled_labels1[i] = labels1[indices[i]];
    }
    std::cout << "Size of the shuffled_doubleData1: " << shuffled_doubleData1.size() << std::endl;
    std::cout << "Size of the shuffled_labels1: " << shuffled_labels1.size() << std::endl;


    //Randomize the data and labels for doubleData2 and labels2
    // Create a vector of indices
    std::vector<size_t> indices2(doubleData2.size());
    std::iota(indices2.begin(), indices2.end(), 0);

    // Shuffle the indices
    std::shuffle(indices2.begin(), indices2.end(), g);
    
    // Create new vectors to store shuffled data
    std::vector<std::vector<double>> shuffled_doubleData2(doubleData2.size());
    std::vector<std::string> shuffled_labels2(labels2.size());

    // Use the shuffled indices to reorder both doubleData2 and labels2
    for (size_t i = 0; i < indices2.size(); ++i) {
        shuffled_doubleData2[i] = doubleData2[indices2[i]];
        shuffled_labels2[i] = labels2[indices2[i]];
    }
    std::cout << "Size of the shuffled_doubleData2: " << shuffled_doubleData2.size() << std::endl;
    std::cout << "Size of the shuffled_labels2: " << shuffled_labels2.size() << std::endl;

    //Randomize the data and labels for doubleData3 and labels3
    // Create a vector of indices
    std::vector<size_t> indices3(doubleData3.size());
    std::iota(indices3.begin(), indices3.end(), 0);

    // Shuffle the indices
    std::shuffle(indices3.begin(), indices3.end(), g);

    // Create new vectors to store shuffled data
    std::vector<std::vector<double>> shuffled_doubleData3(doubleData3.size());
    std::vector<std::string> shuffled_labels3(labels3.size());

    // Use the shuffled indices to reorder both doubleData3 and labels3
    for (size_t i = 0; i < indices3.size(); ++i) {
        shuffled_doubleData3[i] = doubleData3[indices3[i]];
        shuffled_labels3[i] = labels3[indices3[i]];
    }
    std::cout << "Size of the shuffled_doubleData3: " << shuffled_doubleData3.size() << std::endl;
    std::cout << "Size of the shuffled_labels3: " << shuffled_labels3.size() << std::endl;


    double eps = 1.0;
    int minPts = 5;
    int dimensions = 512;
    KDTree kdTree(dimensions);
    std::cout << "KDTree size: " << kdTree.size() << std::endl;

    int clusterID = 0;
    DBSCAN dbscan(eps, minPts, kdTree, clusterID);
    std::cout << "DBSCAN declared with eps " << eps << ", minPts " << minPts << ", and " << dimensions << "D vectors" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    dbscan.cluster(shuffled_doubleData1);
    std::cout << "DBSCAN clustered" << std::endl;
    
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> fit_time = end - start;
    std::cout << "Time taken to fit DBSCAN: " << fit_time.count() << " seconds." << std::endl;


    start = std::chrono::high_resolution_clock::now();
    std::vector<int> clusterLabels;
    dbscan.getClustersLabels(clusterLabels, clusterID);
    // Dump it into a file
    for (size_t i = 0; i < shuffled_doubleData1.size(); ++i) {
        std::string output_file = "clusters.txt";
        std::ofstream outfile(output_file, std::ios_base::app);
        std::string cluster = std::to_string(kdTree.getClusterId(shuffled_doubleData1[i]));
        std::string result = shuffled_labels1[i] + "," + cluster;
        outfile << result << std::endl;
        outfile.close();
    }
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> print_time = end - start;
    std::cout << "Time taken to print clusters: " << print_time.count() << " seconds." << std::endl;

    std::cout << "-----------------------------------" << std::endl;
    //INCDBSCAN
    eps = 1.0;
    int startingIndex = sliced_index;
    std::cout << "Starting INCDBSCAN with clusterID " << clusterID << " startingIndex " << startingIndex << std::endl;
    INCDBSCAN incdbscan(eps, minPts, kdTree);
    std::cout << "INCDBSCAN declared with eps " << eps << ", minPts " << minPts << ", and " << dimensions << "D vectors" << std::endl;

    incdbscan.cluster(shuffled_doubleData2, clusterID, startingIndex);
    std::cout << "INCDBSCAN clustered" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    std::vector<int> incclusterLabels;
    int incclusterID;
    // incdbscan.getClustersLabels(incclusterLabels, incclusterID);
    incdbscan.getLastClusterId(incclusterID);
    std::cout << "INCDBSCAN clusterID: " << incclusterID << std::endl;
    
    // Dump it into a file
    for (size_t i = 0; i < shuffled_doubleData2.size(); ++i) {
        std::string output_file = "incclusters.txt";
        std::ofstream outfile(output_file, std::ios_base::app);
        std::string cluster = std::to_string(kdTree.getClusterId(shuffled_doubleData2[i]));
        std::string result = shuffled_labels2[i] + "," + cluster;
        outfile << result << std::endl;
        outfile.close();
    }
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> incprint_time = end - start;
    std::cout << "Time taken to print clusters: " << incprint_time.count() << " seconds." << std::endl;
    std::cout << "-----------------------------------" << std::endl;

    startingIndex = 2*sliced_index;
    clusterID = incclusterID;
    
    incdbscan.cluster(shuffled_doubleData3, clusterID, startingIndex);
    std::cout << "INCDBSCAN clustered" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    std::vector<int> incclusterLabels3;
    int incclusterID3;
    // incdbscan.getClustersLabels(incclusterLabels3, incclusterID3);
    incdbscan.getLastClusterId(incclusterID3);
    std::cout << "INCDBSCAN clusterID: " << incclusterID3 << std::endl;

    // Dump it into a file
    for (size_t i = 0; i < shuffled_doubleData3.size(); ++i) {
        std::string output_file = "incclusters2.txt";
        std::ofstream outfile(output_file, std::ios_base::app);
        std::string cluster = std::to_string(kdTree.getClusterId(shuffled_doubleData3[i]));
        std::string result = shuffled_labels3[i] + "," + cluster;
        outfile << result << std::endl;
        outfile.close();
    }

    for(int i = 0; i < shuffled_doubleData1.size(); i++){
        std::string output_file = "combinedclusters.txt";
        std::ofstream outfile(output_file, std::ios_base::app);
        std::string cluster = std::to_string(kdTree.getClusterId(shuffled_doubleData1[i]));
        std::string result = shuffled_labels1[i] + "," + cluster;
        outfile << result << std::endl;
        outfile.close();
    }
    for(int i = 0; i < shuffled_doubleData2.size(); i++){
        std::string output_file = "combinedclusters.txt";
        std::ofstream outfile(output_file, std::ios_base::app);
        std::string cluster = std::to_string(kdTree.getClusterId(shuffled_doubleData2[i]));
        std::string result = shuffled_labels2[i] + "," + cluster;
        outfile << result << std::endl;
        outfile.close();
    }
    for(int i = 0; i < shuffled_doubleData3.size(); i++){
        std::string output_file = "combinedclusters.txt";
        std::ofstream outfile(output_file, std::ios_base::app);
        std::string cluster = std::to_string(kdTree.getClusterId(shuffled_doubleData3[i]));
        std::string result = shuffled_labels3[i] + "," + cluster;
        outfile << result << std::endl;
        outfile.close();
    }
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> incprint_time3 = end - start;
    std::cout << "Time taken to print clusters: " << incprint_time3.count() << " seconds." << std::endl;
    
    
    return 0;
}