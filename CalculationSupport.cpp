#include "CalculationSupport.h"
#include "IntegrationAlgorithms.h"

#include <chrono>
#include <cmath>
#include <stdexcept>


static const std::string NAME_TRAP = "Trapezoidal Rule";
static const std::string NAME_S13  = "Simpson's 1/3 Rule";
static const std::string NAME_S38  = "Simpson's 3/8 Rule";
static const std::string NAME_COMP = "Composite Simpson (auto)";




static std::string describeN(std::size_t n) {
    return "n = " + std::to_string(n);
}

static double elapsedMicroseconds(const std::chrono::high_resolution_clock::time_point& start) {
    return std::chrono::duration<double, std::micro>(
        std::chrono::high_resolution_clock::now() - start).count();
}

static void setMethodErrors(MethodResult& method, double exactResult) {
    if (!method.applicable || exactResult != exactResult) return;
    method.absoluteError = std::fabs(method.value - exactResult);
    if (std::fabs(exactResult) > 1e-12) {
        method.relativeError = method.absoluteError / std::fabs(exactResult);
        method.percentError = method.relativeError * 100.0;
    } else if (method.absoluteError < 1e-12) {
        method.relativeError = 0.0;
        method.percentError = 0.0;
    }
}


std::vector<double> CalculationSupport::xOf(const std::vector<DataPoint>& pts) {
    std::vector<double> v;
    for (std::size_t i = 0; i < pts.size(); ++i) v.push_back(pts[i].x);
    return v;
}

std::vector<double> CalculationSupport::yOf(const std::vector<DataPoint>& pts) {
    std::vector<double> v;
    for (std::size_t i = 0; i < pts.size(); ++i) v.push_back(pts[i].y);
    return v;
}

void CalculationSupport::validateSeries(const std::vector<double>& x,const std::vector<double>& y,const std::string& context,bool requireNonNegativeY) {
    if (x.size() != y.size())
        throw std::invalid_argument(context + ": x and y lists have different lengths.");
    if (x.size() < 2)
        throw std::invalid_argument(context + ": at least 2 data points are required.");

    for (std::size_t i = 0; i < x.size(); ++i) {
        std::string where = context + ", point " + std::to_string(i + 1) + ": ";

        if (!std::isfinite(x[i]) || !std::isfinite(y[i]))
            throw std::invalid_argument(where + "value is not a finite number.");
        if (i > 0 && !(x[i] > x[i - 1]))
            throw std::invalid_argument(where + "x values must be strictly increasing.");
        if (requireNonNegativeY && y[i] < 0.0)
            throw std::invalid_argument(where + "negative value is not physically valid.");
    }
}




IntegralOutcome CalculationSupport::integrate(const std::vector<double>& x,const std::vector<double>& y,IntegrationMethod method) {
    std::size_t n = x.size() - 1;          
    double h = 0.0;
    bool uniform = IntegrationEngine::isUniformSpacing(x, h);

    bool fits13 = uniform && n >= 2 && n % 2 == 0;   
    bool fits38 = uniform && n >= 3 && n % 3 == 0;   

    IntegralOutcome out;

    
    int rule = 0;

    if (method == IntegrationMethod::TRAPEZOIDAL) {
        rule = 0;
    } else if (method == IntegrationMethod::AUTO_SELECT) {
        if (fits13) rule = 1;
        else if (fits38) rule = 2;
        else if (uniform && n >= 2) rule = 3;
        else {
            rule = 0;
            out.note = "Auto-select: Trapezoidal Rule used (spacing not uniform or only one interval).";
        }
    } else {
        bool want13 = (method == IntegrationMethod::SIMPSON_1_3);
        std::string wanted = want13 ? NAME_S13 : NAME_S38;
        bool fits = want13 ? fits13 : fits38;

        if (fits) {
            rule = want13 ? 1 : 2;
        } else {
            if (uniform && n >= 2) {
                rule = 3;
                out.note = wanted + " does not fit " + describeN(n) + "; composite Simpson used instead.";
            } else {
                rule = 0;
                out.note = wanted + " needs uniform spacing and at least 2 intervals; "
                "Trapezoidal Rule used instead.";
            }
        }
    }

    if (rule == 1) {
        out.value = IntegrationEngine::solveSimpson13(y, h);
        out.methodName = NAME_S13;
    } else if (rule == 2) {
        out.value = IntegrationEngine::solveSimpson38(y, h);
        out.methodName = NAME_S38;
    } else if (rule == 3) {
        std::string resolved;
        out.value = IntegrationEngine::solveAdaptiveComposite(x, y, resolved);
        out.methodName = resolved.empty() ? NAME_COMP : resolved;
    } else {
        out.value = IntegrationEngine::solveTrapezoidal(x, y);
        out.methodName = NAME_TRAP;
    }
    return out;
}




CalculationResult CalculationSupport::buildResult(const std::string& title,const std::string& unit,const std::vector<double>& x,const std::vector<double>& y,IntegrationMethod method, double exactResult) {
    validateSeries(x, y, title);

    CalculationResult r;
    r.calculationTitle = title;
    r.unit = unit;
    r.intervalCount = x.size() - 1;
    r.exactResult = exactResult;

    double h = 0.0;
    r.isUniformSpacing = IntegrationEngine::isUniformSpacing(x, h);
    r.stepSize = r.isUniformSpacing ? h : 0.0;

    const std::chrono::high_resolution_clock::time_point start =
        std::chrono::high_resolution_clock::now();
    IntegralOutcome out = integrate(x, y, method);
    r.methodUsed = out.methodName;
    r.primaryResult = out.value;
    if (!out.note.empty()) r.warnings.push_back(out.note);

    MethodResult row;
    row.name = out.methodName;
    row.applicable = true;
    row.isBaseline = true;
    row.value = out.value;
    row.executionTimeMicroseconds = elapsedMicroseconds(start);
    setMethodErrors(row, exactResult);
    r.methodComparison.push_back(row);

    return r;
}

void CalculationSupport::applyErrorAnalysis(CalculationResult& result, double exactResult) {
    result.exactResult = exactResult;
    for (std::size_t i = 0; i < result.methodComparison.size(); ++i)
        setMethodErrors(result.methodComparison[i], exactResult);
}




CalculationResult CalculationSupport::compareAllMethods(const std::string& title,const std::string& unit,const std::vector<double>& x,const std::vector<double>& y, double exactResult) {
    validateSeries(x, y, title);
    std::size_t n = x.size() - 1;
    double h = 0.0;
    bool uniform = IntegrationEngine::isUniformSpacing(x, h);
    bool fits13 = uniform && n >= 2 && n % 2 == 0;
    bool fits38 = uniform && n >= 3 && n % 3 == 0;
    bool useComposite = uniform && n >= 2 && !fits13 && !fits38;   
    
    MethodResult trap;
    trap.name = NAME_TRAP;
    trap.applicable = true;
    {
        const std::chrono::high_resolution_clock::time_point start =
            std::chrono::high_resolution_clock::now();
        trap.value = IntegrationEngine::solveTrapezoidal(x, y);
        trap.executionTimeMicroseconds = elapsedMicroseconds(start);
    }

    
    MethodResult s13;
    s13.name = NAME_S13;
    if (fits13) {
        s13.applicable = true;
        const std::chrono::high_resolution_clock::time_point start =
            std::chrono::high_resolution_clock::now();
        s13.value = IntegrationEngine::solveSimpson13(y, h);
        s13.executionTimeMicroseconds = elapsedMicroseconds(start);
    } else if (!uniform) {
        s13.note = "needs uniform spacing";
    } else {
        s13.note = "needs an even number of intervals (" + describeN(n) + ")";
    }

    
    MethodResult s38;
    s38.name = NAME_S38;
    if (fits38) {
        s38.applicable = true;
        const std::chrono::high_resolution_clock::time_point start =
            std::chrono::high_resolution_clock::now();
        s38.value = IntegrationEngine::solveSimpson38(y, h);
        s38.executionTimeMicroseconds = elapsedMicroseconds(start);
    } else if (!uniform) {
        s38.note = "needs uniform spacing";
    } else {
        s38.note = "needs a multiple of 3 intervals (" + describeN(n) + ")";
    }

    
    MethodResult comp;
    comp.name = NAME_COMP;
    if (useComposite) {
        std::string resolved;
        comp.applicable = true;
        const std::chrono::high_resolution_clock::time_point start =
            std::chrono::high_resolution_clock::now();
        comp.value = IntegrationEngine::solveAdaptiveComposite(x, y, resolved);
        comp.executionTimeMicroseconds = elapsedMicroseconds(start);
        if (!resolved.empty()) comp.name = resolved;
    }

    
    std::vector<MethodResult> rows;
    rows.push_back(trap);
    rows.push_back(s13);
    rows.push_back(s38);
    if (useComposite) rows.push_back(comp);

    
    std::size_t base = 0;
    if (fits13) base = 1;
    else if (fits38) base = 2;
    else if (useComposite) base = 3;
    rows[base].isBaseline = true;
    double baseValue = rows[base].value;

    
    for (std::size_t i = 0; i < rows.size(); ++i) {
        if (!rows[i].applicable) continue;
        double diff = std::fabs(rows[i].value - baseValue);
        if (std::fabs(baseValue) > 1e-12) rows[i].percentDiffVsBaseline = diff / std::fabs(baseValue) * 100.0;
        else if (diff < 1e-12) rows[i].percentDiffVsBaseline = 0.0;
        setMethodErrors(rows[i], exactResult);
    }

    
    CalculationResult r;
    r.calculationTitle = title;
    r.unit = unit;
    r.intervalCount = n;
    r.exactResult = exactResult;
    r.isUniformSpacing = uniform;
    r.stepSize = uniform ? h : 0.0;
    r.methodUsed = "Comparison (baseline: " + rows[base].name + ")";
    r.primaryResult = baseValue;
    r.methodComparison = rows;

    if (!uniform)
        r.warnings.push_back("Spacing is not uniform: only the Trapezoidal Rule is applicable.");
    else if (useComposite)
        r.warnings.push_back("Neither pure Simpson rule fits " + describeN(n) +
        "; the composite rule is used as the baseline.");
    return r;
}
