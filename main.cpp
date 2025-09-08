#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <string_view>
#include <array>
#include <memory>
#include <filesystem>
#include <unordered_map>
#include <stdexcept>

namespace fs = std::filesystem;

std::string usageStr =
    "Usage: polyglot <source1> <source2> -o <outputFile>\n"
    "Supported extensions:\n"
    "  C++: .cpp, .cc, .cxx\n"
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

bool checkSyntax(const std::string& file, const std::string& ext) {
    std::string res;

    if (ext == ".cpp" || ext == ".cc" || ext == ".cxx") {
        res = runCmd("g++ -fsyntax-only " + file + " 2>&1");
        if (!res.empty()) {
            std::cerr << "C++ syntax errors in " << file << ":\n" << res;
            return false;
        }
        return true;
    } else if (ext == ".py") {
        res = runCmd("pyflakes " + file + " 2>&1");
        if (!res.empty()) {
            std::cerr << "Python syntax errors in " << file << ":\n" << res;
            std::cerr << "If python file does not have syntax errors, please check if pyflakes is installed." << std::endl;
            return false;
        }
        return true;
    } else if (ext == ".rb") {
        res = runCmd("ruby -c " + file + " 2>&1");
        if (res.find("Syntax OK") == std::string::npos) {
            std::cerr << "Ruby syntax errors in " << file << ":\n" << res;
            return false;
        }
        return true;
    } else if (ext == ".sh") {
        res = runCmd("bash -n " + file + " 2>&1");
        if (!res.empty()) {
            std::cerr << "Bash syntax errors in " << file << ":\n" << res;
            return false;
        }
        return true;
    }

    std::cerr << "Unsupported file extension: " << ext << "\n";
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
    const std::string& lang1, const std::vector<std::string>& content1,
    const std::string& lang2, const std::vector<std::string>& content2
) {
    std::ofstream out(outFile);
    if (!out.is_open()) throw std::runtime_error("Failed to open output: " + outFile);

    out << "#if 0\nr'''\n#endif\n";

    for (auto& l : content1) out << l << "\n";

    out << "#if 0\n'''\n#endif\n";
    out << "#if 0\n";

    for (auto& l : content2) out << l << "\n";

    out << "#endif\n";
}

int main(int argc, char* argv[]) {
    std::vector<std::string_view> args(argv, argv + argc);
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
