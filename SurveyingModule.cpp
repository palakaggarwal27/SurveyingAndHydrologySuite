#include "SurveyingModule.h"
#include "CalculationSupport.h"

CalculationResult SurveyingCalculator::calculateBoundaryArea(const std::vector<DataPoint>& points,IntegrationMethod method) {
    std::vector<double> x = CalculationSupport::xOf(points);
    std::vector<double> y = CalculationSupport::yOf(points);
    CalculationSupport::validateSeries(x, y, "Boundary offsets");

    CalculationResult r = CalculationSupport::buildResult("Irregular Plot Boundary Area", "m^2", x, y, method);

    for (std::size_t i = 0; i < y.size(); ++i) {
        if (y[i] < 0.0) {
            r.warnings.push_back("Some ordinates are negative, so the computed area is signed: "
            "parts below the baseline contribute negatively.");
            break;
        }
    }
    return r;
}

CalculationResult SurveyingCalculator::calculateEarthworkVolume(const std::vector<double>& stations,const std::vector<double>& areas,IntegrationMethod method) {
    CalculationSupport::validateSeries(stations, areas, "Earthwork cross-sections");

    CalculationResult r = CalculationSupport::buildResult("Earthwork Volume", "m^3", stations, areas, method);

    
    bool hasNegative = false;
    for (std::size_t i = 0; i < areas.size(); ++i)
        if (areas[i] < 0.0) hasNegative = true;

    if (hasNegative) {
        r.calculationTitle = "Earthwork Volume (net, cut - fill)";

        
        std::vector<double> cut, fill;
        for (std::size_t i = 0; i < areas.size(); ++i) {
            if (areas[i] > 0.0) { cut.push_back(areas[i]); fill.push_back(0.0); }
            else                { cut.push_back(0.0);      fill.push_back(-areas[i]); }
        }
        XtraResult totalCut  = {"Total cut",  CalculationSupport::integrate(stations, cut,  method).value, "m^3"};
        XtraResult totalFill = {"Total fill", CalculationSupport::integrate(stations, fill, method).value, "m^3"};
        r.extraMetrics.push_back(totalCut);
        r.extraMetrics.push_back(totalFill);
        r.warnings.push_back("Cut and fill totals are approximate in sections where the "
        "area changes sign between two stations.");
    }
    return r;
}

CalculationResult SurveyingCalculator::compareSurveyingMethods(const std::vector<DataPoint>& points) {
    std::vector<double> x = CalculationSupport::xOf(points);
    std::vector<double> y = CalculationSupport::yOf(points);
    return CalculationSupport::compareAllMethods("Irregular Plot Boundary Area - method comparison", "m^2", x, y);
}
