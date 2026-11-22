#include "FileIO.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <cctype>
#include <stdexcept>

static std::string trim(const std::string& value) {
    const std::string whitespace = " \t\r\n";
    const std::size_t first = value.find_first_not_of(whitespace);
    if (first == std::string::npos) return "";
    const std::size_t last = value.find_last_not_of(whitespace);
    return value.substr(first, last - first + 1);
}

static bool parseNumber(const std::string& text, double& value) {
    try {
        std::size_t consumed = 0;
        const std::string cleaned = trim(text);
        value = std::stod(cleaned, &consumed);
        return consumed == cleaned.size() && std::isfinite(value);
    } catch (const std::exception&) {
        return false;
    }
}

static void setError(std::string* errorMessage, const std::string& message) {
    if (errorMessage != 0) *errorMessage = message;
}

bool FileManager::loadDataPoints(const std::string& filePath, std::vector<DataPoint>& outPoints, std::string* errorMessage) {
    outPoints.clear();
    std::ifstream file(filePath);
    if (!file.is_open()) {
        setError(errorMessage, "Could not open the file.");
        return false;
    }

    std::string line;
    std::size_t lineNumber = 0;
    while (std::getline(file, line)) {
        ++lineNumber;
        const std::string cleanedLine = trim(line);
        if (cleanedLine.empty() || cleanedLine[0] == '#') continue;
        std::stringstream ss(line);
        std::string xStr, yStr, extra;
        if (!std::getline(ss, xStr, ',') || !std::getline(ss, yStr, ',') ||
            std::getline(ss, extra, ',')) {
            setError(errorMessage, "Malformed CSV at line " + std::to_string(lineNumber) +
                ": expected exactly two numeric columns.");
            outPoints.clear();
            return false;
        }
        double x = 0.0, y = 0.0;
        if (!parseNumber(xStr, x) || !parseNumber(yStr, y)) {
            setError(errorMessage, "Malformed CSV at line " + std::to_string(lineNumber) +
                ": values must be finite numbers.");
            outPoints.clear();
            return false;
        }
        if (!outPoints.empty() && x <= outPoints.back().x) {
            setError(errorMessage, "Invalid x-values at line " + std::to_string(lineNumber) +
                ": x values must be strictly increasing.");
            outPoints.clear();
            return false;
        }
        outPoints.push_back({ x, y });
    }
    if (outPoints.size() < 2) {
        setError(errorMessage, "The file must contain at least two data rows.");
        return false;
    }
    return true;
}

bool FileManager::loadHydrologyPoints(const std::string& filePath, std::vector<HydrologyPoint>& outPoints, std::string* errorMessage) {
    outPoints.clear();
    std::ifstream file(filePath);
    if (!file.is_open()) {
        setError(errorMessage, "Could not open the file.");
        return false;
    }

    std::string line;
    std::size_t lineNumber = 0;
    while (std::getline(file, line)) {
        ++lineNumber;
        const std::string cleanedLine = trim(line);
        if (cleanedLine.empty() || cleanedLine[0] == '#') continue;
        std::stringstream ss(line);
        std::string dStr, depStr, vStr, extra;
        if (!std::getline(ss, dStr, ',') || !std::getline(ss, depStr, ',') ||
            !std::getline(ss, vStr, ',') || std::getline(ss, extra, ',')) {
            setError(errorMessage, "Malformed CSV at line " + std::to_string(lineNumber) +
                ": expected exactly three numeric columns.");
            outPoints.clear();
            return false;
        }
        double distance = 0.0, depth = 0.0, velocity = 0.0;
        if (!parseNumber(dStr, distance) || !parseNumber(depStr, depth) ||
            !parseNumber(vStr, velocity)) {
            setError(errorMessage, "Malformed CSV at line " + std::to_string(lineNumber) +
                ": values must be finite numbers.");
            outPoints.clear();
            return false;
        }
        if (depth < 0.0) {
            setError(errorMessage, "Invalid physical measurement at line " +
                std::to_string(lineNumber) + ": depth cannot be negative.");
            outPoints.clear();
            return false;
        }
        if (!outPoints.empty() && distance <= outPoints.back().distance) {
            setError(errorMessage, "Invalid x-values at line " + std::to_string(lineNumber) +
                ": distances must be strictly increasing.");
            outPoints.clear();
            return false;
        }
        outPoints.push_back({ distance, depth, velocity });
    }
    if (outPoints.size() < 2) {
        setError(errorMessage, "The file must contain at least two data rows.");
        return false;
    }
    return true;
}







static void writeNumber(std::ostream& out, double value, int width, int digits) {
    if (value != value)
        out << std::setw(width) << "-";
    else
        out << std::setw(width) << std::fixed << std::setprecision(digits) << value;
}


static void writeResultSection(std::ostream& file, const CalculationResult& result) {
    file << "====================================================\n";
    file << "         NUMERICAL INTEGRATION SUITE REPORT         \n";
    file << "====================================================\n";
    file << "Calculation Target : " << result.calculationTitle << "\n";
    file << "Method Used        : " << result.methodUsed << "\n";
    file << "Primary Result     : " << std::fixed << std::setprecision(4)
         << result.primaryResult << " " << result.unit << "\n";
    file << "Exact Result       : ";
    writeNumber(file, result.exactResult, 0, 4);
    file << " " << result.unit << "\n";
    file << "Subintervals (n)   : " << result.intervalCount << "\n";
    if (result.isUniformSpacing)
        file << "Step size (h)      : " << result.stepSize << "\n";
    else
        file << "Step size (h)      : spacing is not uniform\n";
    
    if (!result.extraMetrics.empty()) {
        file << "\nAdditional results:\n";
        for (std::size_t i = 0; i < result.extraMetrics.size(); ++i) {
            file << "  " << result.extraMetrics[i].label << " : "
                 << std::fixed << std::setprecision(4) << result.extraMetrics[i].value
                 << " " << result.extraMetrics[i].unit << "\n";
        }
    }

    
    if (!result.warnings.empty()) {
        file << "\nNotes / warnings:\n";
        for (std::size_t i = 0; i < result.warnings.size(); ++i)
            file << "  - " << result.warnings[i] << "\n";
    }

    
    if (!result.methodComparison.empty()) {
        file << "\nMethod comparison:\n";
        file << std::left << std::setw(44) << "Method" << std::right
             << std::setw(14) << "Value" << std::setw(12) << "Abs error"
             << std::setw(12) << "Rel error" << std::setw(12) << "% error"
             << std::setw(14) << "Time (us)"
             << "\n";
        for (std::size_t i = 0; i < result.methodComparison.size(); ++i) {
            const MethodResult& m = result.methodComparison[i];
            std::string name = m.name;
            if (m.isBaseline) name = name + " (baseline)";
            file << std::left << std::setw(44) << name << std::right;
            if (!m.applicable) {
                file << "  N/A - " << m.note << "\n";
                continue;
            }
            writeNumber(file, m.value, 14, 4);
            writeNumber(file, m.absoluteError, 12, 4);
            writeNumber(file, m.relativeError, 12, 6);
            writeNumber(file, m.percentError, 12, 4);
            writeNumber(file, m.executionTimeMicroseconds, 14, 3);
            file << "\n";
        }
    }
    file << "\n";
}

static bool isCsvPath(const std::string& filePath) {
    if (filePath.size() < 4) return false;
    std::string suffix = filePath.substr(filePath.size() - 4);
    for (std::size_t i = 0; i < suffix.size(); ++i)
        suffix[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(suffix[i])));
    return suffix == ".csv";
}

static std::string csvNumber(double value) {
    std::ostringstream text;
    text << std::setprecision(12) << value;
    return text.str();
}

static void appendCsvCommentField(std::ostream& file, bool& firstField,const std::string& name, const std::string& value) {
    if (!firstField) file << ", ";
    file << name << ": " << value;
    firstField = false;
}

static void writeCsvReportComments(std::ostream& file, const CalculationResult& result) {
    file << "# Numerical Integration Suite report\n";
    file << "# calculation: " << result.calculationTitle << "\n";
    file << "# primary_result: " << std::setprecision(12) << result.primaryResult
         << " " << result.unit << "\n";
    if (result.exactResult == result.exactResult)
        file << "# exact_result: " << result.exactResult << " " << result.unit << "\n";
    else
        file << "# exact_result: not supplied\n";
    file << "# method comparison:\n";
    for (std::size_t i = 0; i < result.methodComparison.size(); ++i) {
        const MethodResult& method = result.methodComparison[i];
        bool firstField = true;
        file << "# ";
        appendCsvCommentField(file, firstField, "method", method.name);
        appendCsvCommentField(file, firstField, "applicable", method.applicable ? "true" : "false");
        if (method.value == method.value)
            appendCsvCommentField(file, firstField, "calculated_result", csvNumber(method.value));
        if (method.absoluteError == method.absoluteError)
            appendCsvCommentField(file, firstField, "absolute_error", csvNumber(method.absoluteError));
        if (method.relativeError == method.relativeError)
            appendCsvCommentField(file, firstField, "relative_error", csvNumber(method.relativeError));
        if (method.percentError == method.percentError)
            appendCsvCommentField(file, firstField, "percentage_error", csvNumber(method.percentError));
        if (method.executionTimeMicroseconds == method.executionTimeMicroseconds)
            appendCsvCommentField(file, firstField, "execution_time_microseconds",
                                  csvNumber(method.executionTimeMicroseconds));
        if (method.isBaseline)
            appendCsvCommentField(file, firstField, "baseline", "true");
        if (!method.note.empty())
            appendCsvCommentField(file, firstField, "note", method.note);
        file << "\n";
    }
}

bool FileManager::exportReport(const std::string& filePath,const CalculationResult& result,const std::vector<DataPoint>& inputPoints) {
    std::ofstream file(filePath);
    if (!file.is_open()) return false;

    if (isCsvPath(filePath)) {
        writeCsvReportComments(file, result);
        for (std::size_t i = 0; i < inputPoints.size(); ++i)
            file << std::setprecision(12) << inputPoints[i].x << "," << inputPoints[i].y << "\n";
        return file.good();
    }

    writeResultSection(file, result);

    
    file.unsetf(std::ios::floatfield);
    file.precision(10);

    file << "Point Data (x, y):\n";
    file << "Index, x, y\n";
    for (size_t i = 0; i < inputPoints.size(); ++i) {
        file << i + 1 << ", " << inputPoints[i].x << ", " << inputPoints[i].y << "\n";
    }
    return file.good();
}

bool FileManager::exportReport(const std::string& filePath,const CalculationResult& result,const std::vector<HydrologyPoint>& hydroPoints) {
    std::ofstream file(filePath);
    if (!file.is_open()) return false;

    if (isCsvPath(filePath)) {
        writeCsvReportComments(file, result);
        for (std::size_t i = 0; i < hydroPoints.size(); ++i)
            file << std::setprecision(12) << hydroPoints[i].distance << "," << hydroPoints[i].depth << ","
            << hydroPoints[i].velocity << "\n";
        return file.good();
    }

    writeResultSection(file, result);

    
    file.unsetf(std::ios::floatfield);
    file.precision(10);

    file << "River Data:\n";
    file << "Index, Distance (m), Depth (m), Velocity (m/s), q = depth x velocity (m^2/s)\n";
    for (size_t i = 0; i < hydroPoints.size(); ++i) {
        file << i + 1 << ", " << hydroPoints[i].distance << ", " << hydroPoints[i].depth << ", "
             << hydroPoints[i].velocity << ", " << hydroPoints[i].getUnitDischarge() << "\n";
    }
    return file.good();
}
