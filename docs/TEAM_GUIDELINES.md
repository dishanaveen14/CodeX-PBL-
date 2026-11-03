# Team Development Guidelines

## 1. Golden Rules for Developers (Persons 1, 2, 3, 4)

1. **Pure C Code Only:**
   - Every file must compile as C (`.c` / `.h`).
   - Standard: C11 (`-std=c11`).
   - Do not use C++, Python, or external runtime dependencies.

2. **Respect Module Boundaries:**
   - Put your header declarations in `include/<your_module>/`.
   - Put your function implementations in `src/<your_module>/`.
   - Do NOT modify another member's header signatures without communicating with Person 4.

3. **Adhere to the Shared Data Contract:**
   - Never redefine `SystemState`, `Process`, `Resource`, or error codes in your local file.
   - Always include `../../include/common/system_state.h` and `../../include/common/types.h`.
   - Use row index $i$ for processes, column index $j$ for resources.

4. **Safety & Rolled-Back State:**
   - Algorithm functions should be side-effect-free unless explicitly documented to mutate state.
   - Person 2's Banker's algorithm must operate on a temporary working copy (`Work`, `Finish`).
   - Person 3's Deadlock detection must read `outstanding_requests`, not `need`.

5. **Error Return Convention:**
   - Always return `ErrorCode` from state operations.
   - Return descriptive codes (`ERR_EXCEEDS_NEED`, `ERR_INSUFFICIENT_AVAILABLE`, etc.) rather than generic -1.

6. **Testing Your Code:**
   - Add unit test cases for your functions in `tests/test_skeleton.c` or create a new test file under `tests/`.
   - Run tests regularly using `build.bat test` or `powershell .\build.ps1 test`.

