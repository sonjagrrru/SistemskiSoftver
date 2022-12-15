#include <iostream>
#include <string>
#include "../inc/Emulator.hpp"

using namespace std;

int main(int argc, char *argv[])
{
    string inputFileName;
    if (argc == 2) 
    {
        inputFileName = argv[1];
    }
    else // Invalid arguments
    {
        cerr << "Error: invalid arguments! " << endl;
        return -1;
    }

    // EMULATOR CALLED
    Emulator emulator(inputFileName);
    emulator.Emulate();
}

/**
 * {
	"version": "2.0.0",
	"tasks": [
		{
			"type": "cppbuild",
			"label": "Asembler",
			"command": "/usr/bin/g++",
			"args": [
				"-fdiagnostics-color=always",
				"-g",
				"-o",
				"${workspaceFolder}/Test/assembler",
				"${workspaceFolder}/Asembler/src/*.cpp"
			],
			"options": {
				"cwd": "${workspaceFolder}"
			},
			"problemMatcher": [
				"$gcc"
			],
			"group": {
				"kind": "build",
				"isDefault": true
			},
			"detail": "compiler: /usr/bin/g++"
		},
		{
			"type": "cppbuild",
			"label": "Linker",
			"command": "/usr/bin/g++",
			"args": [
				"-fdiagnostics-color=always",
				"-g",
				"-o",
				"${workspaceFolder}/Test/linker",
				"${workspaceFolder}/Linker/src/*.cpp"
			],
			"options": {
				"cwd": "${workspaceFolder}"
			},
			"problemMatcher": [
				"$gcc"
			],
			"group": {
				"kind": "build",
				"isDefault": true
			},
			"detail": "compiler: /usr/bin/g++"
		},
		{
			"type": "cppbuild",
			"label": "Emulator",
			"command": "/usr/bin/g++",
			"args": [
				"-fdiagnostics-color=always",
				"-g",
				"-o",
				"${workspaceFolder}/Test/emulator",
				"${workspaceFolder}/Emulator/src/*.cpp"
			],
			"options": {
				"cwd": "${workspaceFolder}"
			},
			"problemMatcher": [
				"$gcc"
			],
			"group": {
				"kind": "build",
				"isDefault": true
			},
			"detail": "compiler: /usr/bin/g++"
		}
	]
}
 * 
 */