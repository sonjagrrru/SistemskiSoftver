#include <iostream>
#include <string>
#include "../inc/Asembler.hpp"

using namespace std;

int main(int argc, char *argv[])
{
    string inputFileName;
    string outputFileName;

    if (argc == 4 && strcmp(argv[1], "-o") == 0)
    {
        inputFileName = argv[3];
        outputFileName = argv[2];
    }
    else //Invalid arguments
    {
        cerr << "Error: invalid arguments! " << endl;
        return -1;
    }

    //OPENING INPUT AND OUTPUT FILES
    ifstream inputFile(inputFileName);
    ofstream outputFile(outputFileName);
    if (!inputFile.is_open())
    {
        cerr << "Error: unable to open input file!" << endl;
        return -1;
    }
    if (!outputFile.is_open())
    {
        cerr << "Error: unable to open output file!" << endl;
        inputFile.close();
        return -1;
    }
    //ASSEMBLER CALLED
    Asembler assembler(&inputFile, &outputFile);
    assembler.Pass();

    //CLOSING INPUT AND OUTPUT FILES
    inputFile.close();
    outputFile.close();
    fflush(stdout);
}