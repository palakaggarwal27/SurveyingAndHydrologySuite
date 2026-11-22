#ifndef SURVEYING_MODULE_H
#define SURVEYING_MODULE_H

// Performs surveying calculations for boundary areas, earthwork volumes, and method comparisons.

#include "DataStructures.h"
#include "IntegrationAlgorithms.h"
#include <vector>

class SurveyingCalculator {
public:
    // Calculates an irregular boundary area from baseline and offset points.
    CalculationResult calculateBoundaryArea(const std::vector<DataPoint>& points,IntegrationMethod method);

    // Calculates earthwork volume from station distances and cross-sectional areas.
    CalculationResult calculateEarthworkVolume(const std::vector<double>& stationDistances,
    const std::vector<double>& crossSectionAreas,IntegrationMethod method);

    // Compares all applicable integration methods for boundary area.
    CalculationResult compareSurveyingMethods(const std::vector<DataPoint>& points);
};

#endif
