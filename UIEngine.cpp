#include "UIEngine.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <stdexcept>
#include <cmath>
#include <cstdlib>


static const int MAX_POINTS = 10000;

static bool readFilePath(std::string& path) {
    std::cout << "Enter file path (.csv/.txt; paths with spaces are supported): ";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::getline(std::cin, path);
    if (path.size() >= 2 &&
        ((path.front() == '"' && path.back() == '"') ||
         (path.front() == '\'' && path.back() == '\''))) {
        path = path.substr(1, path.size() - 2);
    }
    if (path.empty()) {
        std::cout << "[ERROR] A file path is required.\n";
        return false;
    }
    return true;
}

static bool chooseInputSource() {
    std::cout << "\nHow do you want to provide the data?\n";
    std::cout << "  (1) Enter data manually\n";
    std::cout << "  (2) Load data from a CSV/TXT file\n";
    std::cout << "  (3) Back to main menu\n";
    return true;
}

void UserInterface::displayHeader() const {
    std::cout << "\n======================================================================\n";
    std::cout << "     NUMERICAL INTEGRATION SUITE (SURVEYING & HYDROLOGY)\n";
    std::cout << "======================================================================\n";
}

int UserInterface::getMenuChoice(int minOption, int maxOption) const {
    int choice;
    while (true) {
        std::cout << "Select an option [" << minOption << "-" << maxOption << "]: ";
        if (std::cin >> choice && choice >= minOption && choice <= maxOption) {
            return choice;
        }
        
        
        if (std::cin.eof()) {
            std::cout << "\n[INFO] Input ended. Exiting application.\n";
            std::exit(0);
        }
        std::cout << "Invalid selection. Please enter a valid number.\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

IntegrationMethod UserInterface::promptMethodSelection() const {
    std::cout << "\nSelect Integration Method:\n";
    std::cout << "  (1) Trapezoidal Rule\n";
    std::cout << "  (2) Simpson's 1/3 Rule\n";
    std::cout << "  (3) Simpson's 3/8 Rule\n";
    std::cout << "  (4) Auto-Select / Adaptive Composite\n";
    int choice = getMenuChoice(1, 4);
    return static_cast<IntegrationMethod>(choice - 1);
}







bool UserInterface::readPointCount(int& n) const {
    std::cout << "Enter number of data points: ";
    if (!(std::cin >> n)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "[ERROR] That is not a number. Returning to the menu.\n";
        return false;
    }
    if (n < 2 || n > MAX_POINTS) {
        std::cout << "[ERROR] The number of points must be between 2 and " << MAX_POINTS
                  << ". Returning to the menu.\n";
        return false;
    }
    return true;
}



bool UserInterface::inputFailed() const {
    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "[ERROR] A value was not a number. Returning to the menu.\n";
        return true;
    }
    return false;
}




void UserInterface::offerToSave(const CalculationResult& result, const std::vector<DataPoint>& points) const {
    std::cout << "\nSave this report to a file? [Y/N]: ";
    char answer = 'N';
    std::cin >> answer;
    if (answer != 'Y' && answer != 'y') return;

    std::cout << "Enter file name (example: report.txt): ";
    std::string fileName;
    std::cin >> fileName;

    if (FileManager::exportReport(fileName, result, points))
        std::cout << "[SUCCESS] Report saved to '" << fileName << "'.\n";
    else
        std::cout << "[ERROR] Could not write the file '" << fileName << "'.\n";
}

void UserInterface::offerToSave(const CalculationResult& result, const std::vector<HydrologyPoint>& points) const {
    std::cout << "\nSave this report to a file? [Y/N]: ";
    char answer = 'N';
    std::cin >> answer;
    if (answer != 'Y' && answer != 'y') return;

    std::cout << "Enter file name (example: report.txt): ";
    std::string fileName;
    std::cin >> fileName;

    if (FileManager::exportReport(fileName, result, points))
        std::cout << "[SUCCESS] Report saved to '" << fileName << "'.\n";
    else
        std::cout << "[ERROR] Could not write the file '" << fileName << "'.\n";
}






static double cleanZero(double value) {
    if (std::fabs(value) < 1e-9) return 0.0;
    return value;
}


static void printNumber(double value, int width, int digits) {
    if (value != value)
        std::cout << std::setw(width) << "-";
    else
        std::cout << std::setw(width) << std::fixed << std::setprecision(digits) << value;
}

bool UserInterface::promptExactAnswer(CalculationResult& result) const {
    std::cout << "\nDo you know the exact answer for this input? [Y/N]: ";
    char answer = 'N';
    std::cin >> answer;
    if (answer != 'Y' && answer != 'y') return false;

    double exactAnswer = 0.0;
    std::cout << "Enter the exact answer (" << result.unit << "): ";
    if (!(std::cin >> exactAnswer) || !std::isfinite(exactAnswer)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "[ERROR] The exact answer must be a finite number. Errors were not calculated.\n";
        return false;
    }
    CalculationSupport::applyErrorAnalysis(result, exactAnswer);
    return true;
}

void UserInterface::renderResultTable(const CalculationResult& result) {
    std::cout << "\n======================================================================\n";
    std::cout << "                      CALCULATION REPORT                              \n";
    std::cout << "======================================================================\n";
    std::cout << " Target:               " << result.calculationTitle << "\n";
    std::cout << " Integration Method:   " << result.methodUsed << "\n";
    std::cout << " Total Intervals (n):  " << result.intervalCount << "\n";
    std::cout << "----------------------------------------------------------------------\n";
    std::cout << " COMPUTED VALUE:       " << std::fixed << std::setprecision(4)
              << cleanZero(result.primaryResult) << " " << result.unit << "\n";
    std::cout << " EXACT VALUE:          ";
    printNumber(result.exactResult, 12, 4);
    std::cout << " " << result.unit << "\n";

    
    for (std::size_t i = 0; i < result.extraMetrics.size(); ++i) {
        std::cout << " " << result.extraMetrics[i].label << ": " << std::fixed << std::setprecision(4)
                  << cleanZero(result.extraMetrics[i].value) << " " << result.extraMetrics[i].unit << "\n";
    }

    
    if (!result.methodComparison.empty()) {
        std::cout << "----------------------------------------------------------------------\n";
        std::cout << " " << std::left << std::setw(35) << "Method" << std::right
                  << std::setw(12) << "Result" << std::setw(12) << "Abs error"
                  << std::setw(12) << "Rel error" << std::setw(12) << "% error"
                  << std::setw(14) << "Time (us)"
                  << "\n";
        bool anyBaseline = false;
        for (std::size_t i = 0; i < result.methodComparison.size(); ++i) {
            const MethodResult& m = result.methodComparison[i];
            std::string name = m.name;
            if (m.isBaseline) { name = name + " *"; anyBaseline = true; }
            std::cout << " " << std::left << std::setw(35) << name << std::right;
            if (!m.applicable) {
                std::cout << "N/A (" << m.note << ")\n";
                continue;
            }
            printNumber(m.value, 12, 4);
            printNumber(m.absoluteError, 12, 4);
            printNumber(m.relativeError, 12, 6);
            printNumber(m.percentError, 12, 4);
            printNumber(m.executionTimeMicroseconds, 14, 3);
            std::cout << "\n";
        }
        if (anyBaseline) {
            std::cout << " * = baseline (best rule that fits the data)";
            if (result.exactResult != result.exactResult)
                std::cout << "; errors require a known exact value";
            std::cout << "\n";
        }
    }

    
    if (!result.warnings.empty()) {
        std::cout << "----------------------------------------------------------------------\n";
        for (std::size_t i = 0; i < result.warnings.size(); ++i)
            std::cout << " NOTE: " << result.warnings[i] << "\n";
    }
    std::cout << "======================================================================\n";
}

void UserInterface::runSurveyingWorkflow() {
    std::cout << "\n--- LAND SURVEYING MODULE ---\n";
    std::cout << "  (1) Boundary Area Calculation\n";
    std::cout << "  (2) Earthwork Cut & Fill Volume\n";
    int choice = getMenuChoice(1, 2);

    chooseInputSource();
    int source = getMenuChoice(1, 3);
    if (source == 3) return;

    std::vector<DataPoint> points;
    if (source == 1) {
        int n;
        if (!readPointCount(n)) return;
        points.resize(n);
        std::cout << "Enter coordinates (x y) for each point:\n";
        for (int i = 0; i < n; ++i) {
            std::cout << "Point " << i + 1 << ": ";
            std::cin >> points[i].x >> points[i].y;
            if (inputFailed()) return;
        }
    } else {
        std::string path;
        std::string fileError;
        if (!readFilePath(path) || !FileManager::loadDataPoints(path, points, &fileError)) {
            std::cout << "[ERROR] " << (fileError.empty() ? "Could not read the file." : fileError) << "\n";
            return;
        }
        std::cout << "[SUCCESS] Loaded " << points.size() << " data points.\n";
    }

    IntegrationMethod method = promptMethodSelection();
    CalculationResult res;

    
    
    try {
        if (choice == 1) {
            res = surveyingCalc.calculateBoundaryArea(points, method);
        } else {
            std::vector<double> stations(points.size()), areas(points.size());
            for (std::size_t i = 0; i < points.size(); ++i) {
                stations[i] = points[i].x;
                areas[i] = points[i].y;
            }
            res = surveyingCalc.calculateEarthworkVolume(stations, areas, method);
        }
    } catch (const std::exception& e) {
        std::cout << "\n[ERROR] " << e.what() << "\n";
        return;
    }

    if (source == 2) promptExactAnswer(res);
    renderResultTable(res);
    offerToSave(res, points);
}

void UserInterface::runHydrologyWorkflow() {
    std::cout << "\n--- HYDROLOGY MODULE ---\n";
    std::cout << "  (1) River Cross-Sectional Area\n";
    std::cout << "  (2) River Discharge (Q = Area x Velocity)\n";
    std::cout << "  (3) Reservoir Storage Capacity\n";
    int choice = getMenuChoice(1, 3);

    chooseInputSource();
    int source = getMenuChoice(1, 3);
    if (source == 3) return;

    IntegrationMethod method;
    CalculationResult res;

    if (choice == 2) {
        std::vector<HydrologyPoint> hPoints;
        if (source == 1) {
            int n;
            if (!readPointCount(n)) return;
            hPoints.resize(n);
            std::cout << "Enter (Distance Depth Velocity) for each point:\n";
            for (int i = 0; i < n; ++i) {
                std::cout << "Point " << i + 1 << ": ";
                std::cin >> hPoints[i].distance >> hPoints[i].depth >> hPoints[i].velocity;
                if (inputFailed()) return;
            }
        } else {
            std::string path;
            std::string fileError;
            if (!readFilePath(path) || !FileManager::loadHydrologyPoints(path, hPoints, &fileError)) {
                std::cout << "[ERROR] " << (fileError.empty() ? "Could not read the file." : fileError) << "\n";
                return;
            }
            std::cout << "[SUCCESS] Loaded " << hPoints.size() << " river data points.\n";
        }
        method = promptMethodSelection();
        try {
            res = hydrologyCalc.calculateRiverDischarge(hPoints, method);
        } catch (const std::exception& e) {
            std::cout << "\n[ERROR] " << e.what() << "\n";
            return;
        }
        if (source == 2) promptExactAnswer(res);
        renderResultTable(res);
        offerToSave(res, hPoints);
    } else {
        std::vector<DataPoint> points;
        if (source == 1) {
            int n;
            if (!readPointCount(n)) return;
            points.resize(n);
            std::cout << "Enter (x y) points:\n";
            for (int i = 0; i < n; ++i) {
                std::cout << "Point " << i + 1 << ": ";
                std::cin >> points[i].x >> points[i].y;
                if (inputFailed()) return;
            }
        } else {
            std::string path;
            std::string fileError;
            if (!readFilePath(path) || !FileManager::loadDataPoints(path, points, &fileError)) {
                std::cout << "[ERROR] " << (fileError.empty() ? "Could not read the file." : fileError) << "\n";
                return;
            }
            std::cout << "[SUCCESS] Loaded " << points.size() << " data points.\n";
        }
        method = promptMethodSelection();
        try {
            if (choice == 1) res = hydrologyCalc.calculateRiverArea(points, method);
            else res = hydrologyCalc.calculateReservoirVolume(points, method);
        } catch (const std::exception& e) {
            std::cout << "\n[ERROR] " << e.what() << "\n";
            return;
        }
        if (source == 2) promptExactAnswer(res);
        renderResultTable(res);
        offerToSave(res, points);
    }
}

void UserInterface::runComparisonWorkflow() {
    std::cout << "\n--- MULTI-METHOD COMPARISON TOOL ---\n";
    std::cout << "  (1) Survey boundary area (x, y)\n";
    std::cout << "  (2) River discharge (distance, depth, velocity)\n";
    std::cout << "  (3) Reservoir storage (stage, surface area)\n";
    int calculation = getMenuChoice(1, 3);
    chooseInputSource();
    int source = getMenuChoice(1, 3);
    if (source == 3) return;

    std::vector<DataPoint> points;
    std::vector<HydrologyPoint> riverPoints;
    if (calculation == 2 && source == 2) {
        std::string path;
        std::string fileError;
        if (!readFilePath(path) || !FileManager::loadHydrologyPoints(path, riverPoints, &fileError)) {
            std::cout << "[ERROR] " << (fileError.empty() ? "Could not read the file." : fileError) << "\n";
            return;
        }
        std::cout << "[SUCCESS] Loaded " << riverPoints.size() << " river data points.\n";
    } else if (source == 2) {
        std::string path;
        std::string fileError;
        if (!readFilePath(path) || !FileManager::loadDataPoints(path, points, &fileError)) {
            std::cout << "[ERROR] " << (fileError.empty() ? "Could not read the file." : fileError) << "\n";
            return;
        }
        std::cout << "[SUCCESS] Loaded " << points.size() << " data points.\n";
    }
    else if (calculation == 2) {
        int n;
        if (!readPointCount(n)) return;
        riverPoints.resize(n);
        std::cout << "Point format: distance depth velocity\n";
        for (int i = 0; i < n; ++i) {
            std::cout << "Point " << i + 1 << ": ";
            std::cin >> riverPoints[i].distance >> riverPoints[i].depth >> riverPoints[i].velocity;
            if (inputFailed()) return;
        }
    } else {
        int n;
        if (!readPointCount(n)) return;
        points.resize(n);
        for (int i = 0; i < n; ++i) {
            std::cout << "Point " << i + 1 << " (x y): ";
            std::cin >> points[i].x >> points[i].y;
            if (inputFailed()) return;
        }
    }

    CalculationResult res;
    try {
        if (calculation == 1)
            res = surveyingCalc.compareSurveyingMethods(points);
        else if (calculation == 2)
            res = hydrologyCalc.compareHydrologyMethods(riverPoints);
        else
            res = hydrologyCalc.compareReservoirMethods(points);
    } catch (const std::exception& e) {
        std::cout << "\n[ERROR] " << e.what() << "\n";
        return;
    }
    if (source == 2) promptExactAnswer(res);
    renderResultTable(res);
    if (calculation == 2) offerToSave(res, riverPoints);
    else offerToSave(res, points);
}

void UserInterface::runKnownAnswerTests() {
    struct KnownAnswerCase {
        const char* name;
        const char* unit;
        std::vector<DataPoint> points;
        double exactAnswer;
    };

    const KnownAnswerCase cases[] = {
        { "Test case for reservoir volume", "m^3",
          { {0.0, 0.0}, {1.0, 1.0}, {2.0, 16.0}, {3.0, 81.0},
            {4.0, 256.0}, {5.0, 625.0}, {6.0, 1296.0} }, 1555.2 },
        { "Test case for earthwork volume", "m^3",
          { {0.0, 2.0}, {1.0, 5.0}, {2.0, 10.0}, {3.0, 17.0},
          {4.0, 26.0} }, 136.0 / 3.0 },
        { "Test case for river discharge", "m^3/s",
          { {0.0, 0.0}, {1.0, 2.0}, {2.0, 6.0}, {3.0, 12.0},
            {4.0, 20.0} }, 88.0 / 3.0 }
    };

    const int caseCount = static_cast<int>(sizeof(cases) / sizeof(cases[0]));
    std::cout << "\n--- KNOWN-ANSWER TEST CASES ---\n";
    for (int i = 0; i < caseCount; ++i)
        std::cout << "  (" << i + 1 << ") " << cases[i].name << "\n";
    std::cout << "  (" << caseCount + 1 << ") Back to main menu\n";

    const int selected = getMenuChoice(1, caseCount + 1);
    if (selected == caseCount + 1) return;

    const KnownAnswerCase& testCase = cases[selected - 1];
    std::vector<double> x = CalculationSupport::xOf(testCase.points);
    std::vector<double> y = CalculationSupport::yOf(testCase.points);
    CalculationResult result = CalculationSupport::compareAllMethods(
        testCase.name, testCase.unit, x, y, testCase.exactAnswer);

    std::cout << "\n" << testCase.name << "\n";
    std::cout << "Input values (x, y):\n";
    for (std::size_t i = 0; i < testCase.points.size(); ++i)
        std::cout << "  (" << testCase.points[i].x << ", " << testCase.points[i].y << ")\n";
    std::cout << "Exact answer: " << std::fixed << std::setprecision(6)
              << testCase.exactAnswer << " " << testCase.unit << "\n";
    renderResultTable(result);
}

void UserInterface::startMainLoop() {
    while (true) {
        displayHeader();
        std::cout << "  [1] Land Surveying Module\n";
        std::cout << "  [2] Hydrology & Water Resources Module\n";
        std::cout << "  [3] Multi-Method Comparison Tool\n";
        std::cout << "  [4] Known-Answer Test Cases\n";
        std::cout << "  [5] Exit Application\n";

        int choice = getMenuChoice(1, 5);
        switch (choice) {
            case 1: runSurveyingWorkflow(); break;
            case 2: runHydrologyWorkflow(); break;
            case 3: runComparisonWorkflow(); break;
            case 4: runKnownAnswerTests(); break;
            case 5: std::cout << "\nExiting application. Goodbye!\n"; return;
        }
    }
}
