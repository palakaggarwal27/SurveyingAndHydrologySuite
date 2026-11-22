#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

// Defines shared data types, integration options, input points, and calculation results.

#include <cstddef>
#include <limits>
#include <string>
#include <vector>

const double kNaN = std::numeric_limits<double>::quiet_NaN();

enum class IntegrationMethod {
    TRAPEZOIDAL,
    SIMPSON_1_3,
    SIMPSON_3_8,
    AUTO_SELECT
};

struct DataPoint {
    double x;
    double y;
};

struct HydrologyPoint {
    double distance;
    double depth;
    double velocity;

    // Calculates unit discharge as depth multiplied by velocity.
    double getUnitDischarge() const {
        return depth * velocity;
    }
};

struct XtraResult {
    std::string label;
    double value;
    std::string unit;
};

struct MethodResult {
    std::string name;
    bool applicable = false;
    bool isBaseline = false;
    double value = kNaN;
    double absoluteError = kNaN;
    double relativeError = kNaN;
    double percentError = kNaN;
    double executionTimeMicroseconds = kNaN;
    double percentDiffVsBaseline = kNaN;
    std::string note;
};

struct CalculationResult {
    std::string calculationTitle;
    std::string methodUsed;
    double primaryResult = 0.0;
    std::string unit;
    std::size_t intervalCount = 0;
    double stepSize = 0.0;
    bool isUniformSpacing = false;
    double exactResult = kNaN;
    std::vector<XtraResult> extraMetrics;
    std::vector<MethodResult> methodComparison;
    std::vector<std::string> warnings;
};

#endif
