#if 0
r'''
#endif

//main.cpp
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
    "  Bash: .sh\n"
    "  Perl: .pl\n";  // 添加Perl支持说明
    
std::string runCmd(const std::string& cmd) {
    std::array<char, 256> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        result += buffer.data();
    return result;
}

static std::string shellSafePath(const std::string& path) {
    std::string s = fs::path(path).generic_string();
    std::string escaped;
    for (char c : s) {
        if (c == '"') escaped += "\\\"";
        else escaped += c;
    }
    return "\"" + escaped + "\"";
}

std::string replace(const std::string& str, const std::string& replace, const std::string& with) {
    if (replace.empty()) return str;
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
        res = runCmd("g++ -fsyntax-only " + flag + quoted + " 2>&1");
        if (!res.empty()) {
            std::cerr << "C/C++ syntax errors in " << file << ":\n" << res;
            return false;
        }
        return true;
    } else if (ext == ".py") {
        res = runCmd("pyflakes " + quoted + " 2>&1");
        if (!res.empty()) {
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
        res = runCmd("bash -n " + quoted + " 2>&1");
        if (!res.empty()) {
            std::cerr << "Bash syntax errors in " << file << ":\n" << res;
            return false;
        }
        return true;
    } else if (ext == ".pl") {  // 添加Perl语法检查
        res = runCmd("perl -c " + quoted + " 2>&1");
        if (res.find("syntax OK") == std::string::npos) {
            std::cerr << "Perl syntax errors in " << file << ":\n" << res;
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
        if (ext == ".py") return "r'''";
        if (ext == ".rb") return "=begin";
        if (ext == ".sh") return ": '";
        if (ext == ".pl") return "=pod";  // 添加Perl围栏开始
        return "";
    };

    auto closeFence = [](const std::string& ext) -> std::string {
        if (ext == ".py") return "'''";
        if (ext == ".rb") return "=end";
        if (ext == ".sh") return "'";
        if (ext == ".pl") return "=cut";  // 添加Perl围栏结束
        return "";
    };

    auto escapeCpp = [](const std::string& content) -> std::string {
        return "#if 0\n" + content + "\n#endif\n";
    };

    auto writeLine = [&out](const std::string& line) {
        out.write(line.c_str(), line.size());
        out.put('\n');
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

    if (ext1 == ".cpp" || ext1 == ".cc" || ext1 == ".cxx" || ext1 == ".c") {
        writeLine(escapeCpp(openFence(ext2)));
        for (const std::string& l : content1) {
            writeLine(escapeForPython(l));
        }
        writeLine(escapeCpp(closeFence(ext2)));

        writeLine("#if 0");
        for (auto& l : content2) {
            writeLine(escapeForPython(l));
        }
        writeLine("#endif");
    }
    else if (ext2 == ".cpp" || ext2 == ".cc" || ext2 == ".cxx" || ext2 == ".c") {
        writeLine(escapeCpp(openFence(ext1)));
        for (auto& l : content2) writeLine(l);
        writeLine(escapeCpp(closeFence(ext1)));

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
import subprocess
import sys
import os
from pathlib import Path

usage_str = """Usage: polyglot <source1> <source2> -o <outputFile>
Supported extensions:
  C/C++: .cpp, .cc, .cxx, .c
  Python: .py
  Ruby: .rb
  Bash: .sh
  Perl: .pl"""

def run_cmd(cmd):
    try:
        result = subprocess.run(cmd, shell=True, check=True, 
                              stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                              text=True)
        return result.stdout
    except subprocess.CalledProcessError as e:
        return e.stderr

def shell_safe_path(path):
    path_str = str(Path(path))
    escaped = path_str.replace('"', '\\"')
    return f'"{escaped}"'

def check_syntax(file, ext):
    quoted = shell_safe_path(file)
    
    if ext in ('.cpp', '.cc', '.cxx', '.c'):
        flag = '-x c ' if ext == '.c' else ''
        res = run_cmd(f"g++ -fsyntax-only {flag}{quoted}")
        if res.strip():
            print(f"C/C++ syntax errors in {file}:\n{res}", file=sys.stderr)
            return False
        return True
    elif ext == '.py':
        res = run_cmd(f"pyflakes {quoted}")
        if res.strip():
            fallback = run_cmd(f"python -m py_compile {quoted}")
            if fallback.strip():
                print(f"Python syntax errors in {file}:\n{fallback}", file=sys.stderr)
                print("If pyflakes is desired, please install it or ensure it's on PATH.", file=sys.stderr)
                return False
        return True
    elif ext == '.rb':
        res = run_cmd(f"ruby -c {quoted}")
        if "Syntax OK" not in res:
            print(f"Ruby syntax errors in {file}:\n{res}", file=sys.stderr)
            return False
        return True
    elif ext == '.sh':
        res = run_cmd(f"bash -n {quoted}")
        if res.strip():
            print(f"Bash syntax errors in {file}:\n{res}", file=sys.stderr)
            return False
        return True
    elif ext == '.pl':
        res = run_cmd(f"perl -c {quoted}")
        if "syntax OK" not in res:
            print(f"Perl syntax errors in {file}:\n{res}", file=sys.stderr)
            return False
        return True
    
    print(f"\nUnsupported file extension: {ext}\n", file=sys.stderr)
    return False

def read_file(filename):
    with open(filename, 'r', encoding='utf-8') as f:
        return f.readlines()

def write_merged(out_file, ext1, content1, ext2, content2):
    def open_fence(ext):
        if ext == '.py': return "r'''"
        if ext == '.rb': return "=begin"
        if ext == '.sh': return ": '"
        if ext == '.pl': return "=pod"
        return ""
    
    def close_fence(ext):
        if ext == '.py': return "'''"
        if ext == '.rb': return "=end"
        if ext == '.sh': return "'"
        if ext == '.pl': return "=cut"
        return ""
    
    def escape_cpp(content):
        return "#if 0\n" + content + "\n#endif\n"
    
    def escape_for_python(line):
        return line.replace("'''", "\\'\\'\\'")
    
    with open(out_file, 'w', encoding='utf-8') as out:
        def write_line(line):
            out.write(line + '\n')
        
        if ext1 in ('.cpp', '.cc', '.cxx', '.c'):
            write_line(escape_cpp(open_fence(ext2)))
            for line in content1:
                write_line(escape_for_python(line.rstrip('\n')))
            write_line(escape_cpp(close_fence(ext2)))
            
            write_line("#if 0")
            for line in content2:
                write_line(escape_for_python(line.rstrip('\n')))
            write_line("#endif")
        elif ext2 in ('.cpp', '.cc', '.cxx', '.c'):
            write_line(escape_cpp(open_fence(ext1)))
            for line in content2:
                write_line(line.rstrip('\n'))
            write_line(escape_cpp(close_fence(ext1)))
            
            write_line("#if 0")
            for line in content1:
                write_line(line.rstrip('\n'))
            write_line("#endif")
        else:
            raise RuntimeError("No C/C++ file in pair")

def main():
    args = sys.argv[1:]
    if len(args) < 4:
        print(usage_str, file=sys.stderr)
        sys.exit(1)
    
    file1 = file2 = out_file = None
    i = 0
    while i < len(args):
        if args[i] == '-o':
            if i + 1 >= len(args):
                print("Error: -o requires an argument", file=sys.stderr)
                sys.exit(1)
            out_file = args[i+1]
            i += 2
        elif file1 is None:
            file1 = args[i]
            i += 1
        elif file2 is None:
            file2 = args[i]
            i += 1
        else:
            print(f"Error: unexpected argument: {args[i]}", file=sys.stderr)
            sys.exit(1)
    
    if not file1 or not file2 or not out_file:
        print(usage_str, file=sys.stderr)
        sys.exit(1)
    
    ext1 = Path(file1).suffix
    ext2 = Path(file2).suffix
    
    print(f"Checking syntax for {file1}... ", end='')
    if not check_syntax(file1, ext1):
        print(f"\nSyntax error in {file1}", file=sys.stderr)
        sys.exit(1)
    print("OK")
    
    print(f"Checking syntax for {file2}... ", end='')
    if not check_syntax(file2, ext2):
        print(f"\nSyntax error in {file2}", file=sys.stderr)
        sys.exit(1)
    print("OK")
    
    content1 = read_file(file1)
    content2 = read_file(file2)
    
    write_merged(out_file, ext1, content1, ext2, content2)
    print(f"Merged into {out_file}")

if __name__ == "__main__":
    main()
#endif
