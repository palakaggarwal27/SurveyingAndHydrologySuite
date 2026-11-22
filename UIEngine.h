#ifndef UI_ENGINE_H
#define UI_ENGINE_H

// Provides the console interface and coordinates surveying, hydrology, comparison, and report workflows.

#include "DataStructures.h"
#include "CalculationSupport.h"
#include "SurveyingModule.h"
#include "HydrologyModule.h"
#include "FileIO.h"
#include <string>
#include <vector>

class UserInterface {
private:
    SurveyingCalculator surveyingCalc;
    HydrologyCalculator hydrologyCalc;

    // Displays the application header.
    void displayHeader() const;

    // Reads and validates a menu choice within the allowed range.
    int getMenuChoice(int minOption, int maxOption) const;

    // Prompts the user to select an integration method.
    IntegrationMethod promptMethodSelection() const;

    // Reads and validates the number of data points.
    bool readPointCount(int& n) const;

    // Checks and clears a failed numeric input.
    bool inputFailed() const;

    // Offers to save a result containing x-y point data.
    void offerToSave(const CalculationResult& result, const std::vector<DataPoint>& points) const;

    // Offers to save a result containing river measurement data.
    void offerToSave(const CalculationResult& result, const std::vector<HydrologyPoint>& points) const;

    // Runs the land surveying workflow.
    void runSurveyingWorkflow();

    // Runs the hydrology workflow.
    void runHydrologyWorkflow();

    // Runs the multi-method comparison workflow.
    void runComparisonWorkflow();

    // Runs the known-answer test workflow.
    void runKnownAnswerTests();

    // Prompts for an exact answer and updates error analysis.
    bool promptExactAnswer(CalculationResult& result) const;

public:
    // Starts the main console application loop.
    void startMainLoop();

    // Displays a formatted calculation result table.
    static void renderResultTable(const CalculationResult& result);
};

#endif
