#include <iostream>
#include <fstream> 
#include <vector>
#include <string_view>
#include <string>
#include <array>
#include <memory>

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

    if (a[1][a[1].size() - 4] == '.' 
        && a[1][a[1].size() - 3] == 'c' 
        && a[1][a[1].size() - 2] == 'p'
        && a[1][a[1].size() - 1] == 'p'

        && a[2][a[2].size() - 3] == '.' 
        && a[2][a[2].size() - 2] == 'p' 
        && a[2][a[2].size() - 1] == 'y'
    ) {
        cppFileName = a[1];
        pyFileName = a[2];
        outFileName = a[3];
    } else if (a[1][a[1].size() - 3] == '.'
            && a[1][a[1].size() - 2] == 'p'
            && a[1][a[1].size() - 1] == 'y' 

            && a[2][a[2].size() - 4] == '.' 
            && a[2][a[2].size() - 3] == 'c' 
            && a[2][a[2].size() - 2] == 'p' 
            && a[2][a[2].size() - 1] == 'p'
    ) {
        pyFileName = a[1];
        cppFileName = a[2];
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
        cppFileContent.push_back(line);
    }
    while (std::getline(pyFileStream, line)) {
        pyFileContent.push_back(line);
    }
    

    std::cout << "Merging files..." << std::endl;
    
    std::ofstream outFileStream(outFileName);
    outFileStream << "#if 0\n'''\n#endif\n";

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