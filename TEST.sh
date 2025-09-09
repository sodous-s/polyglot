#!/usr/bin/env bash

# compile polyglot
g++ main.cpp -o polyglot;

# run tests
echo '===RUNNING TESTS FOR C++ & PYTHON (C++ binary)===';
./polyglot ./test/test.cpp ./test/test.py -o ./test/out.cpp;

g++ ./test/out.cpp -o ./test/out;
./test/out;

python ./test/out.cpp;

echo '===RUNNING TESTS FOR PYTHON & C++ (C++ binary)==='
./polyglot ./test/test.py ./test/test.cpp -o ./test/out.cpp;

g++ ./test/out.cpp -o ./test/out;
./test/out;

python ./test/out.cpp;

echo '===RUNNING TESTS FOR C++ & RUBY (C++ binary)===';
./polyglot ./test/test.cpp ./test/test.rb -o ./test/out.cpp;

g++ ./test/out.cpp -o ./test/out;
./test/out;

ruby ./test/out.cpp;

echo '===RUNNING TESTS FOR RUBY & C++ (C++ binary)===';
./polyglot ./test/test.rb ./test/test.cpp -o ./test/out.cpp;

g++ ./test/out.cpp -o ./test/out;
./test/out;

ruby ./test/out.cpp;

echo '===RUNNING TESTS FOR C++ & BASH (C++ binary)===';
./polyglot ./test/test.cpp ./test/test.sh -o ./test/out.cpp;

g++ ./test/out.cpp -o ./test/out;
./test/out;

bash ./test/out.cpp;

echo '===RUNNING TESTS FOR BASH & C++ (C++ binary)===';
./polyglot ./test/test.sh ./test/test.cpp -o ./test/out.cpp;

g++ ./test/out.cpp -o ./test/out;
./test/out;

bash ./test/out.cpp;

# Additional tests using Python version
echo '===RUNNING TESTS FOR C++ & PYTHON (Python script)===';
python main.py ./test/test.cpp ./test/test.py -o ./test/out.cpp;

g++ ./test/out.cpp -o ./test/out;
./test/out;

python ./test/out.cpp;

echo '===RUNNING TESTS FOR PYTHON & C++ (Python script)===';
python main.py ./test/test.py ./test/test.cpp -o ./test/out.cpp;

g++ ./test/out.cpp -o ./test/out;
./test/out;

python ./test/out.cpp;

echo '===RUNNING TESTS FOR C++ & RUBY (Python script)===';
python main.py ./test/test.cpp ./test/test.rb -o ./test/out.cpp;

g++ ./test/out.cpp -o ./test/out;
./test/out;

ruby ./test/out.cpp;

echo '===RUNNING TESTS FOR RUBY & C++ (Python script)===';
python main.py ./test/test.rb ./test/test.cpp -o ./test/out.cpp;

g++ ./test/out.cpp -o ./test/out;
./test/out;

ruby ./test/out.cpp;

echo '===RUNNING TESTS FOR C++ & BASH (Python script)===';
python main.py ./test/test.cpp ./test/test.sh -o ./test/out.cpp;

g++ ./test/out.cpp -o ./test/out;
./test/out;

bash ./test/out.cpp;

echo '===RUNNING TESTS FOR BASH & C++ (Python script)===';
python main.py ./test/test.sh ./test/test.cpp -o ./test/out.cpp;

g++ ./test/out.cpp -o ./test/out;
./test/out;

bash ./test/out.cpp;


echo '===RUNNING TESTS FOR C & PYTHON (C binary)===';
./polyglot ./test/test.c ./test/test.py -o ./test/out.c;

gcc ./test/out.c -o ./test/out;
./test/out;

python ./test/out.c;

echo '===RUNNING TESTS FOR PYTHON & C (C binary)===';
./polyglot ./test/test.py ./test/test.c -o ./test/out.c;

gcc ./test/out.c -o ./test/out;
./test/out;

python ./test/out.c;

echo '===RUNNING TESTS FOR C & RUBY (C binary)===';
./polyglot ./test/test.c ./test/test.rb -o ./test/out.c;

gcc ./test/out.c -o ./test/out;
./test/out;

ruby ./test/out.c;

echo '===RUNNING TESTS FOR RUBY & C (C binary)===';
./polyglot ./test/test.rb ./test/test.c -o ./test/out.c;

gcc ./test/out.c -o ./test/out;
./test/out;

ruby ./test/out.c;

echo '===RUNNING TESTS FOR C & BASH (C binary)===';
./polyglot ./test/test.c ./test/test.sh -o ./test/out.c;

gcc ./test/out.c -o ./test/out;
./test/out;

bash ./test/out.c;

echo '===RUNNING TESTS FOR BASH & C (C binary)===';
./polyglot ./test/test.sh ./test/test.c -o ./test/out.c;

gcc ./test/out.c -o ./test/out;
./test/out;

bash ./test/out.c;


# Python script version
echo '===RUNNING TESTS FOR C & PYTHON (Python script)===';
python main.py ./test/test.c ./test/test.py -o ./test/out.c;

gcc ./test/out.c -o ./test/out;
./test/out;

python ./test/out.c;

echo '===RUNNING TESTS FOR PYTHON & C (Python script)===';
python main.py ./test/test.py ./test/test.c -o ./test/out.c;

gcc ./test/out.c -o ./test/out;
./test/out;

python ./test/out.c;

echo '===RUNNING TESTS FOR C & RUBY (Python script)===';
python main.py ./test/test.c ./test/test.rb -o ./test/out.c;

gcc ./test/out.c -o ./test/out;
./test/out;

ruby ./test/out.c;

echo '===RUNNING TESTS FOR RUBY & C (Python script)===';
python main.py ./test/test.rb ./test/test.c -o ./test/out.c;

gcc ./test/out.c -o ./test/out;
./test/out;

ruby ./test/out.c;

echo '===RUNNING TESTS FOR C & BASH (Python script)===';
python main.py ./test/test.c ./test/test.sh -o ./test/out.c;

gcc ./test/out.c -o ./test/out;
./test/out;

bash ./test/out.c;

echo '===RUNNING TESTS FOR BASH & C (Python script)===';
python main.py ./test/test.sh ./test/test.c -o ./test/out.c;

gcc ./test/out.c -o ./test/out;
./test/out;

bash ./test/out.c;
