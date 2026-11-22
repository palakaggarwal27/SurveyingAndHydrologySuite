#include "HydrologyModule.h"
#include "CalculationSupport.h"

#include <cmath>
#include <stdexcept>



static void splitRiver(const std::vector<HydrologyPoint>& pts,std::vector<double>& x, std::vector<double>& depth,std::vector<double>& vel, std::vector<double>& q) {
    for (std::size_t i = 0; i < pts.size(); ++i) {
        x.push_back(pts[i].distance);
        depth.push_back(pts[i].depth);
        vel.push_back(pts[i].velocity);
        q.push_back(pts[i].getUnitDischarge());
    }
    CalculationSupport::validateSeries(x, depth, "River cross-section", true);   
    for (std::size_t i = 0; i < vel.size(); ++i)
        if (!std::isfinite(vel[i]))
            throw std::invalid_argument("River cross-section: velocity is not a finite number.");
}

CalculationResult HydrologyCalculator::calculateRiverArea(const std::vector<DataPoint>& profilePoints,IntegrationMethod method) {
    std::vector<double> x = CalculationSupport::xOf(profilePoints);
    std::vector<double> d = CalculationSupport::yOf(profilePoints);
    CalculationSupport::validateSeries(x, d, "River depth profile", true);
    return CalculationSupport::buildResult("River Cross-Sectional Area", "m^2", x, d, method);
}

CalculationResult HydrologyCalculator::calculateRiverDischarge(const std::vector<HydrologyPoint>& hydroPoints,IntegrationMethod method) {
    std::vector<double> x, depth, vel, q;
    splitRiver(hydroPoints, x, depth, vel, q);

    
    CalculationResult r = CalculationSupport::buildResult("River Discharge (Q)", "m^3/s", x, q, method);

    
    double area = CalculationSupport::integrate(x, depth, method).value;
    XtraResult areaInfo = {"Cross-sectional area (A)", area, "m^2"};
    r.extraMetrics.push_back(areaInfo);
    if (area > 1e-12) {
        XtraResult meanV = {"Mean velocity (Q/A)", r.primaryResult / area, "m/s"};
        r.extraMetrics.push_back(meanV);
    }
    XtraResult width = {"Top width", x.back() - x.front(), "m"};
    r.extraMetrics.push_back(width);

    for (std::size_t i = 0; i < vel.size(); ++i) {
        if (vel[i] < 0.0) {
            r.warnings.push_back("Negative velocity found (reverse flow): it reduces the net discharge.");
            break;
        }
    }
    return r;
}

CalculationResult HydrologyCalculator::calculateReservoirVolume(const std::vector<DataPoint>& stageAreaPoints,IntegrationMethod method) {
    std::vector<double> x = CalculationSupport::xOf(stageAreaPoints);
    std::vector<double> a = CalculationSupport::yOf(stageAreaPoints);
    CalculationSupport::validateSeries(x, a, "Reservoir stage-area data", true);
    return CalculationSupport::buildResult("Reservoir Storage Volume", "m^3", x, a, method);
}

CalculationResult HydrologyCalculator::compareHydrologyMethods(const std::vector<HydrologyPoint>& hydroPoints) {
    std::vector<double> x, depth, vel, q;
    splitRiver(hydroPoints, x, depth, vel, q);
    return CalculationSupport::compareAllMethods("River Discharge (Q) - method comparison", "m^3/s", x, q);
}

CalculationResult HydrologyCalculator::compareReservoirMethods(const std::vector<DataPoint>& stageAreaPoints) {
    std::vector<double> x = CalculationSupport::xOf(stageAreaPoints);
    std::vector<double> a = CalculationSupport::yOf(stageAreaPoints);
    CalculationSupport::validateSeries(x, a, "Reservoir stage-area data", true);
    return CalculationSupport::compareAllMethods("Reservoir Storage Volume - method comparison", "m^3", x, a);
}
