# Surveying and Hydrology Suite

Group Project (Group 4 - Project P4) for the **Computational Methods** subject at **MSIT** (Maharaja Surajmal Institute of Technology), as per the **GGSIPU (IPU)** syllabus.

A C++ command-line application that uses numerical integration methods to solve practical problems in land surveying and hydrology.

---

## Features

### 1. Land Surveying
- **Boundary Area**: Calculate irregular plot area from baseline offsets.
- **Earthwork Volume**: Calculate excavation/embankment volume from cross-sectional areas and distances.

### 2. Hydrology
- **River Cross-Section Area**: Calculate river cross-sectional area from depth measurements.
- **River Discharge**: Calculate total discharge from depth and flow velocity measurements.
- **Reservoir Storage**: Calculate water storage volume from stage and contour surface areas.

### 3. Numerical Integration Methods
- **Trapezoidal Rule** (for equal or unequal spacing)
- **Simpson's 1/3 Rule** (for equal spacing and even number of intervals)
- **Simpson's 3/8 Rule** (for equal spacing and intervals divisible by 3)

### 4. Method Comparison & Error Analysis
- Runs all applicable integration methods side by side.
- Calculates absolute error, relative error, and percentage error when an exact solution is known.
- Option to save results as a text report or CSV file.

---

## How to Compile and Run

### Requirements
- C++ compiler with C++11 support (e.g. MinGW `g++` on Windows).

### Compilation
Open a terminal in the project folder and run:

```bash
g++ -std=c++11 -o suite.exe main.cpp UIEngine.cpp FileIO.cpp IntegrationAlgorithms.cpp CalculationSupport.cpp SurveyingModule.cpp HydrologyModule.cpp
```

### Execution
```bash
.\suite.exe
```

---

## Sample Data
Sample input files are provided in the `data/` folder:
- `survey_boundary_area.csv`
- `earthwork_volume.csv`
- `river_cross_section_area.csv`
- `river_discharge.csv`
- `reservoir_storage.csv`

Lines starting with `#` in CSV files are ignored as comments.
