#if 0
r'''
#endif
#include <iostream>
#include <fstream> 
#include <vector>
#include <string_view>
#include <string>
#include <array>
#include <memory>
#include <filesystem>
#include <cstdio>

std::string usageStr = "Usage: polyglot <cppFile>.cpp <pyFile>.py <outFile>.cpp\n";

std::string system(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

std::string replace(const std::string& str, const std::string& from, const std::string& to) {
    if (from.empty()) return str;
    
    std::string result = str;
    size_t start_pos = 0;
    
    while ((start_pos = result.find(from, start_pos)) != std::string::npos) {
        result.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Move past the replacement
    }
    
    return result;
}

int main(int argc, char* argv[]) {
    std::vector<std::string_view> a(argv, argv + argc);

    if (a.size() != 4) {
        std::cerr << "Error: expected 3 arguments, got " << a.size() - 1 << std::endl;
        std::cout << usageStr;
        return 1;
    }

    std::string cppFileName;
    std::string pyFileName;
    std::string outFileName;

    namespace fs = std::filesystem;

    fs::path arg1(a[1]);
    fs::path arg2(a[2]);

    if (arg1.extension() == ".cpp" && arg2.extension() == ".py") {
        cppFileName = arg1.string();
        pyFileName  = arg2.string();
        outFileName = a[3];
    } else if (arg1.extension() == ".py" && arg2.extension() == ".cpp") {
        pyFileName  = arg1.string();
        cppFileName = arg2.string();
        outFileName = a[3];
    } else {
        std::cout << "Usage not recognized." << std::endl;
        std::cout << usageStr;
    }


    std::ifstream cppFileStream(cppFileName);
    std::ifstream pyFileStream(pyFileName);
    
    if (!cppFileStream.is_open() || !pyFileStream.is_open()) {
        std::cerr << "File creation failed" << std::endl;
        return 1;
    }
    
    std::cout << "Trying to compile c++ program" << std::endl;
    std::string compileOutput = system("g++ " + cppFileName);
    if (compileOutput.find("Error") != std::string::npos) {
        std::cout << "Syntax Errors present!" << std::endl;
        return 1;
    }
    std::cout << "Removing temp file from compilation" << std::endl;

    #if defined(_WIN32) || defined(_WIN64) || defined(__linux__) || defined(__APPLE__)
        int result = remove("a.exe");
        if (result != 0) {
            std::cout << "Temp file a.exe cannot be removed" << std::endl;
        }
    #endif
    std::cout << "No syntax errors, code compiled successfully" << std::endl;


checkPySyntax:
    std::cout << "Checking python file syntax" << std::endl;
    std::string pyOutput = system("pyflakes " + pyFileName);

    if (pyOutput.find("command not found") != std::string::npos
        || pyOutput.find("CommandNotFound") != std::string::npos
        || pyOutput.find("pyflakes") != std::string::npos
    ) {
        std::cout << "PyFlakes not installed. Proceed to install? [Y/n]" << std::endl;
        std::string proceed;
        std::cin >> proceed;
        if (proceed == "Y" || proceed == "") {
            if (!system("pip install pyflakes")) {
                std::cout << "Fatal: pip not installed. Install pip, and use pip to install pyflakes" << std::endl;
                return 1;
            } else {
                goto checkPySyntax;
            }
        }
    } else if (pyOutput.find("invalid syntax") != std::string::npos) {
        std::cout << "Syntax Errors present!" << std::endl;
        return 1;
    }
    std::cout << "No syntax errors, python syntax check passed" << std::endl;

    std::vector<std::string> cppFileContent;
    std::vector<std::string> pyFileContent;
    std::string line;
    while (std::getline(cppFileStream, line)) {
        if (line.find("\'\'\'") != std::string::npos
            && line.find("\\'\\'\\'") == std::string::npos) {
            line = replace(line, "\'\'\'", "\\'\\'\\'");
        }
        cppFileContent.push_back(line);
    }
    while (std::getline(pyFileStream, line)) {
        pyFileContent.push_back(line);
    }
    

    std::cout << "Merging files..." << std::endl;
    
    std::ofstream outFileStream(outFileName);
    outFileStream << "#if 0\nr\'\'\'\n#endif\n";

    // insert c++ code
    for (const std::string& l : cppFileContent) {
        outFileStream << l << "\n";
    }

    outFileStream << "#if 0\n\'\'\'\n#endif\n";

    outFileStream << "#if 0\n";

    // insert python code
    for (const std::string& l : pyFileContent) {
        outFileStream << l << "\n";
    }

    outFileStream << "#endif" << std::endl;

    std::cout << "Finished. Check " << outFileName << std::endl;


    return 0;
}
#if 0
'''
#endif
#if 0
#!/usr/bin/env python3

import sys
import subprocess
from pathlib import Path
import shutil

USAGE = "Usage: polyglot.py <cppFile>.cpp <pyFile>.py <outFile>.cpp\n"


def run_cmd(cmd, capture_output=True, text=True):
    try:
        return subprocess.run(cmd, capture_output=capture_output, text=text, check=False)
    except FileNotFoundError as e:
        # command not found
        class Dummy:
            returncode = 127
            stdout = ""
            stderr = str(e)
        return Dummy()


def check_cpp_syntax(cpp_path: Path) -> bool:
    print("Trying to compile C++ program (syntax-only check)...")
    # -fsyntax-only checks syntax without producing an object/binary
    proc = run_cmd(["g++", "-fsyntax-only", str(cpp_path)])
    # Many compilers print errors to stderr
    output = (proc.stdout or "") + (proc.stderr or "")
    if proc.returncode != 0:
        print("Syntax Errors present in C++ file. Compiler output:")
        print(output)
        return False
    print("No syntax errors, C++ syntax check passed.")
    return True


def check_py_syntax(py_path: Path) -> bool:
    print("Checking Python file syntax with pyflakes...")
    # Try invoking pyflakes as a module first (works if installed for current python)
    proc = run_cmd([sys.executable, "-m", "pyflakes", str(py_path)])
    stdout = proc.stdout or ""
    stderr = proc.stderr or ""

    # Detect module-not-found style message
    module_missing = "No module named pyflakes" in stderr or ("pyflakes" in stderr and "not found" in stderr.lower()) or proc.returncode == 127

    if module_missing:
        print("pyflakes is not installed for this Python interpreter.")
        # Ask user if they want to install
        proceed = input("PyFlakes not installed. Proceed to install? [Y/n] ").strip()
        if proceed == "" or proceed.lower() == "y":
            print("Installing pyflakes using pip...")
            pip_proc = run_cmd([sys.executable, "-m", "pip", "install", "pyflakes"])
            pip_out = (pip_proc.stdout or "") + (pip_proc.stderr or "")
            if pip_proc.returncode != 0:
                print("Fatal: pip install failed. Install pip and/or pyflakes manually and re-run the script.")
                print("pip output:")
                print(pip_out)
                return False
            # re-run pyflakes
            proc = run_cmd([sys.executable, "-m", "pyflakes", str(py_path)])
            stdout = proc.stdout or ""
            stderr = proc.stderr or ""
        else:
            print("Aborting: pyflakes required to continue.")
            return False

    # If pyflakes returns non-zero, it might have found issues and printed them to stdout/stderr.
    if proc.returncode != 0:
        combined = (stdout or "") + (stderr or "")
        # pyflakes prints messages about issues; treat any output as "errors/warnings"
        if combined.strip():
            print("pyflakes reported issues:")
            print(combined)
            # treat as fatal as original code treats 'invalid syntax' as fatal
            return False
        else:
            # No output but nonzero? treat as failure.
            print("pyflakes returned non-zero exit code; output:")
            print(combined)
            return False

    # pyflakes exitcode == 0 => no issues
    print("No syntax errors, python syntax check passed.")
    return True


def merge_files(cpp_path: Path, py_path: Path, out_path: Path) -> None:
    print("Merging files...")
    try:
        cpp_text = cpp_path.read_text(encoding="utf-8")
    except Exception as e:
        print(f"Failed to read C++ file: {e}")
        raise

    try:
        py_text = py_path.read_text(encoding="utf-8")
    except Exception as e:
        print(f"Failed to read Python file: {e}")
        raise

    merged = []
    merged.append("#if 0")
    merged.append("'''")
    merged.append("#endif")
    merged.append("")  # blank line
    merged.append(cpp_text.rstrip("\n"))  # preserve original except trailing newline
    merged.append("")  # ensure separation
    merged.append("#if 0")
    merged.append("'''")
    merged.append("#endif")
    merged.append("")  # blank line
    merged.append("#if 0")
    merged.append(py_text.rstrip("\n"))
    merged.append("#endif")

    out_path.write_text("\n".join(merged) + "\n", encoding="utf-8")
    print(f"Finished. Check {out_path}")


def main(argv):
    if len(argv) != 4:
        print("Error: expected 3 arguments, got", len(argv) - 1)
        print(USAGE, end="")
        return 1

    # argv[1], argv[2] may be in either order
    p1 = Path(argv[1])
    p2 = Path(argv[2])
    outp = Path(argv[3])

    if not p1.exists() or not p2.exists():
        print("Error: one or more input files do not exist.")
        print(f"Got: {p1} (exists={p1.exists()}), {p2} (exists={p2.exists()})")
        return 1

    # determine which is which
    if p1.suffix == ".cpp" and p2.suffix == ".py":
        cpp_path, py_path = p1, p2
    elif p1.suffix == ".py" and p2.suffix == ".cpp":
        py_path, cpp_path = p1, p2
    else:
        print("Usage not recognized.")
        print(USAGE, end="")
        return 1

    # C++ syntax check
    if not check_cpp_syntax(cpp_path):
        return 1

    # Python syntax check via pyflakes
    if not check_py_syntax(py_path):
        return 1

    # Merge
    try:
        merge_files(cpp_path, py_path, outp)
    except Exception as e:
        print("Failed to merge files:", e)
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
#endif
