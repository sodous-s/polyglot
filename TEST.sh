# compile polyglot
g++ main.cpp -o polyglot;

# run tests
echo "===RUNNING TESTS FOR C++ & PYTHON===";
./polyglot ./test/test.cpp ./test/test.py -o ./test/out.cpp;

g++ ./test/out.cpp -o ./test/out;
./test/out;

python ./test/out.cpp;


echo "===RUNNING TESTS FOR PYTHON & C++==="
./polyglot ./test/test.py ./test/test.cpp -o ./test/out.cpp;

g++ ./test/out.cpp -o ./test/out;
./test/out;

python ./test/out.cpp;


echo "===RUNNING TESTS FOR C++ & RUBY===";
./polyglot ./test/test.cpp ./test/test.rb -o ./test/out.cpp;

g++ ./test/out.cpp -o ./test/out;
./test/out;

ruby ./test/out.cpp;


echo "===RUNNING TESTS FOR RUBY & C++===";
./polyglot ./test/test.rb ./test/test.cpp -o ./test/out.cpp;

g++ ./test/out.cpp -o ./test/out;
./test/out;

ruby ./test/out.cpp;


echo "===RUNNING TESTS FOR C++ & BASH===";
./polyglot ./test/test.cpp ./test/test.sh -o ./test/out.cpp;

g++ ./test/out.cpp -o ./test/out;
./test/out;

bash ./test/out.cpp;


echo "===RUNNING TESTS FOR BASH & C++===";
./polyglot ./test/test.sh ./test/test.cpp -o ./test/out.cpp;

g++ ./test/out.cpp -o ./test/out;
./test/out;

bash ./test/out.cpp;