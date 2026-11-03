# Deadlock Simulation System

## Four-Person Implementation and Integration Plan

**Project:** Design and Implementation of an Interactive Deadlock
Simulation System\
**Basis:** Week 4 project brief supplied by the team\
**Team structure:** Three implementation members and one integration
member
**Programming language:** The entire application, including the GUI-related application logic, shared data structures, algorithms, and integration, must be implemented in the C programming language.

------------------------------------------------------------------------

## 1. Project goal

Build an interactive graphical application that demonstrates how an
operating system manages multiple processes competing for multiple
resource types. A user must be able to create processes and resources,
enter allocation and maximum-requirement data, make resource requests,
inspect system state, run Banker's Algorithm, detect deadlocks, and
demonstrate recovery.

The application must display, at minimum:

-   Available Resource Vector
-   Allocation Matrix
-   Maximum Requirement Matrix
-   Need Matrix
-   Resource Requests
-   Resource Allocation Graph (RAG)
-   Safe Sequence
-   Deadlocked Processes
-   Overall system state: **SAFE**, **UNSAFE**, or **DEADLOCKED**

The brief explicitly asks for a proper GUI. The application should
therefore be an interactive simulator, not merely a command-line program
or a collection of algorithm outputs.

## 1.1 Programming language requirement

-   The implementation must be done in the **C programming language**.
-   All four team members must write their assigned modules in C.
-   Shared data structures, function interfaces, algorithms, and module
    integration must use C-compatible definitions and conventions.
-   The final integrated application must remain a C implementation.
-   Do not implement the core application or any assigned module in C++,
    Python, Java, JavaScript, or another programming language.


## 2. Team responsibility overview

  -----------------------------------------------------------------------
  Member                  Primary responsibility  Main deliverable
  ----------------------- ----------------------- -----------------------
  Person 1                Process and Resource    Working setup screens,
                          Management              state data, matrices,
                                                  validation, and
                                                  resource
                                                  allocation/request
                                                  operations

  Person 2                Banker's Algorithm and  Working safety
                          Safe-State Analysis     algorithm, request
                                                  algorithm, safe
                                                  sequence, and
                                                  safe/unsafe result

  Person 3                Deadlock Detection,     Working deadlock
                          Recovery, and RAG       detection, recovery
                                                  operations, graph
                                                  data/visualization, and
                                                  deadlocked-process
                                                  display

  Person 4                Architecture,           Shared project
                          Integration, GUI        structure and
                          coordination, and final contracts, integrated
                          testing                 application, end-to-end
                                                  tests, and final build
  -----------------------------------------------------------------------

**Important:** Person 4 is the final integration owner, but must
participate from the beginning by agreeing on the shared data
structures, function interfaces, file boundaries, and integration rules.
This prevents the three implementation modules from being built with
incompatible assumptions.

------------------------------------------------------------------------

# 3. Person 1 --- Process and Resource Management

## 3.1 Objective

Implement the foundation of the simulator: process creation, resource
creation, initial system configuration, matrix entry, resource
availability calculations, and resource request/allocation operations.

Person 1 owns the data-entry and system-configuration workflow. The
other algorithm modules depend on the system state produced by this
module.

## 3.2 Features to implement

### A. Process management

-   Allow the user to create multiple processes.
-   Assign each process a unique identifier, such as P0, P1, P2.
-   Display the list of created processes.
-   Allow a process to be removed before it participates in allocations
    or requests, or define and document a safe removal rule.
-   Keep process identifiers consistent across all matrices, request
    records, and graph data.

### B. Resource management

-   Allow the user to create multiple resource types, such as A, B, and
    C.
-   Let the user enter the total number of instances for each resource
    type.
-   Display resource names and total instances.
-   Validate that resource totals are non-negative integers.
-   Keep resource ordering consistent across every vector and matrix.

### C. Initial allocation and maximum matrices

Provide GUI forms or editable tables for:

1.  **Allocation Matrix:** Number of instances of each resource
    currently allocated to each process.
2.  **Maximum Requirement Matrix:** Maximum number of instances of each
    resource that each process may need.

The application must validate:

-   Matrix row count matches the number of processes.
-   Matrix column count matches the number of resource types.
-   Every entry is a non-negative integer.
-   Allocation for a resource does not exceed its total instances.
-   A process's allocation does not exceed its declared maximum
    requirement.

### D. Need Matrix

Calculate and display:

`Need[i][j] = Maximum[i][j] - Allocation[i][j]`

The Need Matrix must be recalculated whenever the Allocation or Maximum
Matrix changes. Do not maintain a separately editable Need Matrix.

### E. Available Resource Vector

Calculate:

`Available[j] = Total[j] - Sum of Allocation[i][j] for all processes i`

Validate that the sum of allocated instances for a resource never
exceeds its total instances.

### F. Resource requests and allocation

Create a resource-request interface where the user selects a process and
enters the number of instances requested for each resource type.

Validate the request before passing it to the algorithm modules:

-   The request must contain one value for every resource type.
-   Values must be non-negative integers.
-   The request must not exceed the selected process's remaining Need.
-   The request must not exceed the currently Available resources.

Person 1 should implement the shared state update operations that apply
an approved allocation or restore resources after a release/recovery
operation. The final decision to grant a request must come from the
relevant safety/avoidance logic, not from the form alone.

## 3.3 Suggested GUI screens/components

-   Process creation panel
-   Resource creation panel
-   Allocation Matrix editor
-   Maximum Requirement Matrix editor
-   Need Matrix display
-   Available Resource Vector display
-   Resource Request form
-   Validation messages and operation feedback

## 3.4 Suggested module boundary

Person 1 should expose a documented state model and operations similar
to:

``` text
createProcess(...)
removeProcess(...)
createResource(...)
setAllocationMatrix(...)
setMaximumMatrix(...)
calculateNeedMatrix(state)
calculateAvailableVector(state)
validateResourceRequest(state, processId, request)
applyAllocation(state, processId, request)
releaseResources(state, processId, resources)
```

These are proposed responsibilities, not mandatory programming-language
syntax. Agree on exact names and parameter formats with Person 4 before
implementation.

## 3.5 Deliverables

-   Process and resource creation workflow
-   Allocation and Maximum Matrix entry and display
-   Automatically calculated Need Matrix
-   Automatically calculated Available Vector
-   Resource request form and input validation
-   Shared state update functions
-   Sample initial configurations for testing
-   Short documentation explaining the data structure and public
    functions

## 3.6 Completion criteria

Person 1's module is ready when a user can create processes and resource
types, enter valid matrices, see correct Need and Available values,
submit a validated request, and observe the state after an allocation or
release operation.

------------------------------------------------------------------------

# 4. Person 2 --- Banker's Algorithm and Safe-State Analysis

## 4.1 Objective

Implement deadlock avoidance using Banker's Algorithm. This module
determines whether the current resource-allocation state is safe and
whether a proposed resource request can be granted without moving the
system into an unsafe state.

Person 2 owns the safety algorithm and the explanation of its results.

## 4.2 Features to implement

### A. Safety algorithm

Implement the Banker's safety check using the current:

-   Available Vector
-   Allocation Matrix
-   Need Matrix
-   Process list

The algorithm should:

1.  Initialize `Work` using a copy of `Available`.
2.  Initialize `Finish` as false for each process.
3.  Find an unfinished process whose Need is less than or equal to Work
    for every resource type.
4.  If found, add that process's Allocation to Work and mark it
    finished.
5.  Repeat until no further process can be selected.
6.  If every process can finish, classify the state as SAFE and return a
    safe sequence.
7.  Otherwise, classify the state as UNSAFE and identify processes that
    could not be marked finished.

The implementation must use copies or temporary working data. Running a
safety check must not accidentally change the live system's allocations
or available resources.

### B. Safe sequence

-   Return the order in which processes can complete.
-   Display the sequence clearly, for example: `P1 → P3 → P0 → P2`.
-   Explain that a safe sequence demonstrates at least one order in
    which every process can finish.
-   If no safe sequence exists, display an appropriate unsafe-state
    message rather than inventing a sequence.

### C. Resource-request algorithm

Implement the Banker's resource-request procedure for a selected
process:

1.  Check whether Request is less than or equal to that process's Need.
2.  Check whether Request is less than or equal to Available.
3.  If either check fails, do not grant the request and explain why.
4.  Otherwise, temporarily allocate the requested resources in a trial
    state:
    -   `Available = Available - Request`
    -   `Allocation[process] = Allocation[process] + Request`
    -   `Need[process] = Need[process] - Request`
5.  Run the safety algorithm on the trial state.
6.  If the trial state is safe, approve the request and commit the
    allocation.
7.  If the trial state is unsafe, reject the request and preserve the
    original live state.

The request must never be committed before the safety check succeeds.

### D. Safe and unsafe status presentation

Return structured results so the GUI can show:

-   Current status: SAFE or UNSAFE
-   Whether the request was granted or denied
-   Safe sequence, if one exists
-   Reason for denial, if applicable
-   Relevant intermediate algorithm steps, where practical

### E. Algorithm explanation

Include a short step-by-step explanation in the GUI or results panel so
the user can understand why a state is safe or unsafe. The explanation
should reflect the actual algorithm execution, not a hard-coded example.

## 4.3 Suggested module boundary

``` text
runSafetyCheck(state)
findSafeSequence(state)
checkResourceRequest(state, processId, request)
getSafetyExplanation(result)
```

A useful result structure could contain:

``` text
{
  isSafe,
  safeSequence,
  unfinishedProcesses,
  workSteps,
  message
}
```

The exact structure must be agreed with Person 4 before implementation.
Do not let this module directly manipulate GUI elements.

## 4.4 Deliverables

-   Correct Banker's safety algorithm
-   Safe-sequence generation
-   Resource-request safety check
-   Trial-state handling and rollback on unsafe requests
-   Structured SAFE/UNSAFE results
-   Algorithm step/explanation data
-   Test cases for safe, unsafe, insufficient-availability, and
    excessive-Need requests

## 4.5 Completion criteria

Person 2's module is ready when it correctly identifies safe and unsafe
states, returns a valid safe sequence when one exists, approves only
requests that pass the safety check, and leaves the original state
unchanged when a request is denied.

------------------------------------------------------------------------

# 5. Person 3 --- Deadlock Detection, Recovery, and Resource Allocation Graph

## 5.1 Objective

Implement the deadlock-detection feature, recovery demonstrations, and
Resource Allocation Graph (RAG). This module helps users understand
which processes are waiting, which resources are held, whether a
deadlock exists, and how the system can recover.

## 5.2 Features to implement

### A. Deadlock detection algorithm

Implement a deadlock-detection algorithm using:

-   Available Resource Vector
-   Allocation Matrix
-   Outstanding Request Matrix

The outstanding request matrix represents the resources each process is
currently waiting for. Keep this distinct from the Banker's Need Matrix:
Need represents maximum remaining demand, while an outstanding request
represents a current request.

A standard detection procedure should:

1.  Initialize `Work` as a copy of Available.
2.  Mark processes with no allocated resources as finishable according
    to the selected detection algorithm's convention; initialize other
    Finish entries appropriately.
3.  Find an unfinished process whose outstanding Request is less than or
    equal to Work.
4.  If found, add its Allocation to Work and mark it finished.
5.  Repeat while another process can be selected.
6.  Processes remaining unfinished at the end are candidates for
    deadlock under the algorithm's assumptions.

Document the initialization convention and apply it consistently. Test
the algorithm against known examples.

### B. Outstanding request tracking

-   Record the current outstanding request for each process.
-   Display waiting requests in the GUI.
-   Update or clear a process's outstanding request when it is granted,
    cancelled, or otherwise resolved.
-   Ensure that the detection module receives the current request state,
    not the Maximum Matrix or Need Matrix in its place.

### C. Deadlock result

Display:

-   Whether a deadlock was detected
-   The processes identified as deadlocked
-   Resource types involved, where they can be established from the
    state
-   A clear explanation of the detection result

Do not label every unsafe state as a deadlock. An unsafe state means
safety cannot be guaranteed; a deadlocked state means processes are
actually unable to proceed under the modeled outstanding requests.

### D. Resource Allocation Graph

Create graph data and a visual representation showing:

-   Process nodes
-   Resource-type nodes
-   Allocation edges: resource to process
-   Request edges: process to resource

The graph should update when resources are allocated, requested,
released, or recovered.

Use a clear visual distinction between allocation and request edges,
with a legend. If resource types have multiple instances, make the
representation clear about whether a node represents a resource type or
individual instances. Agree on the representation with Person 4.

### E. Deadlock recovery

Provide a controlled recovery demonstration. A practical initial
recovery operation is to select one of the deadlocked processes as a
victim, terminate it in the simulation, and release its allocated
resources back to Available.

The recovery workflow should:

1.  Show the detected deadlocked processes.
2.  Let the user select a victim process.
3.  Ask for confirmation before recovery.
4.  Remove or mark the selected process as terminated according to the
    agreed state model.
5.  Release its allocated resources.
6.  Update Available, Allocation, outstanding requests, and the RAG.
7.  Run deadlock detection again and display the result.

If the team chooses a different recovery strategy, document it and
ensure the resulting state is updated consistently. The simulator should
clearly identify recovery as a simulation action, not a real
operating-system process termination.

## 5.3 Suggested GUI screens/components

-   Resource Allocation Graph panel
-   Graph legend
-   Outstanding Requests table
-   Detect Deadlock action
-   Deadlock result panel
-   Deadlocked-process list
-   Recovery/victim selection panel
-   Before-and-after recovery state display

## 5.4 Suggested module boundary

``` text
recordOutstandingRequest(state, processId, request)
runDeadlockDetection(state)
buildResourceAllocationGraph(state)
recoverFromDeadlock(state, victimProcessId)
```

A useful detection result could contain:

``` text
{
  deadlockDetected,
  deadlockedProcesses,
  workSteps,
  message
}
```

The exact interfaces and graph data format must be agreed with Person 4.

## 5.5 Deliverables

-   Outstanding Request Matrix/state
-   Deadlock detection algorithm
-   Deadlocked-process result display
-   Resource Allocation Graph data and visualization
-   Graph legend and edge updates
-   Victim-selection recovery workflow
-   Before-and-after recovery demonstration
-   Test cases for no-deadlock, deadlock, and successful recovery

## 5.6 Completion criteria

Person 3's module is ready when it can identify deadlocked processes
using outstanding requests, display an accurate and updated RAG, perform
the agreed recovery operation, release resources correctly, and rerun
detection after recovery.

------------------------------------------------------------------------

# 6. Person 4 --- Architecture, Integration, GUI Coordination, and Final Testing

## 6.1 Objective

Own the shared application structure and ensure that the three
independently implemented modules work together as one consistent
simulator. Person 4 is responsible for final integration and end-to-end
validation.

Person 4 should not wait until the final week to discover interface
mismatches. The integration process must be prepared from the beginning,
even though the final module integration happens after the three main
implementations are ready.

## 6.2 Responsibilities before implementation begins

### A. Establish the shared project structure

-   Create the central repository/project structure.
-   Define the application entry point and module locations.
-   Define shared naming conventions and coding conventions.
-   Decide how shared state will be stored and updated.
-   Define how errors and validation messages will be returned.
-   Agree on how modules will communicate.
-   Create a basic application shell so each member can develop against
    the same structure.

### B. Define the shared data contract

All three implementation members must use the same definitions for:

-   Process identifiers and process status
-   Resource identifiers and total instances
-   Available Vector
-   Allocation Matrix
-   Maximum Matrix
-   Need Matrix
-   Outstanding Request Matrix
-   Safe/unsafe/deadlocked status
-   Safe sequence
-   Deadlocked-process list
-   RAG nodes and edges

A suggested conceptual state model is:

``` text
SystemState
  processes[]
  resources[]
  total[]
  available[]
  allocation[][]
  maximum[][]
  need[][]
  outstandingRequests[][]
  status
```

Derived values such as Available and Need should have one authoritative
calculation/update path. Avoid allowing different modules to maintain
conflicting copies.

### C. Define module interfaces

-   Define shared C structs, enums, constants, and function prototypes in
    common header files (`.h`).
-   Keep function declarations in headers and their implementations in
    corresponding C source files (`.c`).
-   Agree on memory ownership, allocation, and deallocation rules for
    dynamically allocated data.
-   Ensure all modules compile and link together as a single C project.


Before coding proceeds too far, agree on:

-   Function names and input parameters
-   Return-value formats
-   Whether functions mutate state or return a new state
-   Error/result formats
-   Process and resource indexing rules
-   How the graph receives updated state
-   How terminated processes are represented

Write these agreements in a short shared interface document and
distribute it to all members.

## 6.3 Responsibilities during implementation

-   Maintain the shared project shell and integration branch.
-   Review whether each member is following the agreed contracts.
-   Prepare placeholder modules or mock data so the GUI can be developed
    before all algorithms are complete.
-   Establish a common way to report module completion and known
    limitations.
-   Integrate small, stable changes regularly where possible; do not
    leave all conflict resolution until the final day.
-   Keep the main branch in a runnable state.

## 6.4 Final integration responsibilities

When Persons 1, 2, and 3 submit their modules:

1.  Check that each module follows the agreed interfaces.
2.  Connect Person 1's system state and input operations to Person 2's
    Banker's Algorithm.
3.  Connect Person 1's resource and request state to Person 3's deadlock
    detection.
4.  Connect the graph builder to the live state so the RAG updates after
    every relevant operation.
5.  Connect recovery to the shared state-update functions.
6.  Connect all results to the GUI.
7.  Resolve naming, type, data-shape, and state-update conflicts.
8.  Verify that a denied request does not change the live state.
9.  Verify that recovery updates every affected matrix and graph.
10. Confirm that all displayed values refer to the same current system
    state.

## 6.5 GUI and user experience coordination

Person 4 owns consistency across the complete GUI. Coordinate the design
so the application has:

-   A clear dashboard or home screen
-   Process and resource setup
-   Matrix entry and display
-   Resource request workflow
-   Banker's Algorithm result area
-   Deadlock detection result area
-   Resource Allocation Graph
-   Recovery workflow
-   System status display
-   Clear validation, success, and error feedback

The GUI should make it easy to understand the system state before an
operation, the operation being performed, and the resulting state
afterward.

## 6.6 Final testing

Run the integrated test scenarios in Section 8. Fix integration defects,
verify that the application starts from a clean checkout, and prepare
the final demonstration.

## 6.7 Deliverables

-   Shared project structure and interface document
-   Common state/data contract
-   Integrated GUI and modules
-   Integration fixes
-   End-to-end test results
-   Final runnable application
-   Demonstration sequence and brief user instructions

## 6.8 Completion criteria

The integration is complete when all modules operate on one consistent
system state, all required GUI workflows work end to end, the test
scenarios pass, and the application can be run from the project
repository using the documented instructions.

------------------------------------------------------------------------

# 7. Shared interface and data rules

These rules apply to all four members.

## 7.1 Matrix and vector ordering

-   The process list defines the row order for all process matrices.
-   The resource list defines the column order for all resource vectors
    and matrices.
-   Every module must use the same ordering.
-   Prefer stable IDs over relying only on display labels or array
    positions.
-   If a process or resource is removed, update all related structures
    consistently.

## 7.2 State ownership

Person 1 owns the foundational system-state model and ordinary
state-update operations. Persons 2 and 3 should perform calculations
against a supplied state and return structured results. Any approved
state change should be committed through the agreed shared update path.

## 7.3 Keep these concepts separate

  Concept               Meaning
  --------------------- ---------------------------------------------------------
  Maximum               Maximum resource instances a process may require
  Allocation            Instances currently held by a process
  Need                  Remaining possible demand: Maximum minus Allocation
  Outstanding Request   Resources a process is currently waiting for
  Available             Total instances minus all currently allocated instances

Do not use Need as a substitute for Outstanding Request in deadlock
detection.

## 7.4 Status rules

-   **SAFE:** At least one safe sequence exists for the current state.
-   **UNSAFE:** The system cannot guarantee that all processes can
    complete safely.
-   **DEADLOCKED:** The current outstanding requests and allocations
    show processes that cannot proceed under the detection model.

The GUI should not automatically equate UNSAFE with DEADLOCKED. Banker's
safety analysis and deadlock detection answer different questions.

## 7.5 Git and collaboration rules

-   Use one shared repository.
-   Keep a stable main branch.
-   Each member should work in a separate feature branch.
-   Use clear branch names, such as `feature/process-resource`,
    `feature/bankers-algorithm`, `feature/deadlock-recovery`, and
    `integration/app-integration`.
-   Commit small, meaningful changes.
-   Open pull requests or merge requests for review.
-   Do not independently rename shared fields or change function
    signatures without informing the team.
-   Merge stable work regularly and resolve conflicts as they arise.
-   Keep setup/run instructions up to date.

------------------------------------------------------------------------

# 8. Integrated test scenarios

The team should use shared test data so all modules can be checked
against the same system state.

## Test 1 --- Basic setup and matrix calculation

-   Create at least three processes and two resource types.
-   Enter total resources, Allocation, and Maximum.
-   Verify every Need value equals Maximum minus Allocation.
-   Verify every Available value equals Total minus the sum of
    allocations.
-   Confirm invalid negative values and over-allocation are rejected.

## Test 2 --- Safe state

-   Enter a known safe configuration.
-   Run Banker's Algorithm.
-   Verify the result is SAFE.
-   Verify the returned sequence contains every active process exactly
    once.
-   Verify each process can finish in the returned order using the Work
    calculation.
-   Verify that running the algorithm does not mutate live allocations.

## Test 3 --- Unsafe state

-   Enter a configuration for which no safe sequence exists.
-   Run Banker's Algorithm.
-   Verify the result is UNSAFE.
-   Verify no fabricated safe sequence is displayed.
-   Verify the original state remains unchanged.

## Test 4 --- Resource request that can be granted

-   Select a process and enter a request within its Need and Available
    resources.
-   Run the request safety check.
-   If the trial state is safe, verify that the request is granted.
-   Verify Available decreases and Allocation increases by the request.
-   Verify Need decreases by the same amount.
-   Verify the RAG reflects the allocation.

## Test 5 --- Resource request that must be denied

Test both cases:

-   Request exceeds the process's remaining Need.
-   Request exceeds currently Available resources.

Also test a request that passes those two preliminary checks but
produces an unsafe trial state. Verify that the request is denied and
the original live state is restored exactly.

## Test 6 --- Deadlock detection

-   Create a state with outstanding requests and allocations that
    produces a deadlock.
-   Run deadlock detection.
-   Verify the identified deadlocked processes.
-   Verify the algorithm uses Outstanding Request, not the Need Matrix.
-   Test a separate state where all processes can finish and confirm
    that no deadlock is reported.

## Test 7 --- Resource Allocation Graph

-   Verify process and resource nodes match the current state.
-   Verify allocation edges point from resource to process.
-   Verify request edges point from process to resource.
-   Allocate, request, and release resources.
-   Confirm the graph updates after each operation.

## Test 8 --- Recovery

-   Start from a detected deadlock.
-   Select a victim process and confirm recovery.
-   Verify the victim's allocated resources are returned to Available.
-   Verify its outstanding requests are cleared and its process status
    is updated.
-   Verify all matrices and the graph are refreshed.
-   Run detection again and show whether the deadlock has been resolved.

## Test 9 --- Full workflow

Starting from an empty system:

1.  Create processes.
2.  Create resources.
3.  Enter Allocation and Maximum matrices.
4.  Display Need and Available.
5.  Submit a resource request.
6.  Run Banker's Algorithm.
7.  Display the RAG.
8.  Run deadlock detection.
9.  Perform recovery if a deadlock exists.
10. Display the final system status.

## Test 10 --- GUI and application reliability

-   Check every screen at the expected window size.
-   Check empty, invalid, and boundary inputs.
-   Check that buttons and navigation work.
-   Check that errors are understandable.
-   Restart the application and verify the expected initial state.
-   Confirm that the application can be installed/run using the written
    instructions.

------------------------------------------------------------------------

# 9. Suggested execution schedule

Use the following as a relative schedule. Adjust the duration to match
the team's actual deadline.

  ------------------------------------------------------------------------------------
  Phase            Person 1           Person 2         Person 3         Person 4
  ---------------- ------------------ ---------------- ---------------- --------------
  Phase 1:         Review data        Review algorithm Review detection Create
  Agreement and    requirements       inputs/outputs   and graph        repository,
  setup                                                inputs/outputs   app shell,
                                                                        shared data
                                                                        contract, and
                                                                        interface
                                                                        document

  Phase 2:         Build              Build safety     Build detection, Prepare GUI
  Independent      process/resource   algorithm and    RAG, and         shell, mock
  implementation   setup, matrices,   request check    recovery logic   data,
                   requests           using agreed     using agreed     integration
                                      sample state     sample state     checklist, and
                                                                        review
                                                                        interfaces

  Phase 3: Module  Validate           Validate         Validate         Check
  completion       calculations and   safe/unsafe and  deadlock, graph, deliverables
                   state operations   request outcomes and recovery     and resolve
                                                       outcomes         contract
                                                                        questions
                                                                        before
                                                                        integration

  Phase 4:         Support fixes to   Support          Support          Merge and
  Integration      state and matrix   algorithm        detection,       connect all
                   connections        integration and  graph, and       modules;
                                      result display   recovery         resolve
                                                       integration      conflicts

  Phase 5: System  Verify setup and   Verify Banker's  Verify detection Run all
  testing          matrix workflows   scenarios        and recovery     end-to-end
                                                       scenarios        tests, fix
                                                                        defects, and
                                                                        prepare final
                                                                        demo
  ------------------------------------------------------------------------------------

**Suggested time allocation:** Spend approximately the first 10%
agreeing on contracts and setup, the next 50% on independent
implementation, the next 25% on integration, and the final 15% on
testing and demonstration. These percentages are planning guidance, not
a requirement from the project brief.

------------------------------------------------------------------------

# 10. Definition of done for the complete project

The project is ready for submission when:

-   [ ] Users can create processes and resources.
-   [ ] Users can enter Allocation and Maximum matrices.
-   [ ] Need Matrix is calculated and displayed correctly.
-   [ ] Available Resource Vector is calculated and displayed correctly.
-   [ ] Users can make resource requests.
-   [ ] Banker's Algorithm identifies safe and unsafe states.
-   [ ] A safe sequence is displayed when one exists.
-   [ ] Unsafe requests are rejected without changing the live state.
-   [ ] Outstanding requests are tracked separately from Need.
-   [ ] Deadlock detection identifies deadlocked processes.
-   [ ] Resource Allocation Graph displays allocation and request
    relationships.
-   [ ] Recovery can be demonstrated and updates the system state.
-   [ ] SAFE, UNSAFE, and DEADLOCKED are presented as distinct outcomes.
-   [ ] The GUI is complete and understandable.
-   [ ] All modules work together in one application.
-   [ ] Integrated test scenarios pass.
-   [ ] The project includes instructions to run and demonstrate it.

------------------------------------------------------------------------

# 11. Final handoff checklist

Each implementation member must hand Person 4:

1.  Completed source code for the assigned module.
2.  A list of public functions/components and their exact interfaces.
3.  Required input data format and returned output format.
4.  Any dependencies or setup instructions.
5.  Test cases and their expected results.
6.  Known limitations or unfinished items.
7.  Confirmation that the module uses the agreed shared data contract.

Person 4 should not integrate a module based only on screenshots or
verbal confirmation. The source code, interface details, and test
evidence must be provided.

## Final team rule

**Build independently, agree on interfaces early, and integrate against
one shared system state.** Persons 1--3 are responsible for the
correctness of their assigned modules. Person 4 is responsible for
making the modules work together as one complete, tested graphical
application.
