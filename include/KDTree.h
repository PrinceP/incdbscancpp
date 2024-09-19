// KDTree.h
#ifndef KDTREE_H
#define KDTREE_H

#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <cmath>
#include <limits>

struct VectorHash {
    std::size_t operator()(const std::pair<std::vector<double>, double>& key) const {
        std::size_t seed = 0;
        for (double i : key.first) {
            seed ^= std::hash<double>()(i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        seed ^= std::hash<double>()(key.second) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

struct VectorEqual {
    bool operator()(const std::pair<std::vector<double>, double>& a, const std::pair<std::vector<double>, double>& b) const {
        return a.second == b.second && a.first == b.first;
    }
};


class KDTree {
public:
    struct Node {
        std::vector<double> point;
        std::shared_ptr<Node> left;
        std::shared_ptr<Node> right;
        int clusterId;
        int tick;
        int index;
        bool visited_node;

        Node(const std::vector<double>& pt, int idx) : point(pt), left(nullptr), right(nullptr), clusterId(-1), tick(0), visited_node(false), index(idx) {}
    };

    std::unordered_map<std::pair<std::vector<double>, double>, std::vector<std::vector<double>>, VectorHash, VectorEqual> radiusSearchCache;


    using NodePtr = std::shared_ptr<Node>;

    KDTree(int dimensions) : dimensions(dimensions), root(nullptr) {}

    std::vector<NodePtr> nodes;

    void insert(const std::vector<double>& point, int index) {
        NodePtr newNode = std::make_shared<Node>(point, index);
        nodes.push_back(newNode);
        root = insertRec(root, newNode, 0);
    }

    void remove(const std::vector<double>& point) {
        root = removeRec(root, point, 0);
    }

    std::vector<std::vector<double>> radiusSearch(const std::vector<double>& target, double radius) {
        std::vector<std::vector<double>> results;
        radiusSearchRec(root, target, radius, 0, results);
        return results;
    }
 
    std::vector<std::vector<double>> radiusSearchUsingCache(const std::vector<double>& target, double radius) {
        auto cacheKey = std::make_pair(target, radius);

        // Check if the result is already in the cache
        auto it = radiusSearchCache.find(cacheKey);
        if (it != radiusSearchCache.end()) {
            return it->second; // Return the cached result
        }

        // Perform the search if not cached
        std::vector<std::vector<double>> results;
        radiusSearchRec(root, target, radius, 0, results);

        // Store the result in the cache
        radiusSearchCache[cacheKey] = results;

        return results;
    }

    // Assign a cluster ID to a specific point
    void assignClusterID(const std::vector<double>& point, int clusterID) {
        NodePtr node = findNode(root, point, 0);
        if (node) {
            node->clusterId = clusterID;
        }
    }

    // Get the cluster ID for a specific point
    int getClusterId(const std::vector<double>& point) const {
        NodePtr node = findNode(root, point, 0);
        if(node) return node->clusterId;
        else return -1;
    }

    // Update the cluster ID for a specific point
    void updateClusterId(const std::vector<double>& point, int newClusterId) {
        NodePtr node = findNode(root, point, 0);
        if (node) {
            node->clusterId = newClusterId;
        }
        else {
            std::cout << "Point not found in the tree." << std::endl;
        }
    }
    //Get the size of the tree
    int size() const {
        return sizeRec(root);
    }

    //Set the visited node
    void setVisitedNode(const std::vector<double>& point, bool visited_node) {
        NodePtr node = findNode(root, point, 0);
        if (node) {
            node->visited_node = visited_node;
        }
    }

    //Check if the node is visited
    bool isVisitedNode(const std::vector<double>& point) const {
        NodePtr node = findNode(root, point, 0);
        if (node) {
            return node->visited_node;
        }
        return false;
    }

    //Set all nodes to unvisited
    void setAllNodesToUnvisited() {
        setAllNodesToUnvisitedRec(root);
    }

    //Set all nodes to unvisited recursively
    void setAllNodesToUnvisitedRec(NodePtr node) {
        if (node) {
            node->visited_node = false;
            setAllNodesToUnvisitedRec(node->left);
            setAllNodesToUnvisitedRec(node->right);
        }
    }

    //Merge clusterID1 to clusterID2
    void mergeClusters(int clusterID1, int clusterID2) {
        for (auto& node : nodes) {
            if (node->clusterId == clusterID1) {
                node->clusterId = clusterID2;
            }
        }
    }

    //Get the index of a point
    int getIndex(const std::vector<double>& point) const {
        NodePtr node = findNode(root, point, 0);
        if (node) {
            return node->index;
        }
        return -1;
    }

private:
    int dimensions;
    NodePtr root;

    int sizeRec(NodePtr node) const {
        if (!node) return 0;
        return 1 + sizeRec(node->left) + sizeRec(node->right);
    }

    NodePtr insertRec(NodePtr node, NodePtr newNode, int depth) {
        if (!node) return newNode;

        int axis = depth % dimensions;
        if (newNode->point[axis] < node->point[axis])
            node->left = insertRec(node->left, newNode, depth + 1);
        else
            node->right = insertRec(node->right, newNode, depth + 1);

        return node;
    }

    NodePtr removeRec(NodePtr node, const std::vector<double>& point, int depth) {
        if (!node) return nullptr;

        int axis = depth % dimensions;
        if (node->point == point) {
            if (!node->left)
                return node->right;
            else if (!node->right)
                return node->left;

            NodePtr minNode = findMin(node->right, axis, depth + 1);
            node->point = minNode->point;
            node->right = removeRec(node->right, minNode->point, depth + 1);
        } else if (point[axis] < node->point[axis]) {
            node->left = removeRec(node->left, point, depth + 1);
        } else {
            node->right = removeRec(node->right, point, depth + 1);
        }
        return node;
    }

    NodePtr findMin(NodePtr node, int axis, int depth) {
        if (!node) return nullptr;

        int currentAxis = depth % dimensions;
        if (currentAxis == axis) {
            if (!node->left) return node;
            return findMin(node->left, axis, depth + 1);
        }
        return minNode(node, findMin(node->left, axis, depth + 1), findMin(node->right, axis, depth + 1), axis);
    }

    NodePtr minNode(NodePtr a, NodePtr b, NodePtr c, int axis) {
        NodePtr res = a;
        if (b && (!res || b->point[axis] < res->point[axis])) res = b;
        if (c && (!res || c->point[axis] < res->point[axis])) res = c;
        return res;
    }

    void radiusSearchRec(NodePtr node, const std::vector<double>& target, double radius, int depth, std::vector<std::vector<double>>& results) const {
        if (!node) return;

        double dist = distance(node->point, target);
        if (dist <= radius) {
            results.push_back(node->point);
        }

        int axis = depth % dimensions;
        if (target[axis] - radius <= node->point[axis])
            radiusSearchRec(node->left, target, radius, depth + 1, results);
        if (target[axis] + radius >= node->point[axis])
            radiusSearchRec(node->right, target, radius, depth + 1, results);
    }

    NodePtr findNode(NodePtr node, const std::vector<double>& point, int depth) const {
        if (!node) return nullptr;

        if (node->point == point) return node;

        int axis = depth % dimensions;
        if (point[axis] < node->point[axis])
            return findNode(node->left, point, depth + 1);
        else
            return findNode(node->right, point, depth + 1);
    }

    double distance(const std::vector<double>& a, const std::vector<double>& b) const {
        double dist = 0.0;
        for (size_t i = 0; i < a.size(); ++i) {
            dist += (a[i] - b[i]) * (a[i] - b[i]);
        }
        return std::sqrt(dist);
    }
};

#endif
