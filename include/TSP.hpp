#ifndef UTILS_INCLUDE_TSP_HPP
#define UTILS_INCLUDE_TSP_HPP

#include <fmt/core.h>

#include <cassert>
#include <cmath>
#include <fstream>
#include <sstream>
#include <vector>

#include "StringUtils.hpp"

struct customer {
    double x = 0.0;
    double y = 0.0;
};

struct TSPInstance {

    std::string filename;

    std::string name;
    std::string comment;
    std::string type;
    std::string edge_type;

    int dimension;
    customer* customers = nullptr;

    int ecount;
    int* elist = nullptr;
    int* elength = nullptr;

public:
    explicit TSPInstance(std::string filename_) : filename(filename_) {
        std::ifstream input_file(filename);

        if (!input_file.is_open()) { throw std::string("Error opening file " + filename); }

        std::string key;
        std::string line;
        while (std::getline(input_file, line)) {

            std::istringstream line_stream(line);
            std::getline(line_stream, key, ':');
            trim(key, " \t\n\r\f\v");

            fmt::print(key + '\n');

            if (key.empty()) { continue; }

            if (key == "EOF") {
                break;
            } else if (key == "NAME") {
                line_stream >> name;
            } else if (key == "COMMENT") {
                getline(line_stream, comment, '\n');
            } else if (key == "TYPE") {
                line_stream >> type;
            } else if (key == "DIMENSION") {
                line_stream >> dimension;
                customers = new customer[dimension];
            } else if (key == "EDGE_WEIGHT_TYPE") {
                line_stream >> edge_type;
            } else if (key == "DISPLAY_DATA_TYPE") {
                continue;
            } else if (key == "NODE_COORD_SECTION") {
                for (int i = 0, idx; i < dimension; ++i) {
                    input_file >> idx;
                    input_file >> customers[idx - 1].x >> customers[idx - 1].y;
                }
            } else {
                throw std::string("Unexpected data in input file: " + key);
            }
        }

        fmt::print("\nInstance Information --------------------------------------------------------------------------\n");
        fmt::print("NAME: {}\n", name);
        fmt::print("COMMENT: {}\n", comment);
        fmt::print("TYPE: {}\n", type);
        fmt::print("DIMENSION: {}\n", dimension);
        fmt::print("EDGE_WEIGHT_TYPE: {}\n", edge_type);
        fmt::print("-----------------------------------------------------------------------------------------------\n");

        ecount = (dimension * (dimension - 1)) / 2;
        elist = new int[ecount * 2];
        elength = new int[ecount];

        int edge = 0;
        int edge_w = 0;
        for (int i = 0; i < dimension; ++i) {
            for (int j = i + 1; j < dimension; ++j) {
                assert(edge_w < ecount);
                elist[edge++] = i;
                elist[edge++] = j;
                elength[edge_w++] = dist(i, j);
            }
        }
    }

    ~TSPInstance() {
        delete[] customers;
        delete[] elist;
        delete[] elength;
    }

    // classic euclidean distance
    int dist(int i, int j) {
        double t1 = customers[i].x - customers[j].x;
        double t2 = customers[i].y - customers[j].y;
        return (int)(std::sqrt(t1 * t1 + t2 * t2) + 0.5);
    }

    int xpos_sym(int i, int j) {
        if (i > j) return xpos_sym(j, i);
        else
            return (i * dimension + j - ((i + 1) * (i + 2)) / 2);
    }
};

#endif