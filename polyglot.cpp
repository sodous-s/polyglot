#if 0
r'''
#endif

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <array>
#include <memory>
#include <filesystem>
#include <stdexcept>

namespace fs = std::filesystem;

std::string usageStr =
    "Usage: polyglot <source1> <source2> -o <outputFile>\n"
    "Supported extensions:\n"
    "  C/C++: .cpp, .cc, .cxx, .c\n"
    "  Python: .py\n"
    "  Ruby: .rb\n"
    "  Bash: .sh\n";
    
std::string runCmd(const std::string& cmd) {
    std::array<char, 256> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        result += buffer.data();
    return result;
}

// add this helper above checkSyntax (near the other helpers)
static std::string shellSafePath(const std::string& path) {
    // use generic_string() so Windows backslashes become forward slashes
    std::string s = fs::path(path).generic_string();

    // escape double quotes if present, then wrap the path in double-quotes
    std::string escaped;
    for (char c : s) {
        if (c == '"') escaped += "\\\"";
        else escaped += c;
    }
    return "\"" + escaped + "\"";
}

std::string replace(const std::string& str, const std::string& replace, const std::string& with) {
    if (replace.empty()) return str; // avoid infinite loop
    std::string result;
    result.reserve(str.size());
    std::size_t start = 0;
    std::size_t pos;
    while ((pos = str.find(replace, start)) != std::string::npos) {
        result.append(str, start, pos - start);
        result += with;
        start = pos + replace.length();
    }
    result.append(str, start, str.size() - start);
    return result;
}

bool checkSyntax(const std::string& file, const std::string& ext) {
    std::string res;
    std::string quoted = shellSafePath(file);

    if (ext == ".cpp" || ext == ".cc" || ext == ".cxx" || ext == ".c") {
        std::string flag = ext == ".c" ? "-x c " : "";
        // quote the path for safety
        res = runCmd("g++ -fsyntax-only " + flag + quoted + " 2>&1");
        if (!res.empty()) {
            std::cerr << "C/C++ syntax errors in " << file << ":\n" << res;
            return false;
        }
        return true;
    } else if (ext == ".py") {
        // Prefer a simple, widely-available check if pyflakes isn't installed.
        // Try pyflakes first; if it fails because pyflakes isn't present,
        // fall back to python -m py_compile.
        res = runCmd("pyflakes " + quoted + " 2>&1");
        if (!res.empty()) {
            // if pyflakes seems missing (its stderr often mentions 'No module named'),
            // try python compile fallback
            std::string fallback = runCmd("python -m py_compile " + quoted + " 2>&1");
            if (!fallback.empty()) {
                std::cerr << "Python syntax errors in " << file << ":\n" << fallback;
                std::cerr << "If pyflakes is desired, please install it or ensure it's on PATH.\n";
                return false;
            }
        }
        return true;
    } else if (ext == ".rb") {
        res = runCmd("ruby -c " + quoted + " 2>&1");
        if (res.find("Syntax OK") == std::string::npos) {
            std::cerr << "Ruby syntax errors in " << file << ":\n" << res;
            return false;
        }
        return true;
    } else if (ext == ".sh") {
        // use bash -n with a properly quoted, forward-slash path
        res = runCmd("bash -n " + quoted + " 2>&1");
        if (!res.empty()) {
            std::cerr << "Bash syntax errors in " << file << ":\n" << res;
            return false;
        }
        return true;
    }

    std::cerr << "\nUnsupported file extension: " << ext << "\n";
    return false;
}


std::vector<std::string> readFile(const std::string& filename) {
    std::ifstream in(filename);
    if (!in.is_open()) throw std::runtime_error("Failed to open: " + filename);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(in, line)) lines.push_back(line);
    return lines;
}

void writeMerged(
    const std::string& outFile,
    const std::string& ext1, const std::vector<std::string>& content1,
    const std::string& ext2, const std::vector<std::string>& content2
) {
    std::ofstream out(outFile, std::ios::binary);
    if (!out.is_open()) throw std::runtime_error("Failed to open output: " + outFile);

    auto openFence = [](const std::string& ext) -> std::string {
        if (ext == ".py") return "r\'\'\'";
        if (ext == ".rb") return "=begin";
        if (ext == ".sh") return ": '";
        return "";
    };

    auto closeFence = [](const std::string& ext) -> std::string {
        if (ext == ".py") return "\'\'\'";
        if (ext == ".rb") return "=end";
        if (ext == ".sh") return "'";
        return "";
    };

    auto escapeCpp = [](const std::string& content) -> std::string {
        return "#if 0\n" + content + "\n#endif\n";
    };

    auto writeLine = [&](const std::string& line) {
        out.write(line.c_str(), line.size());
        out.put('\n'); // always LF
    };

    auto escapeForPython = [](const std::string& line) -> std::string {
        std::string out;
        for (size_t i = 0; i < line.size(); i++) {
            if (i + 2 < line.size() && line[i] == '\'' && line[i+1] == '\'' && line[i+2] == '\'') {
                out += "\\'\\'\\'";
                i += 2;
            } else {
                out += line[i];
            }
        }
        return out;
    };


    // --- Block A: output C++ file, hide non-C++ fence ---
    if (ext1 == ".cpp" || ext1 == ".cc" || ext1 == ".cxx" || ext1 == ".c") {
        // ext1 is C++, show it
        writeLine(escapeCpp(openFence(ext2)));
        for (const std::string& l : content1) {
            writeLine(escapeForPython(l));
        }
        writeLine(escapeCpp(closeFence(ext2)));

        // hide ext2
        writeLine("#if 0");
        for (auto& l : content2) {
            writeLine(escapeForPython(l));
        }
        writeLine("#endif");
    }
    else if (ext2 == ".cpp" || ext2 == ".cc" || ext2 == ".cxx" || ext2 == ".c") {
        // ext2 is C++, show it
        writeLine(escapeCpp(openFence(ext1)));
        for (auto& l : content2) writeLine(l);
        writeLine(escapeCpp(closeFence(ext1)));

        // hide ext1
        writeLine("#if 0");
        for (auto& l : content1) writeLine(l);
        writeLine("#endif");
    }
    else {
        throw std::runtime_error("No C/C++ file in pair");
    }
}


int main(int argc, char* argv[]) {
    std::vector<std::string> args(argv, argv + argc);
    if (argc < 5) {
        std::cerr << usageStr;
        return 1;
    }

    std::string file1, file2, outFile;
    for (int i = 1; i < argc; i++) {
        if (args[i] == "-o") {
            if (i + 1 >= argc) {
                std::cerr << "Error: -o requires an argument\n";
                return 1;
            }
            outFile = args[++i];
        } else if (file1.empty()) {
            file1 = args[i];
        } else if (file2.empty()) {
            file2 = args[i];
        } else {
            std::cerr << "Error: unexpected argument: " << args[i] << "\n";
            return 1;
        }
    }

    if (file1.empty() || file2.empty() || outFile.empty()) {
        std::cerr << usageStr;
        return 1;
    }

    std::string ext1 = fs::path(file1).extension().string();
    std::string ext2 = fs::path(file2).extension().string();

    // Check syntax
    std::cout << "Checking syntax for " << file1 << "... ";
    if (!checkSyntax(file1, ext1)) {
        std::cerr << "\nSyntax error in " << file1 << "\n";
        return 1;
    }
    std::cout << "OK\n";

    std::cout << "Checking syntax for " << file2 << "... ";
    if (!checkSyntax(file2, ext2)) {
        std::cerr << "\nSyntax error in " << file2 << "\n";
        return 1;
    }
    std::cout << "OK\n";

    // Read + merge
    auto content1 = readFile(file1);
    auto content2 = readFile(file2);

    writeMerged(outFile, ext1, content1, ext2, content2);
    std::cout << "Merged into " << outFile << "\n";
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
import argparse
import shlex

def run_cmd(cmd):
    try:
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
        return result.stdout
    except Exception as e:
        return str(e)
    
def shell_safe(path: str) -> str:
    # Convert backslashes to forward slashes (for WSL/bash)
    p = Path(path).as_posix()
    # Quote the path safely for the shell
    return shlex.quote(p)

def check_syntax(file_path, ext):
    file_str = str(file_path)
    if ext in ['.cpp', '.cc', '.cxx', '.c']:
        flag = " -x c " if ext == ".c" else ""
        res = run_cmd(f"g++ -fsyntax-only {flag} {file_str} 2>&1")
        if res.strip():
            print(f"C++ syntax errors in {file_str}:\n{res}")
            return False
        return True
    elif ext == '.py':
        res = run_cmd(f"pyflakes {file_str} 2>&1")
        if res.strip():
            print(f"Python syntax errors in {file_str}:\n{res}")
            print("If python file does not have syntax errors, please check if pyflakes is installed.")
            return False
        return True
    elif ext == '.rb':
        res = run_cmd(f"ruby -c {file_str} 2>&1")
        if "Syntax OK" not in res:
            print(f"Ruby syntax errors in {file_str}:\n{res}")
            return False
        return True
    elif ext == '.sh':
        res = run_cmd(f"bash -n {shell_safe(file_str)} 2>&1")
        if res.strip():
            print(f"Bash syntax errors in {file_str}:\n{res}")
            return False
        return True
    else:
        print(f"Unsupported file extension: {ext}")
        return False

def main():
    usage_str = """Usage: polyglot <source1> <source2> -o <outputFile>
Supported extensions:
  C/C++: .cpp, .cc, .cxx, .c
  Python: .py
  Ruby: .rb
  Bash: .sh
"""
    
    parser = argparse.ArgumentParser(usage=usage_str)
    parser.add_argument('source1', help='First source file')
    parser.add_argument('source2', help='Second source file')
    parser.add_argument('-o', '--output', required=True, help='Output file')
    args = parser.parse_args()

    file1 = Path(args.source1)
    file2 = Path(args.source2)
    out_file = Path(args.output)

    ext1 = file1.suffix
    ext2 = file2.suffix

    # Check syntax
    print(f"Checking syntax for {file1}... ", end='')
    if not check_syntax(file1, ext1):
        print(f"Syntax error in {file1}")
        return 1
    print("OK")

    print(f"Checking syntax for {file2}... ", end='')
    if not check_syntax(file2, ext2):
        print(f"Syntax error in {file2}")
        return 1
    print("OK")

    # Read files
    with open(file1, 'r') as f:
        content1 = f.read().splitlines()
    with open(file2, 'r') as f:
        content2 = f.read().splitlines()

    # Determine fence tokens
    def open_fence(ext):
        if ext == '.py': return "r\'\'\'"
        if ext == '.rb': return "=begin"
        if ext == '.sh': return ": '"
        return ""

    def close_fence(ext):
        if ext == '.py': return "\'\'\'"
        if ext == '.rb': return "=end"
        if ext == '.sh': return "'"
        return ""

    # Write merged file
    with open(out_file, 'w', newline='\n') as f:
        if ext1 in ['.cpp', '.cc', '.cxx', '.c']:
            # Write C++ content with fence tokens for the other language
            f.write(f"#if 0\n{open_fence(ext2)}\n#endif\n")
            for line in content1:
                f.write(line + '\n')
            f.write(f"#if 0\n{close_fence(ext2)}\n#endif\n")
            
            # Write non-C++ content within #if 0 block
            f.write("#if 0\n")
            for line in content2:
                f.write(line + '\n')
            f.write("#endif\n")
            
        elif ext2 in ['.cpp', '.cc', '.cxx', '.c']:
            # Write C++ content with fence tokens for the other language
            f.write(f"#if 0\n{open_fence(ext1)}\n#endif\n")
            for line in content2:
                f.write(line + '\n')
            f.write(f"#if 0\n{close_fence(ext1)}\n#endif\n")
            
            # Write non-C++ content within #if 0 block
            f.write("#if 0\n")
            for line in content1:
                f.write(line + '\n')
            f.write("#endif\n")
            
        else:
            print("Error: No C/C++ file in pair")
            return 1

    print(f"Merged into {out_file}")
    return 0

if __name__ == "__main__":
    sys.exit(main())
#endif
