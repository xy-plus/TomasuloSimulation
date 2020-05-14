D=""

if [ $# != 2 ]
then
    echo "usage: ./run.sh [file_name] [print_state](1 or 0)"
else
    if [ $2 == 1 ]
    then
        D="-DPRINT_STATE"
    fi
    g++ src/main.cpp -o tmsl.out -std=c++11 $D
    (./tmsl.out $1) > out.md
fi
