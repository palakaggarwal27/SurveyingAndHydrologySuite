#ifndef CALCULATION_SUPPORT_H
#define CALCULATION_SUPPORT_H

// Provides shared validation, integration selection, result construction, and error analysis.

#include "DataStructures.h"
#include <string>
#include <vector>

struct IntegralOutcome {
    double value;
    std::string methodName;
    std::string note;
};

class CalculationSupport {
public:
    // Extracts x-coordinates from a collection of data points.
    static std::vector<double> xOf(const std::vector<DataPoint>& pts);

    // Extracts y-coordinates from a collection of data points.
    static std::vector<double> yOf(const std::vector<DataPoint>& pts);

    // Validates series data and optionally requires non-negative y-values.
    static void validateSeries(const std::vector<double>& x, const std::vector<double>& y,const std::string& context, bool requireNonNegativeY = false);

    // Integrates a series using the selected method or a compatible fallback.
    static IntegralOutcome integrate(const std::vector<double>& x, const std::vector<double>& y,IntegrationMethod method);

    // Builds a calculation result for one integration method.
    static CalculationResult buildResult(const std::string& title, const std::string& unit,const std::vector<double>& x,const std::vector<double>& y,IntegrationMethod method,double exactResult = kNaN);

    // Runs all applicable integration methods and compares their results.
    static CalculationResult compareAllMethods(const std::string& title, const std::string& unit,const std::vector<double>& x,
const std::vector<double>& y,double exactResult = kNaN);

    // Applies absolute, relative, and percentage error analysis.
    static void applyErrorAnalysis(CalculationResult& result, double exactResult);
};

#endif
