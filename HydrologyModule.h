#ifndef HYDROLOGY_MODULE_H
#define HYDROLOGY_MODULE_H

// Performs hydrology calculations for river cross-sections, discharge, and reservoir storage.

#include "DataStructures.h"
#include "IntegrationAlgorithms.h"
#include <vector>

class HydrologyCalculator {
public:
    // Calculates river cross-sectional area from distance and depth data.
    CalculationResult calculateRiverArea(const std::vector<DataPoint>& profilePoints,IntegrationMethod method);

    // Calculates river discharge and related hydraulic metrics.
    CalculationResult calculateRiverDischarge(const std::vector<HydrologyPoint>& hydroPoints,IntegrationMethod method);

    // Calculates reservoir storage volume from stage and surface-area data.
    CalculationResult calculateReservoirVolume(const std::vector<DataPoint>& stageAreaPoints,IntegrationMethod method);

    // Compares all applicable integration methods for river discharge.
    CalculationResult compareHydrologyMethods(const std::vector<HydrologyPoint>& hydroPoints);

    // Compares all applicable integration methods for reservoir volume.
    CalculationResult compareReservoirMethods(const std::vector<DataPoint>& stageAreaPoints);
};

#endif
