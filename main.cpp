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
        if (line.find("'''") != std::string::npos
            && line.find("\\'\\'\\'") == std::string::npos) {
            line = replace(line, "'''", "\\'\\'\\'");
        }
        cppFileContent.push_back(line);
    }
    while (std::getline(pyFileStream, line)) {
        pyFileContent.push_back(line);
    }
    

    std::cout << "Merging files..." << std::endl;
    
    std::ofstream outFileStream(outFileName);
    outFileStream << "#if 0\nr'''\n#endif\n";

    // insert c++ code
    for (const std::string& l : cppFileContent) {
        outFileStream << l << "\n";
    }

    outFileStream << "#if 0\n'''\n#endif\n";

    outFileStream << "#if 0\n";

    // insert python code
    for (const std::string& l : pyFileContent) {
        outFileStream << l << "\n";
    }

    outFileStream << "#endif" << std::endl;

    std::cout << "Finished. Check " << outFileName << std::endl;


    return 0;
}