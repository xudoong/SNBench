#pragma once 

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <cstdlib>

const int MAX_N = 100000;


struct LineContent
{
    std::string op_name;
    std::string format;
    double a = 0;
    double b = 0;
    bool valid;
};


class CSVReader {
private:
    std::string filename;
    char delimiter;

public:
    CSVReader(const std::string& filename, char delimiter = ',')
        : filename(filename), delimiter(delimiter) {}

    std::vector<std::vector<std::string>> readData() 
    {
        std::vector<std::vector<std::string>> data;
        std::ifstream file(filename);

        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return data;
        }

        std::string line;
        while (std::getline(file, line)) {
            std::vector<std::string> row;
            std::istringstream lineStream(line);
            std::string cell;

            while (std::getline(lineStream, cell, delimiter)) {
                row.push_back(cell);
            }

            data.push_back(row);
        }

        file.close();
        return data;
    }

    LineContent parse_line(const std::vector<std::string> &line)
    {
        LineContent lc;
        if (line.size() < 4) {
            lc.valid = false;
            return lc;
        }
        if (line[0] == "OP") {
            lc.valid = false;
            return lc;
        }
        lc.op_name = line[0];
        lc.format = line[1];
        if (line[2] == "N/A") {
            lc.valid = false;
            return lc;
        }
        char *endptr;
        lc.a = std::strtod(line[3].c_str(), &endptr);
        if (line.size() >= 5) {
            lc.b = std::strtod(line[4].c_str(), &endptr);
        }

        lc.valid = true;
        return lc;
    }
};

struct InputData
{
    double *a_list;
    double *b_list;
    int size = 0;

    InputData() {
        a_list = (double *) aligned_alloc(64, MAX_N * sizeof(double));
        b_list = (double *) aligned_alloc(64, MAX_N * sizeof(double));
    }
    void push_back(double a, double b) {
        a_list[size] = a;
        b_list[size] = b;
        size++;
    }
};

class RandomInputInit
{
public:
    static void init_from_csv(std::string file_path, std::string op_name, std::vector<std::string> &op_order, std::map<std::string, InputData> &input_map)
    {
        CSVReader reader(file_path);
        auto lines = reader.readData();
        for (auto line : lines) {
            LineContent lc = reader.parse_line(line);
            if (lc.valid && lc.op_name == op_name) {
                std::string op_fmt = lc.op_name + "," + lc.format;
                if (op_order.empty() || op_order.back() != op_fmt)
                    op_order.push_back(op_fmt);
                input_map[op_fmt].push_back(lc.a, lc.b);
            }
        }

        // append to the size of MAXN
        for (auto op_fmt: op_order) {
            auto &input_data = input_map[op_fmt];
            const int init_size = input_data.size;
            for (int i = 0; i < MAX_N - init_size; i++) {
                input_data.push_back(input_data.a_list[i%init_size], input_data.b_list[i%init_size]);
        }
        }
    };
};


class CSVWriter {
private:
    std::string filename;
    char delimiter;

public:
    CSVWriter(const std::string& filename, char delimiter = ',')
        : filename(filename), delimiter(delimiter) {
            // std::ofstream file(filename, std::ios_base::trunc);
        }

    bool writeRow(const std::string& op_fmt, const std::vector<double>& cpi_list) {
        std::ostringstream oss;

        oss << op_fmt << ", ";
        for (size_t i = 0; i < cpi_list.size(); ++i) {
            oss << std::fixed << std::setw(6) << std::setprecision(1) << cpi_list[i];
            if (i < cpi_list.size() - 1) {
                oss << ", ";
            }
        }

        std::ofstream file(filename, std::ios_base::app);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for writing: " << filename << std::endl;
            return false;
        }

        file << oss.str() << std::endl;
        file.close();
        return true;
    }
};