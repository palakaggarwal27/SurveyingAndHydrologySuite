#ifndef INTEGRATION_ALGORITHMS_H
#define INTEGRATION_ALGORITHMS_H

// Implements trapezoidal, Simpson, and adaptive composite numerical integration.

#include "DataStructures.h"
#include <vector>

class IntegrationEngine {
private:
    static constexpr double SPACING_TOLERANCE = 1e-5;

public:
    // Checks whether x-coordinates have uniformly spaced intervals.
    static bool isUniformSpacing(const std::vector<double>& x, double& detectedH);

    // Checks whether the point count is valid for Simpson's 1/3 rule.
    static bool isValidSimpson13(std::size_t numPoints);

    // Checks whether the point count is valid for Simpson's 3/8 rule.
    static bool isValidSimpson38(std::size_t numPoints);

    // Integrates a series using the trapezoidal rule.
    static double solveTrapezoidal(const std::vector<double>& x, const std::vector<double>& y);

    // Integrates a uniformly spaced series using Simpson's 1/3 rule.
    static double solveSimpson13(const std::vector<double>& y, double h);

    // Integrates a uniformly spaced series using Simpson's 3/8 rule.
    static double solveSimpson38(const std::vector<double>& y, double h);

    // Integrates a series using the most suitable composite rule.
    static double solveAdaptiveComposite(const std::vector<double>& x,const std::vector<double>& y, std::string& resolvedMethodName);
};

#endif