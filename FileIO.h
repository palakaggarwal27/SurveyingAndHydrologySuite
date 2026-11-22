#ifndef FILE_IO_H
#define FILE_IO_H

// Provides CSV and text loading and calculation report export operations.

#include "DataStructures.h"
#include <string>
#include <vector>

class FileManager {
public:
    // Loads x-y coordinate points from a CSV or TXT file.
    static bool loadDataPoints(const std::string& filePath,std::vector<DataPoint>& outPoints,std::string* errorMessage = 0);

    // Loads distance, depth, and velocity points from a CSV or TXT file.
    static bool loadHydrologyPoints(const std::string& filePath,std::vector<HydrologyPoint>& outPoints,std::string* errorMessage = 0);

    // Exports a calculation report and x-y input data.
    static bool exportReport(const std::string& filePath,const CalculationResult& result,const std::vector<DataPoint>& inputPoints);

    // Exports a calculation report and river measurement data.
    static bool exportReport(const std::string& filePath,const CalculationResult& result,const std::vector<HydrologyPoint>& hydroPoints);
};

#endif
