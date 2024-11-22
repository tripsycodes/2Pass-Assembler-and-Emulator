
/*********************Declaration of Authorship**********************************/
/////////////////////////NAME : Tripti Jain
////////////////////////ROll NO. 2301CS60

#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <utility>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
using namespace std;

vector<int> mac_code;            // Holds machine code instructions
int memory[1 << 24];             // Memory array for storage
int A, B, PC, SP, lineCount = 0; // Registers and line counter
pair<int, int> readWriteLog;
map<string, string> opcodeImmediate; // Immediate instruction opcodes
map<string, string> opcodeBranch;    // Branch instruction opcodes
map<string, string> opcodeNoOperand; // No operand opcodes
map<string, pair<int, string>> operationTable;

// Arithmetic and logical operations based on ISA requirements
void loadConstant(int value)
{
    B = A;
    A = value;
}

void addConstant(int value)
{
    A += value;
}

void loadMemory(int offset)
{
    B = A;
    A = memory[SP + offset];
    readWriteLog = {SP + offset, 0};
}

void storeMemory(int offset)
{
    readWriteLog = {SP + offset, memory[SP + offset]};
    memory[SP + offset] = A;
    A = B;
}

void loadNonLocal(int offset)
{
    A = memory[A + offset];
    readWriteLog = {SP + offset, 0};
}

void storeNonLocal(int offset)
{
    readWriteLog = {SP + offset, memory[SP + offset]};
    memory[A + offset] = B;
}

void addRegisters(int value)
{
    A += B;
}

void subtractRegisters(int value)
{
    A = B - A;
}

void shiftLeft(int value)
{
    A = B << A;
}

void shiftRight(int value)
{
    A = B >> A;
}

void adjustStack(int value)
{
    SP += value;
}

void moveAtoSP(int value)
{
    SP = A;
    A = B;
}

void moveSPtoA(int value)
{
    B = A;
    A = SP;
}

void callFunction(int offset)
{
    B = A;
    A = PC;
    PC += offset;
}

void returnFunction(int value)
{
    PC = A;
    A = B;
}

void branchIfZero(int offset)
{
    if (A == 0)
    {
        PC += offset;
    }
}

void branchIfLessZero(int offset)
{
    if (A < 0)
    {
        PC += offset;
    }
}

void branchUnconditional(int offset)
{
    PC += offset;
}

void haltExecution(int value)
{
    // Halt the program
    return;
}

// Mnemonics and function pointers for operations
vector<string> mnemonicList = {"ldc", "adc", "ldl", "stl", "ldnl", "stnl", "add", "sub", "shl", "shr", "adj", "a2sp", "sp2a", "call", "return", "brz", "brlz", "br", "HALT"};
void (*functionList[])(int value) = {loadConstant, addConstant, loadMemory, storeMemory, loadNonLocal, storeNonLocal, addRegisters, subtractRegisters, shiftLeft, shiftRight, adjustStack, moveAtoSP, moveSPtoA, callFunction, returnFunction, branchIfZero, branchIfLessZero, branchUnconditional, haltExecution};

// Opcode mapping for various instruction types
void initializeOpcodes()
{
    opcodeImmediate = {{"data", "14"}, {"ldc", "00"}, {"adc", "01"}, {"ldl", "02"}, {"stl", "03"}, {"ldnl", "04"}, {"stnl", "05"}, {"SET", "15"}};
    opcodeBranch = {{"call", "0D"}, {"brz", "0F"}, {"brlz", "10"}, {"br", "11"}};
    opcodeNoOperand = {{"add", "06"}, {"sub", "07"}, {"shl", "08"}, {"shr", "09"}, {"a2sp", "0B"}, {"sp2a", "0C"}, {"return", "0E"}, {"HALT", "12"}};
}

void populateOperationTable()
{
    for (const auto &entry : opcodeImmediate)
        operationTable[entry.first] = {1, entry.second}; // Immediate instructions with operands
    for (auto &entry : opcodeBranch)
        operationTable[entry.first] = {2, entry.second}; // Branch instructions with offset
    for (auto entry : opcodeNoOperand)
        operationTable[entry.first] = {0, entry.second}; // No-operand instructions
}

// Convert decimal to 8-bit hexadecimal string
string toHexString(int num)
{
    stringstream ss;
    ss << hex << setw(8) << setfill('0') << num;
    return ss.str();
}

// Error handling function
void displayError()
{
    cout << "Runtime Error: Possible Segmentation Fault or Infinite Loop" << endl;
    cout << "Please check your program code" << endl;
    exit(0);
}

// Display register states
void outputTrace(ofstream &outfile)
{
    outfile << "PC = " << setfill('0') << setw(8) << hex << PC << ", ";
    outfile << "SP = " << setfill('0') << setw(8) << hex << SP << ", ";
    outfile << "A = " << setfill('0') << setw(8) << hex << A << ", ";
    outfile << "B = " << setfill('0') << setw(8) << hex << B << endl;
    printf("PC = %08X, SP = %08X, A = %08X, B = %08X,\n", PC, SP, A, B);
}

// Memory read and write logging functions
void logMemoryRead()
{
    cout << "Reading memory[" << toHexString(readWriteLog.first) << "] finds " << toHexString(A) << endl;
}

void logMemoryWrite()
{
    cout << "Writing memory[" << toHexString(readWriteLog.first) << "] was " << toHexString(readWriteLog.second) << " now " << toHexString(memory[readWriteLog.first]) << endl;
}

// Memory dump functions
void memoryDumpBefore()
{
    cout << "Memory state before execution:" << endl;
    for (int i = 0; i < (int)mac_code.size(); i += 4)
    {
        cout << toHexString(i) << " ";
        for (int j = i; j < min((int)mac_code.size(), i + 4); ++j)
        {
            cout << toHexString(mac_code[j]) << " ";
        }
        cout << endl;
    }
}

void memoryDumpAfter()
{
    cout << "Memory state after execution:" << endl;
    for (int i = 0; i < (int)mac_code.size(); i += 4)
    {
        cout << toHexString(i) << " ";
        for (int j = i; j < min((int)mac_code.size(), i + 4); ++j)
        {
            cout << toHexString(memory[j]) << " ";
        }
        cout << endl;
    }
}

void resetRegisters()
{
    A = B = SP = PC = 0;
}

// Display ISA
void displayISA()
{
    cout << "Opcode Mnemonic Operand" << '\n';
    cout << "0      ldc      value" << "\n";
    cout << "1      adc      value" << "\n";
    cout << "2      ldl      value" << "\n";
    cout << "3      stl      value" << "\n";
    cout << "4      ldnl     value" << "\n";
    cout << "5      stnl     value" << "\n";
    cout << "6      add" << "\n";
    cout << "7      sub" << "\n";
    cout << "9      shr" << "\n";
    cout << "10     adj      value" << "\n";
    cout << "11     a2sp" << "\n";
    cout << "12     sp2a" << "\n";
    cout << "13     call     offset" << "\n";
    cout << "14     return" << "\n";
    cout << "15     brz      offset" << "\n";
    cout << "16     brlz     offset" << "\n";
    cout << "17     br       offset" << "\n";
    cout << "18     HALT" << "\n";
    cout << "       SET      value" << "\n";
}

bool executeLine(int currentLine, int flag, ofstream &outfile)
{
    int opcode = (currentLine & 0xFF);
    int operand = (currentLine - opcode) >> 8;
    lineCount++;
    (functionList[opcode])(operand);
    if ((PC < 0) || (lineCount > 100000))
    {
        displayError();
        return true;
    }
    if (flag == 0)
    {
        outputTrace(outfile);
        cout << mnemonicList[opcode] << " ";
        if (operationTable.count(mnemonicList[opcode]))
            cout << toHexString(operand);
        cout << endl;
    }
    return false;
}

// Instruction set

// call for each line
void Run(int flag, ofstream &outfile)
{
    while (PC < (int)mac_code.size())
    {
        int currentLine = mac_code[PC++];
        bool toQuit = executeLine(currentLine, flag, outfile);
        if (toQuit)
            break;
    }
}

// checking the command choosen
void executeCommand(string command, ofstream &outfile)
{
    if (command == "-trace")
    {
        Run(0, outfile);
        cout << "Program execution finished!" << endl
             << "Trace File Generated" << endl;
    }
    else if (command == "-read")
        Run(1, outfile);
    else if (command == "-write")
        Run(2, outfile);
    else if (command == "-before")
    {
        Run(3, outfile);
        memoryDumpBefore();
    }
    else if (command == "-after")
    {
        Run(3, outfile);
        memoryDumpAfter();
    }
    else if (command == "-wipe")
        resetRegisters();
    else if (command == "-isa")
        displayISA();
    else
    {
        cout << "Invalid command" << endl;
        exit(0);
    }
    cout << lineCount << " instructions executed" << endl;
}
string extractFilenameWithoutExtension(const std::string &filename)
{
    // Find the position of the last '.' character
    size_t dotPos = filename.find_last_of(".");
    if (dotPos == std::string::npos)
    {
        // If no dot is found, return the entire string
        return filename;
    }
    else
    {
        // Return the substring before the last '.'
        return filename.substr(0, dotPos);
    }
}
// reading input from .o file
void takeInput(string filename)
{
    ifstream file(filename, ios::in | ios::binary);
    unsigned int cur;
    int counter = 0;
    while (file.read((char *)&cur, sizeof(int)))
    {
        mac_code.push_back(cur);
        memory[counter++] = cur;
    }
}

// calling required functions
int main(int argc, char *argv[])
{
    if (argc <= 2)
    {
        cout << "Invalid Format" << endl;
        cout << "Expected format << ./emu [command] filename.o" << endl;
        cout << "Commands are " << endl;
        cout << "-trace  show instruction trace" << endl;
        cout << "-read   show memory reads" << endl;
        cout << "-write  show memory writes" << endl;
        cout << "-before show memory dump before execution" << endl;
        cout << "-after  show memory dump after execution" << endl;
        cout << "-wipe   wipe written flags before execution" << endl;
        cout << "-isa    display ISA" << endl;
        return 0;
    }
    string filename = argv[2];
    string tracefilename = extractFilenameWithoutExtension(filename) + ".trace";
    // open file for writing
    std::ofstream outfile(tracefilename);
    populateOperationTable();
    takeInput(filename);
    string command = argv[1];
    executeCommand(command, outfile);
    return 0;
}