# polyglot

(most of this README is ai generated)

polyglot is a small tool that merges two source files (for example C++ + Python) into a single ".cpp" output file that can be used with C++ toolchains while preserving the other-language portion.

## Purpose
The generated output wraps sections so the C++ compiler ignores the other-language code and the interpreter for that language sees the section as a normal script region. The main program (`main.cpp`) validates syntax using external checkers (for example `g++` for C++ and `pyflakes` for Python) before merging. This tool can be used to **produce a single file that behaves differently when run by Python (or another interpreter) or compiled as C++.**

## Requirements
- g++ (for C++ syntax checking and compiling the tool)
- pip (to install `pyflakes` if absent)
- `pyflakes` for Python syntax checking (optional)

The tool also supports syntax checking for other languages via system tools:
- Ruby (`ruby -c`) — files with `.rb`
- Bash (`bash -n`) — files with `.sh`

## Build
From the repository root:
- Compile the tool:
  - ```bash
    g++ main.cpp -o polyglot
    ```

## Usage
polyglot expects two source files and an output file specified with the `-o` option:

```bash
polyglot <source1> <source2> -o <outputFile>
```

The order of the two source files doesn't matter. The program determines which checks to run based on file extensions.

Example (using the sample files in this repo):

```bash
./polyglot ./test/test.cpp ./test/test.py -o ./test/out.cpp
```

After running, check your chosen output file (for example `./test/out.cpp`) for the merged result.

## Running tests

```bash
g++ test_runner.cpp -o runtests;
./runtests
```

## Notes & Troubleshooting
Notes & Troubleshooting
- If `pyflakes` is not installed the tool will print the error from the attempted check; install it or skip using Python source files.
  - Install manually: `python -m pip install pyflakes`
- On some systems the `pyflakes` executable might not be on PATH; use `python -m pyflakes <file>.py` instead.
- The tool runs external commands (via popen). Ensure `g++`, `bash`, and `ruby` are available if you use those source file types.
- The output file is a `.cpp` file that will compile as C++ and can also be run by an interpreter (for example `python out.cpp`).

## Example files
- test/test.cpp — simple C++ example
- test/test.py — simple Python example
- test/out.cpp — example of a previously merged output

### Complete command
```bash
g++ main.cpp -o polyglot
./polyglot ./test/test.cpp ./test/test.py -o ./test/out.cpp

python ./test/out.cpp

g++ ./test/out.cpp -o ./test/out
./test/out
```

## Contribution
Small fixes and improvements are welcome. Open an issue or submit a PR. Current issues:
1. Improve python triple string escape (currently works for simple cases but not tested for edge cases)
2. Fix python string escape in polyglot.cpp (the merged file from main.cpp and main.py). Running it with python causes a syntax error in 97, suggesting insufficient string escape

## License
MIT License
