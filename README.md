# polyglot

(this README is ai generated)

polyglot is a small tool that merges a C++ source file and a Python source file into a single ".cpp" output file that can be used with C++ toolchains while preserving the Python portion.

## Purpose
The generated output wraps sections so the C++ compiler ignores the Python code and the Python interpreter sees the Python section as a normal script region. The main program (main.cpp) validates syntax using g++ and pyflakes before merging.

## Requirements
- g++ (for C++ syntax check and compiling the tool)
- Python (for running pyflakes and for running Python code if you extract it)
- pip (to install pyflakes if absent)
- (optional) pyflakes for Python syntax checking

## Build
From the repository root:
- Compile the tool:
  - g++ main.cpp -o polyglot

## Usage
polyglot expects three arguments:
- polyglot <cppFile>.cpp <pyFile>.py <outFile>.cpp

Example:
- Using the sample files in this repo:
  - ./polyglot ./test/test.cpp ./test/test.py ./test/out.cpp

After running, check `out.cpp` for the merged result.

## Notes & Troubleshooting
- If `pyflakes` is not installed, the tool will prompt to install it via pip.
  - Alternatively run: python -m pip install pyflakes
- On some systems the `pyflakes` command might not be on PATH. Try: python -m pyflakes <file>.py
- The tool uses system commands (popen). Ensure your environment allows executing those commands.
- The output file is a `.cpp` file that will compile as C++ (Python code is placed in regions ignored by the C++ preprocessor). If you want to run the Python portion directly, extract the Python region into a `.py` file.

## Example files
- test/test.cpp — simple C++ example
- test/test.py — simple Python example
- test/out.cpp — example of a previously merged output

### Complete command
```bash
clang++ main.cpp -o polyglot;
./polyglot ./test/test.cpp ./test/test.py ./test/out.cpp;

python ./test/out.cpp;

clang++ ./test/out.cpp -o ./test/out;
./out; 
```

## Contribution
Small fixes and improvements are welcome. Open an issue or submit a PR. Current issues:
1. If python code has docstrings, it breaks

## License
MIT License
