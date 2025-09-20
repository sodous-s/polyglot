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
