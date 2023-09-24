#pragma once 

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <cstdlib>


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
        if (line.size() < 3) {
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
        lc.a = std::strtod(line[2].c_str(), &endptr);
        if (line.size() >= 4) {
            lc.b = std::strtod(line[3].c_str(), &endptr);
        }


        lc.valid = true;
        return lc;
    }
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

    bool writeRow(const std::vector<std::string>& row) 
    {
        std::ofstream file(filename, std::ios_base::app); // 使用追加模式打开文件

        if (!file.is_open()) {
            std::cerr << "Failed to open file for writing: " << filename << std::endl;
            return false;
        }

        for (auto it = row.begin(); it != row.end(); ++it) {
            file << *it;
            if (it + 1 != row.end()) {
                file << delimiter;
            }
        }
        file << std::endl;

        file.close();
        return true;
    }

    bool writeRow(const std::string& op_name, const std::string& fmt, const std::string& reported_fmt,
                      double a, double b, double output, const std::vector<double>& cpi_list) {
        std::ostringstream oss;

        oss << std::setw(5) << op_name << ", ";
        oss << std::setw(4) << fmt << ", ";
        oss << std::setw(4) << reported_fmt << ", ";
        oss << std::setw(10) << std::scientific << std::setprecision(1) << a << ", ";
        oss << std::setw(10) << std::scientific << std::setprecision(1) << b << ", ";
        oss << std::setw(10) << std::scientific << std::setprecision(1) << output << ", ";
        
        for (size_t i = 0; i < cpi_list.size(); ++i) {
            oss << std::fixed << std::setw(6) << std::setprecision(2) << cpi_list[i];
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