# Implementation Plan

## Simulation and Comparative Analysis of Page Replacement Algorithms

**Project language:** C (primary)\
**Team size:** 4 members\
**Implementation model:** 3 members develop independent modules; 1
member performs final integration\
**GUI approach:** GTK with C\
**Core algorithms:** FIFO, LRU, Optimal

------------------------------------------------------------------------

## 1. Project Overview

The application will simulate how an operating system manages a limited
number of physical-memory page frames. It will accept a page-reference
string, a frame count, and an algorithm selection. It will show the
frame contents after every page request, identify each request as a hit
or fault, calculate performance statistics, and compare FIFO, LRU, and
Optimal.

The implementation must satisfy the supplied project specification:

-   Accept a page-reference string and a positive frame count.
-   Allow FIFO, LRU, Optimal, or comparison of all three algorithms.
-   Display the memory-frame state after every reference.
-   Identify page hits and page faults.
-   Calculate total hits, total faults, hit ratio, and fault ratio.
-   Display a comparative summary.
-   Validate input and handle edge cases.
-   Keep each algorithm in a separate C function/module.
-   Provide an understandable graphical user interface.

### Language and technology decisions

  Component                         Technology
  --------------------------------- -----------------------------------
  Main application and simulation   C
  GUI                               GTK (C API)
  Data structures                   C arrays and structs
  Build system                      Makefile
  Version control                   Git and GitHub
  Testing                           C unit tests and manual GUI tests
  Development environment           VS Code or Code::Blocks

**Important:** The page replacement algorithms, simulation engine,
statistics, validation, and application logic must be written in C. GTK
is also accessed through its C API. Do not implement the main logic in
Python, JavaScript, or another language.

------------------------------------------------------------------------

## 2. Team Responsibilities at a Glance

  -----------------------------------------------------------------------
  Member                  Main responsibility     Main deliverable
  ----------------------- ----------------------- -----------------------
  Person 1                FIFO and LRU algorithms Tested FIFO and LRU C
                                                  modules

  Person 2                Optimal algorithm and   Tested Optimal and
                          common simulation       simulation-engine C
                          engine                  modules

  Person 3                GTK GUI, input          Working GUI module
                          validation, output      
                          tables and statistics   
                          presentation            

  Person 4                Final integration and   Integrated, tested,
                          system testing          runnable application
  -----------------------------------------------------------------------

Persons 1, 2, and 3 are responsible for implementing their assigned
modules. Person 4 should not independently rewrite those modules; the
fourth member's primary responsibility is to connect, test, debug, and
package the work.

All members must follow the shared interfaces in this plan so that the
modules can be integrated without changing function names or data
formats.

------------------------------------------------------------------------

# 3. Detailed Work Allocation

## Person 1 --- FIFO and LRU Algorithm Developer

### Objective

Implement and test the FIFO and LRU page replacement algorithms in C.
Both algorithms must produce the frame state and hit/fault result for
every page reference.

### Tasks

#### A. Understand the required behavior

1.  Study how FIFO selects the page that entered memory earliest.
2.  Study how LRU selects the page that has not been used for the
    longest time.
3.  Understand the difference between a page hit and a page fault.
4.  Confirm how empty frames are filled before replacing an existing
    page.

#### B. Implement FIFO

Create `fifo.c` and `fifo.h`.

Implementation requirements:

1.  Receive the reference string and frame count.
2.  Initialize all frames as empty.
3.  Process page references in their original order.
4.  Check whether the requested page is already in a frame.
5.  If present, record a hit and do not replace a page.
6.  If absent, record a fault.
7.  If an empty frame exists, load the page into it.
8.  Otherwise, replace the page that has been in memory the longest.
9.  Advance the FIFO replacement position correctly.
10. Record the frame contents and hit/fault status after every request.

**Important FIFO detail:** A hit must not change the FIFO replacement
order.

#### C. Implement LRU

Create `lru.c` and `lru.h`.

Implementation requirements:

1.  Receive the reference string and frame count.
2.  Initialize all frames as empty.
3.  Process each page reference sequentially.
4.  Check whether the requested page is already present.
5.  On a hit, update that page's most-recently-used information.
6.  On a fault, load into an empty frame if available.
7.  If all frames are occupied, identify and replace the least recently
    used page.
8.  Update usage information after loading a page.
9.  Record the frame contents and hit/fault status after every request.

Use a C array to store the last-used position or timestamp for each
frame. Ensure the selected victim is the least recently used page, not
simply the first frame.

#### D. Test both algorithms

Use the sample reference string:

`7 0 1 2 0 3 0 4 2 3`

Use 3 frames.

Test at least:

-   A reference string with no repeated pages.
-   A reference string containing repeated pages and several hits.
-   A frame count of 1.
-   A frame count larger than the number of distinct pages.
-   A reference string that causes frequent replacements.
-   Invalid input passed through the common validation layer.

Check every row of the resulting frame table, not just the final fault
count.

### Files owned by Person 1

``` text
src/algorithms/fifo.c
src/algorithms/fifo.h
src/algorithms/lru.c
src/algorithms/lru.h
tests/test_fifo.c
tests/test_lru.c
```

### Deliverables

-   FIFO algorithm implemented in C.
-   LRU algorithm implemented in C.
-   Header files containing the agreed function declarations.
-   Test cases and test results.
-   No GUI implementation is required from this member.

### Completion criteria

-   FIFO replacement order is correct.
-   LRU always replaces the least recently used page.
-   Hits do not cause replacement.
-   Each reference produces exactly one recorded step.
-   Both modules compile independently and use the shared data
    structures.

------------------------------------------------------------------------

## Person 2 --- Optimal Algorithm and Common Simulation Engine Developer

### Objective

Implement the Optimal page replacement algorithm and the common
simulation/result structures needed by the application.

### Tasks

#### A. Implement Optimal

Create `optimal.c` and `optimal.h`.

Implementation requirements:

1.  Receive the complete reference string and frame count.
2.  Initialize all frames as empty.
3.  Process the references sequentially.
4.  Check whether the requested page is already present.
5.  Record a hit when the page is present.
6.  On a fault, use an empty frame if one is available.
7.  If all frames are occupied, examine the remaining future references.
8.  For each page currently in memory, find its next use.
9.  Replace the page whose next use is farthest in the future.
10. If a page is never used again, it is an eligible replacement and
    should be preferred.
11. Record the frame contents and hit/fault status after every request.

Optimal must examine only references occurring after the current
request. It must not use future references that have already been
processed.

#### B. Define common data structures

Create `simulation_types.h`.

The following is the agreed conceptual interface. The final
implementation may add fields if all team members agree before coding.

``` c
#ifndef SIMULATION_TYPES_H
#define SIMULATION_TYPES_H

typedef enum {
    PAGE_HIT,
    PAGE_FAULT
} PageStatus;

typedef struct {
    int page;
    PageStatus status;
    int *frames;
} SimulationStep;

typedef struct {
    char algorithm_name[16];

    int total_references;
    int total_hits;
    int total_faults;

    double hit_ratio;
    double fault_ratio;

    int frame_count;
    SimulationStep *steps;
} SimulationResult;

#endif
```

Use a consistent empty-frame representation. For example, use `-1` to
represent an empty frame and reject negative page numbers during input
validation.

#### C. Define the common simulation interface

Create `simulation.h` and `simulation.c`.

The common simulation layer should:

1.  Receive validated input.
2.  Call the selected algorithm.
3.  Store each algorithm's result in the agreed `SimulationResult`
    format.
4.  Calculate total hits and faults.
5.  Calculate hit and fault ratios.
6.  Return the result to the GUI.
7.  Release allocated memory through a clearly documented cleanup
    function.

The three algorithms may share result structures and helper functions,
but each replacement policy must remain a separate implementation.

**Division of work:** Person 2 owns the common types and
simulation-engine interface. Person 1 implements FIFO/LRU against that
interface. Person 3 consumes the same interface in the GUI.

#### D. Calculate statistics

Use:

``` text
Hit Ratio   = (Total Hits / Total References) × 100
Fault Ratio = (Total Faults / Total References) × 100
```

Requirements:

-   Avoid division by zero.
-   Ensure `total_hits + total_faults == total_references`.
-   Ensure the hit ratio and fault ratio add up to approximately 100%
    when references exist.
-   Store ratios consistently as percentages, not fractions.

#### E. Test Optimal and the common engine

Test:

-   A page that is never referenced again.
-   Multiple pages whose next use is equally far away.
-   A page that is not used again.
-   All references fitting into available frames.
-   One available frame.
-   Repeated references.
-   Empty and invalid inputs rejected by validation.
-   Correct statistics for all algorithms.

### Files owned by Person 2

``` text
src/algorithms/optimal.c
src/algorithms/optimal.h
src/core/simulation.c
src/core/simulation.h
src/core/simulation_types.h
src/core/statistics.c
src/core/statistics.h
tests/test_optimal.c
tests/test_simulation.c
```

### Deliverables

-   Optimal algorithm implemented in C.
-   Shared result structures and function declarations.
-   Common simulation and statistics modules.
-   Memory cleanup functions.
-   Test cases and test results.

### Completion criteria

-   Optimal chooses the correct victim based on future references.
-   Results use the shared structures.
-   Statistics are accurate.
-   Memory allocated for simulation results can be safely released.
-   The common interface is documented and ready for GUI integration.

------------------------------------------------------------------------

## Person 3 --- GUI and Input/Output Developer

### Objective

Develop a user-friendly graphical interface using GTK and C. The GUI
must collect input, invoke the C simulation layer, and present the
results clearly.

### Tasks

#### A. Design the main window

Create a main window with:

1.  Project title: **Page Replacement Algorithm Simulator**.
2.  Page-reference string input.
3.  Number-of-frames input.
4.  Algorithm selection.
5.  Run Simulation button.
6.  Clear/Reset button.
7.  Results area.
8.  Error-message area.

The algorithm selection must provide:

-   FIFO
-   LRU
-   Optimal
-   Compare All

#### B. Implement input handling

Create `input_validation.c` and `input_validation.h`.

Validation requirements:

1.  Reject an empty page-reference string.
2.  Reject a frame count that is missing, zero, or negative.
3.  Reject non-numeric page values.
4.  Reject negative page numbers if `-1` is reserved for empty frames.
5.  Reject malformed separators or incomplete values.
6.  Detect values that exceed the supported integer range.
7.  Ensure the parsed reference count does not exceed the application's
    documented limit.
8.  Display a useful error message instead of crashing.

The parser should accept a clearly documented format, such as:

`7 0 1 2 0 3 0 4 2 3`

It may also accept comma-separated values if implemented consistently.

#### C. Connect the GUI to the simulation engine

1.  Read the text entered by the user.
2.  Parse the reference string into a C integer array.
3.  Read the frame count.
4.  Read the selected algorithm.
5.  Call the common C simulation interface.
6.  Receive the result structures.
7.  Display the results.
8.  Free parsed input and simulation results when no longer needed.

The GUI must not contain a second implementation of FIFO, LRU, or
Optimal. It must call the algorithm/simulation modules developed by
Persons 1 and 2.

#### D. Display step-by-step results

For each selected algorithm, show a table with:

  Page   Frame 1   Frame 2   Frame 3   Status
  ------ --------- --------- --------- --------
  7      7         ---       ---       Fault
  0      7         0         ---       Fault
  1      7         0         1         Fault
  2      2         0         1         Fault
  0      2         0         1         Hit

The number of frame columns must adapt to the frame count. Use `—` or
another consistent display for empty frames.

For Compare All, provide a separate result table for FIFO, LRU, and
Optimal, or clearly separated sections in the same results view.

#### E. Display performance statistics

For each selected algorithm, show:

-   Total page references.
-   Total page hits.
-   Total page faults.
-   Page hit ratio.
-   Page fault ratio.

For Compare All, provide a comparison table:

  Algorithm      Page Hits   Page Faults    Hit Ratio   Fault Ratio
  ----------- ------------ ------------- ------------ -------------
  FIFO          Calculated    Calculated   Calculated    Calculated
  LRU           Calculated    Calculated   Calculated    Calculated
  Optimal       Calculated    Calculated   Calculated    Calculated

All values must come from the C simulation results, not manually entered
values.

#### F. Improve usability

-   Use clear labels and sensible spacing.
-   Make hit and fault statuses easy to distinguish.
-   Display an error dialog or inline message for invalid input.
-   Prevent repeated clicks from leaving stale results on screen.
-   Clear previous results when a new simulation starts.
-   Keep the interface usable with different frame counts and
    reference-string lengths.
-   Make sure long result tables can be scrolled.

#### G. Test the GUI

Test:

-   Every algorithm selection.
-   Compare All.
-   Valid and invalid reference strings.
-   Missing frame count.
-   One frame and several frames.
-   Long output tables.
-   Repeated simulation runs.
-   Clear/Reset behavior.
-   GUI behavior when simulation results are empty or unavailable.

### Files owned by Person 3

``` text
src/gui/main.c
src/gui/main_window.c
src/gui/main_window.h
src/gui/results_view.c
src/gui/results_view.h
src/gui/input_validation.c
src/gui/input_validation.h
```

### Deliverables

-   GTK GUI written in C.
-   Input parsing and validation.
-   Step-by-step result tables.
-   Statistics and comparison view.
-   GUI test results.

### Completion criteria

-   The user can enter a reference string and frame count.
-   The user can select any one algorithm or Compare All.
-   The GUI calls the shared C simulation interface.
-   Results and statistics are displayed correctly.
-   Invalid input is handled without application crashes.

------------------------------------------------------------------------

## Person 4 --- Final Integration and Testing Developer

### Objective

Integrate the independently developed C modules, resolve interface/build
issues, perform end-to-end testing, and prepare the final runnable
application.

Person 4 is the integration owner. The other three members remain
responsible for fixing defects in their own modules.

### Tasks

#### A. Establish the shared repository

1.  Create or maintain the central GitHub repository.
2.  Ensure all members use the same agreed project structure.
3.  Establish a main branch and separate feature branches.
4.  Agree on function names, parameter types, ownership of allocated
    memory, and return-value conventions before module development
    diverges.
5.  Ensure each member commits only their assigned work.
6.  Review pull requests for interface compatibility.

Suggested branches:

``` text
main
person1-fifo-lru
person2-optimal-engine
person3-gui
integration
```

#### B. Verify module readiness

Before integration, confirm that:

-   Person 1 has delivered FIFO and LRU.
-   Person 2 has delivered Optimal, shared structures, simulation
    interface, and statistics.
-   Person 3 has delivered the GTK interface and input validation.
-   All header files are included.
-   Function declarations match their definitions.
-   No module contains unresolved placeholders.
-   Each module has been compiled and tested independently.

#### C. Integrate the C modules

Integration order:

1.  Integrate `simulation_types.h`.
2.  Integrate the simulation-engine headers.
3.  Integrate FIFO and LRU.
4.  Integrate Optimal.
5.  Integrate statistics and cleanup functions.
6.  Integrate input validation.
7.  Connect the GUI to the common simulation interface.
8.  Connect result structures to the GUI tables.
9.  Implement the Compare All flow.
10. Build the complete application.

#### D. Configure the build system

Create and maintain the root `Makefile`.

The build must:

-   Compile all `.c` files.
-   Include the required header directories.
-   Link GTK libraries.
-   Produce one runnable application.
-   Provide a clean command to remove generated object files and
    binaries.
-   Report useful compilation errors.

Suggested commands:

``` bash
make
make clean
make run
```

The exact GTK compiler and linker flags should be obtained through
`pkg-config`, for example:

``` bash
pkg-config --cflags --libs gtk+-3.0
```

The team must use one GTK version consistently across development and
integration.

#### E. Perform end-to-end testing

Use the supplied sample:

``` text
Reference string: 7 0 1 2 0 3 0 4 2 3
Frames: 3
Selection: Compare All
```

Verify:

-   The GUI accepts the input.
-   All three algorithms execute.
-   Every reference produces one result row per algorithm.
-   Frame contents are correct after every request.
-   Hit/fault labels are correct.
-   Statistics are calculated correctly.
-   Comparison results are displayed.
-   Memory is freed after each run.
-   Running a second simulation does not reuse stale results.

Also test:

-   Empty reference string.
-   Zero and negative frame counts.
-   Invalid characters.
-   A single page reference.
-   One frame.
-   More frames than distinct pages.
-   Repeated references.
-   A longer reference string.
-   Each algorithm separately.
-   Compare All.

#### F. Debug and stabilize

1.  Resolve compilation and linker errors.
2.  Resolve mismatched function signatures.
3.  Check array bounds and allocation sizes.
4.  Check for memory leaks and invalid memory access.
5.  Check result-table row/column alignment.
6.  Check that all algorithms receive identical input in Compare All.
7.  Verify that hit and fault totals match the number of references.
8.  Verify that the ratios are calculated consistently.
9.  Confirm that no algorithm or GUI module has been duplicated
    unnecessarily.

Use compiler warnings and, where available, tools such as
AddressSanitizer or Valgrind.

#### G. Prepare final delivery

-   Final source code.
-   Makefile and build instructions.
-   README with dependencies and execution instructions.
-   Test cases and results.
-   Screenshots of the running GUI.
-   Short demonstration flow for the project presentation.

### Files primarily owned by Person 4

``` text
Makefile
README.md
docs/integration_notes.md
tests/test_integration.c
```

Person 4 may edit other files when integration requires it, but should
coordinate changes with the module owner.

### Deliverables

-   Fully integrated C application.
-   Working GTK GUI connected to all algorithms.
-   Successful build instructions.
-   End-to-end test results.
-   Final source repository ready for submission.

### Completion criteria

-   The complete project builds from a clean checkout.
-   All three algorithms work through the GUI.
-   Compare All works correctly.
-   Invalid input is handled.
-   No known critical integration defects remain.
-   The README explains dependencies, compilation, and execution.

------------------------------------------------------------------------

# 4. Agreed Project Structure

Use the following structure from the beginning. Do not allow each member
to invent a different folder structure.

``` text
page-replacement-simulator/
│
├── include/
│   └── common.h
│
├── src/
│   ├── algorithms/
│   │   ├── fifo.c
│   │   ├── fifo.h
│   │   ├── lru.c
│   │   ├── lru.h
│   │   ├── optimal.c
│   │   └── optimal.h
│   │
│   ├── core/
│   │   ├── simulation.c
│   │   ├── simulation.h
│   │   ├── simulation_types.h
│   │   ├── statistics.c
│   │   └── statistics.h
│   │
│   ├── gui/
│   │   ├── main.c
│   │   ├── main_window.c
│   │   ├── main_window.h
│   │   ├── results_view.c
│   │   ├── results_view.h
│   │   ├── input_validation.c
│   │   └── input_validation.h
│   │
│   └── utils/
│       ├── memory.c
│       └── memory.h
│
├── tests/
│   ├── test_fifo.c
│   ├── test_lru.c
│   ├── test_optimal.c
│   ├── test_simulation.c
│   └── test_integration.c
│
├── docs/
│   └── integration_notes.md
│
├── Makefile
├── README.md
└── .gitignore
```

### File ownership

  -----------------------------------------------------------------------
  File/folder                         Owner
  ----------------------------------- -----------------------------------
  `src/algorithms/fifo.*`             Person 1

  `src/algorithms/lru.*`              Person 1

  `src/algorithms/optimal.*`          Person 2

  `src/core/*`                        Person 2

  `src/gui/*`                         Person 3

  `tests/test_fifo.c`,                Person 1
  `tests/test_lru.c`                  

  `tests/test_optimal.c`,             Person 2
  `tests/test_simulation.c`           

  `tests/test_integration.c`          Person 4

  `Makefile`, `README.md`,            Person 4
  `docs/integration_notes.md`         

  `include/common.h`, `src/utils/*`   Agree before implementation;
                                      integration owner coordinates
  -----------------------------------------------------------------------

------------------------------------------------------------------------

# 5. Integration Contract --- How the Modules Connect

This is the most important coordination requirement. The team should
agree on the interface before implementation begins.

## A. Data flow

``` text
                 USER
                   |
                   v
           GTK GUI (Person 3)
                   |
                   v
        Input Parsing and Validation
                   |
                   v
          Common Simulation API
              (Person 2)
                   |
          +--------+--------+
          |        |        |
          v        v        v
         FIFO     LRU    Optimal
       Person 1 Person 1 Person 2
          |        |        |
          +--------+--------+
                   |
                   v
          Simulation Results
                   |
                   v
          Statistics Calculation
                   |
                   v
            GTK Results View
                   |
                   v
       Step Tables and Comparison
```

## B. Shared algorithm function interface

Use a consistent function signature for all three algorithms. One
suitable interface is:

``` c
SimulationResult *run_fifo(
    const int references[],
    int reference_count,
    int frame_count
);

SimulationResult *run_lru(
    const int references[],
    int reference_count,
    int frame_count
);

SimulationResult *run_optimal(
    const int references[],
    int reference_count,
    int frame_count
);
```

The corresponding declarations must be placed in their respective header
files.

All functions must:

-   Receive the same input format.
-   Return the same result structure.
-   Record one step for every page reference.
-   Use the same empty-frame representation.
-   Document who owns and frees the returned memory.
-   Return an error or `NULL` consistently when simulation cannot be
    completed.

## C. Common simulation API

Person 2 should provide a single interface for the GUI to use. For
example:

``` c
typedef enum {
    ALGORITHM_FIFO,
    ALGORITHM_LRU,
    ALGORITHM_OPTIMAL,
    ALGORITHM_ALL
} AlgorithmChoice;

SimulationResult **run_simulation(
    const int references[],
    int reference_count,
    int frame_count,
    AlgorithmChoice choice,
    int *result_count
);

void free_simulation_result(SimulationResult *result);
```

The exact function names may be adjusted once, before implementation,
but all team members must use the final agreed declarations.

For `ALGORITHM_ALL`, the function should return results for all three
algorithms using the same reference string and frame count.

The team must document whether the result array itself is dynamically
allocated and provide a corresponding cleanup function for it.

## D. Memory ownership rules

To avoid integration crashes:

1.  The algorithm function allocates its result and step storage.
2.  The returned result remains valid until the caller frees it.
3.  The GUI must not free individual frame arrays separately unless the
    cleanup contract explicitly requires it.
4.  Cleanup functions must free all nested allocations.
5.  The caller must not access a result after it has been freed.
6.  Every successful simulation must have one clearly defined cleanup
    path.

------------------------------------------------------------------------

# 6. Work Schedule and Milestones

The schedule below is organized by development phase. The team can map
the phases to its available calendar.

  ---------------------------------------------------------------------------------
  Phase          Person 1       Person 2             Person 3       Person 4
  -------------- -------------- -------------------- -------------- ---------------
  1\. Agreement  Review         Define shared        Choose GTK     Set up
                 FIFO/LRU       structures and API   layout and GUI repository and
                 requirements                        interface      project
                                                                    structure

  2\.            Implement      Implement Optimal    Build window   Review
  Independent    FIFO/LRU       and simulation       and input      interfaces and
  development                   engine               validation     prepare build
                                                                    system

  3\. Module     Test FIFO/LRU  Test                 Test GUI and   Check module
  testing                       Optimal/statistics   parsing        readiness

  4\.            Fix algorithm  Fix engine/API       Connect GUI    Integrate all
  Integration    defects        defects              and fix        modules
                                                     display        
                                                     defects        

  5\. System     Verify         Verify statistics    Verify GUI     Run end-to-end
  testing        algorithm                           behavior       and memory
                 output                                             tests

  6\.            Submit final   Submit final core    Submit final   Prepare final
  Finalization   algorithm code code                 GUI code       build, README
                                                                    and
                                                                    demonstration
  ---------------------------------------------------------------------------------

### Milestone gates

**Milestone 1 --- Interface agreement**

No member should start independent implementation until the shared
structures, function signatures, empty-frame representation, and memory
ownership rules are agreed.

**Milestone 2 --- Independent modules compile**

Each member must demonstrate that their assigned C files compile
independently against the shared headers.

**Milestone 3 --- Module tests pass**

Each algorithm and the GUI input layer must pass their own tests.

**Milestone 4 --- First integrated build**

The GUI must be able to call the simulation engine and display results
for at least one valid input.

**Milestone 5 --- Complete comparison**

FIFO, LRU, Optimal, and Compare All must work through the same GUI.

**Milestone 6 --- Final verification**

The application must build from a clean checkout and pass the agreed
end-to-end tests.

------------------------------------------------------------------------

# 7. Testing and Expected Results

Use the sample from the project specification:

``` text
Page-reference string:
7 0 1 2 0 3 0 4 2 3

Number of frames:
3

Algorithm:
Compare All
```

For FIFO, the supplied specification shows this frame progression:

    Page   Frame 1   Frame 2   Frame 3 Status
  ------ --------- --------- --------- --------
       7         7       ---       --- Fault
       0         7         0       --- Fault
       1         7         0         1 Fault
       2         2         0         1 Fault
       0         2         0         1 Hit
       3         2         3         1 Fault
       0         2         3         0 Fault
       4         4         3         0 Fault
       2         4         2         0 Fault
       3         4         2         3 Fault

The expected FIFO summary in the specification is:

  Metric               Expected value
  ------------------ ----------------
  Total references                 10
  FIFO hits                         1
  FIFO faults                       9
  FIFO hit ratio                  10%
  FIFO fault ratio                90%

The LRU and Optimal values must be calculated by the program. Do not
hardcode expected comparison values into the application.

### General correctness checks

For every algorithm:

``` text
Total Hits + Total Faults = Total References
```

``` text
Hit Ratio + Fault Ratio = 100%
```

The second check applies when the reference count is greater than zero,
allowing for normal floating-point rounding.

Every algorithm must produce exactly one step per reference. Each step
must contain the page requested, the frame state after processing that
request, and the hit/fault status.

------------------------------------------------------------------------

# 8. Git and Collaboration Rules

1.  Keep `main` buildable.
2.  Each implementation member works on their own branch.
3.  Commit related changes with clear messages.
4.  Do not independently rename shared structures or functions.
5.  Discuss changes to shared headers before merging.
6.  Submit work through pull requests.
7.  Person 4 reviews integration compatibility, but module owners review
    algorithm or GUI logic changes.
8.  Merge working modules into the integration branch.
9.  Run a clean build after each major merge.
10. Merge the tested integration branch into `main` only when the
    complete application works.

Suggested commit messages:

``` text
feat: implement FIFO page replacement
feat: implement LRU page replacement
feat: implement Optimal page replacement
feat: add common simulation engine
feat: add GTK main window
feat: add input validation
feat: add simulation result tables
test: add algorithm test cases
build: configure Makefile
docs: add project setup instructions
```

------------------------------------------------------------------------

# 9. Final Submission Checklist

## Core implementation

-   [ ] FIFO implemented in C.
-   [ ] LRU implemented in C.
-   [ ] Optimal implemented in C.
-   [ ] Common simulation engine implemented in C.
-   [ ] Hit/fault statistics implemented in C.
-   [ ] Dynamic memory allocation and cleanup handled correctly.

## GUI

-   [ ] GTK interface implemented in C.
-   [ ] Reference-string input available.
-   [ ] Frame-count input available.
-   [ ] Algorithm selection available.
-   [ ] Compare All available.
-   [ ] Step-by-step frame tables displayed.
-   [ ] Hits and faults identified.
-   [ ] Statistics displayed.
-   [ ] Comparison table displayed.
-   [ ] Invalid input messages displayed.

## Integration and testing

-   [ ] All modules use the shared interfaces.
-   [ ] Full application builds using Makefile.
-   [ ] All three algorithms tested.
-   [ ] Sample FIFO output matches the specification.
-   [ ] Comparison values are calculated dynamically.
-   [ ] Edge cases tested.
-   [ ] No known critical memory or integration defects remain.
-   [ ] Clean build verified.

## Documentation and demonstration

-   [ ] README contains dependencies and setup steps.
-   [ ] README contains build and run commands.
-   [ ] Project folder structure is maintained.
-   [ ] Screenshots of the GUI are prepared.
-   [ ] Demonstration uses the supplied sample input.
-   [ ] Each member can explain their assigned module.

------------------------------------------------------------------------

# 10. Final Team Agreement

The project is divided so that the three implementation members can work
in parallel without implementing the same functionality:

-   **Person 1 owns FIFO and LRU.**
-   **Person 2 owns Optimal, common simulation structures, and
    statistics.**
-   **Person 3 owns the GTK interface, input validation, and result
    presentation.**
-   **Person 4 owns final integration, build configuration, system
    testing, and delivery.**

The common interface and memory ownership rules must be agreed upon
before parallel development. The GUI must call the C simulation engine
rather than duplicate algorithm logic. The final application should be a
C-based GTK program, with all page replacement and performance
calculations implemented in C.
