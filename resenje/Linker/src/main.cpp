#include <iostream>
#include <string>
#include "../inc/Linker.hpp"

using namespace std;

int main(int argc, char *argv[])
{
    list<string> inputFileNames = list<string>();
    string outputFileName;
    if (argc >= 5 && strcmp(argv[1], "-hex") == 0 && strcmp(argv[2], "-o")==0) 
    {
        outputFileName = argv[3];
        for (int i = 4; i < argc; i++)
        {
            inputFileNames.push_back(argv[i]);
        }
    }
    else // Invalid arguments
    {
        cerr << "Error: invalid arguments! " << endl;
        return -1;
    }

    // OPENING INPUT AND OUTPUT FILES
    ofstream outputFile(outputFileName);
    if (!outputFile.is_open())
    {
        cerr << "Error: unable to open output file!" << endl;
        return -1;
    }
    // LINKER CALLED
    Linker linker(&outputFile, inputFileNames);
    linker.Pass();

    fflush(stdout);
}
