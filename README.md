# Polynomial Plotter

## Introduction

This project is a small app to train my skills in programming and mathematical simulation (simple for now).

It is a simple real-time graph plotter for polynomial functions from degree 0 to 5, with a clean and easy-to-use UI.

The goal is to demonstrate my ability to optimize the backend, frontend, and documentation in a lightweight application, in order to reuse this type of code later in a more complex simulation.

## How to launch
the executable file is in the build/Desktop_QT... folder.

## Versions

### 1.2 : Optimization update
- Separation of functions into different files to improve readability
- Multiple fixes to prevent memory leaks, unnecessary loops, and improved cache usage
- Better comments and documentation


<details>
<summary> sub-version details </summary>

#### 1.2.0
- Removed two function to prevent memory leaks
- Creation of the polynomialCalculator files to improve readability (not used yet)
- Combine polynomial calculation and derivative calculcation in one function
- Add cache value for the coefficients so no need to get the input each time or calculate the derivative when there is no change

#### 1.2.1
- Removed calculation functions from MainWindow
- Added new files to handle graph-related logic
- This version is more of a clean backup than a major update

#### 1.2.2
- Fixed behavior when xmin > xmax (e.g., if xmin = 2 and xmax = 1 → xmax becomes 3)
- Refactored refreshGraph into smaller functions
- Added caching for derivative coefficients
- Added documentation for each function
- Display of selected values for derivatives


</details>

### 1.1 : Derivative and tangent graph
- Select a value of x and display its corresponding value
- Increased coefficient precision (from 2 to 5 decimal places)
- Added first and second derivative plots (currently on the same graph, with possible separation later)
- Added tangent line display

### 1.0 : Full first version
- Polynomial plotting from degree 0 to 5
- Real-time graph updates
- Cursor displaying x and y values
- Inputs for Xmin and Xmax
- Display of the polynomial equation as a title

## Objectives

### for 1.3 : more functionality
- Add integral visualization
- Add polynomial root calculation
- Display local and global extrema

### for 1.4 : more documentation 
- Access the README directly within the app
- Explain what a polynomial function is
- Add mathematical formulas
- Explain all important parts of the code
- Also explain less critical (anecdotal) parts
