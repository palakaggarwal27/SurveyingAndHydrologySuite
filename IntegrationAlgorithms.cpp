#include "IntegrationAlgorithms.h"
#include <cmath>
#include <stdexcept>


static double simpson13Part(const std::vector<double>& y,std::size_t first, std::size_t last, double h) {
    double sum = y[first] + y[last];
    for (std::size_t i = first + 1; i < last; ++i) {
        if ((i - first) % 2 == 1)
            sum = sum + 4.0 * y[i];      
        else
            sum = sum + 2.0 * y[i];      
    }
    return h / 3.0 * sum;
}





static double simpson38Part(const std::vector<double>& y,std::size_t first, std::size_t last, double h) {
    double sum = y[first] + y[last];
    for (std::size_t i = first + 1; i < last; ++i) {
        if ((i - first) % 3 == 0)
            sum = sum + 2.0 * y[i];      
        else
            sum = sum + 3.0 * y[i];      
    }
    return 3.0 * h / 8.0 * sum;
}







bool IntegrationEngine::isUniformSpacing(const std::vector<double>& x, double& detectedH) {
    if (x.size() < 2) {
        detectedH = 0.0;
        return false;
    }

    detectedH = x[1] - x[0];
    if (detectedH == 0.0) return false;          

    for (std::size_t i = 2; i < x.size(); ++i) {
        double gap = x[i] - x[i - 1];
        if (std::fabs(gap - detectedH) > SPACING_TOLERANCE * std::fabs(detectedH))
            return false;
    }
    return true;
}




bool IntegrationEngine::isValidSimpson13(std::size_t numPoints) {
    if (numPoints < 3) return false;
    return (numPoints - 1) % 2 == 0;
}



bool IntegrationEngine::isValidSimpson38(std::size_t numPoints) {
    if (numPoints < 4) return false;
    return (numPoints - 1) % 3 == 0;
}




double IntegrationEngine::solveTrapezoidal(const std::vector<double>& x,const std::vector<double>& y) {
    if (x.size() != y.size() || x.size() < 2)
        throw std::invalid_argument("Trapezoidal rule: x and y vectors need the same size and at least 2 points.");

    double total = 0.0;
    for (std::size_t i = 1; i < x.size(); ++i) {
        double width = x[i] - x[i - 1];
        total = total + width * (y[i] + y[i - 1]) / 2.0;
    }
    return total;
}




double IntegrationEngine::solveSimpson13(const std::vector<double>& y, double h) {
    if (!isValidSimpson13(y.size()))
        throw std::invalid_argument("Simpson's 1/3 rule needs an odd number of data points (even number of intervals).");

    return simpson13Part(y, 0, y.size() - 1, h);
}




double IntegrationEngine::solveSimpson38(const std::vector<double>& y, double h) {
    if (!isValidSimpson38(y.size()))
        throw std::invalid_argument("Simpson's 3/8 rule needs a number of intervals that is a multiple of 3.");

    return simpson38Part(y, 0, y.size() - 1, h);
}




double IntegrationEngine::solveAdaptiveComposite(const std::vector<double>& x,const std::vector<double>& y,
std::string& resolvedMethodName) {
    if (x.size() != y.size() || x.size() < 2)
        throw std::invalid_argument("Composite rule: x and y need the same size and at least 2 points.");

    std::size_t n = x.size() - 1;      
    double h = 0.0;
    bool uniform = isUniformSpacing(x, h);

    if (!uniform || n == 1) {
        resolvedMethodName = "Trapezoidal Rule";
        return solveTrapezoidal(x, y);
    }

    if (n % 2 == 0) {
        resolvedMethodName = "Simpson's 1/3 Rule";
        return simpson13Part(y, 0, n, h);
    }

    if (n == 3) {
        resolvedMethodName = "Simpson's 3/8 Rule";
        return simpson38Part(y, 0, n, h);
    }

    
    resolvedMethodName = "Simpson's 1/3 + 3/8 (composite)";
    double firstPart = simpson13Part(y, 0, n - 3, h);      
    double lastPart  = simpson38Part(y, n - 3, n, h);      
    return firstPart + lastPart;
}
