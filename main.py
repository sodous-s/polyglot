#!/usr/bin/env python3

import sys
import subprocess
from pathlib import Path
import argparse

def run_cmd(cmd):
    try:
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
        return result.stdout
    except Exception as e:
        return str(e)

def check_syntax(file_path, ext):
    if ext in ['.cpp', '.cc', '.cxx']:
        res = run_cmd(f"g++ -fsyntax-only {file_path} 2>&1")
        if res.strip():
            print(f"C++ syntax errors in {file_path}:\n{res}")
            return False
        return True
    elif ext == '.py':
        res = run_cmd(f"python -m py_compile {file_path} 2>&1")
        if res.strip():
            print(f"Python syntax errors in {file_path}:\n{res}")
            return False
        return True
    elif ext == '.rb':
        res = run_cmd(f"ruby -c {file_path} 2>&1")
        if "Syntax OK" not in res:
            print(f"Ruby syntax errors in {file_path}:\n{res}")
            return False
        return True
    elif ext == '.sh':
        res = run_cmd(f"bash -n {file_path} 2>&1")
        if res.strip():
            print(f"Bash syntax errors in {file_path}:\n{res}")
            return False
        return True
    else:
        print(f"Unsupported file extension: {ext}")
        return False

def main():
    parser = argparse.ArgumentParser(description='Merge two source files into a polyglot')
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
    print(f"Checking syntax for {file1}...")
    if not check_syntax(file1, ext1):
        return 1

    print(f"Checking syntax for {file2}...")
    if not check_syntax(file2, ext2):
        return 1

    # Read files
    with open(file1, 'r') as f:
        content1 = f.read().splitlines()
    with open(file2, 'r') as f:
        content2 = f.read().splitlines()

    # Merge files
    def open_fence(ext):
        if ext == '.py': return r"r'''"
        if ext == '.rb': return "=begin"
        if ext == '.sh': return ": '"
        return ""

    def close_fence(ext):
        if ext == '.py': return "'''"
        if ext == '.rb': return "=end"
        if ext == '.sh': return "'"
        return ""

    with open(out_file, 'w') as f:
        if ext1 in ['.cpp', '.cc', '.cxx']:
            # ext1 is C++, show it
            f.write(f"#if 0\n{open_fence(ext2)}\n#endif\n")
            for line in content1:
                f.write(line + '\n')
            f.write(f"#if 0\n{close_fence(ext2)}\n#endif\n")

            # hide ext2
            f.write("#if 0\n")
            for line in content2:
                f.write(line + '\n')
            f.write("#endif\n")

        elif ext2 in ['.cpp', '.cc', '.cxx']:
            # ext2 is C++, show it
            f.write(f"#if 0\n{open_fence(ext1)}\n#endif\n")
            for line in content2:
                f.write(line + '\n')
            f.write(f"#if 0\n{close_fence(ext1)}\n#endif\n")

            # hide ext1
            f.write("#if 0\n")
            for line in content1:
                f.write(line + '\n')
            f.write("#endif\n")

        else:
            print("Error: No C++ file in pair")
            return 1

    print(f"Merged into {out_file}")
    return 0

if __name__ == "__main__":
    sys.exit(main())
