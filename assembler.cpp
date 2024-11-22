
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
using namespace std;

vector<string> lines;               // stroing all lines
vector<pair<int, string>> comments; // all comments
vector<pair<int, string>> errors;   // all errors
vector<pair<int, string>> warn;     // sotres warnings and line num

map<string, int> unused_text_label;
map<string, int> labels;               // line where the label is used
map<string, pair<int, int>> label_add; // label name, pc, line_num
map<string, int> opcode_1;             // mnemonic and opcode
map<string, int> opcode_br;
map<string, int> opcode_0;
map<pair<int, int>, pair<string, string>> parsed_line; // line_num, address,  operand, mnemonic
bool halt_counter;                                     // checks if the programme has halted
map<string, pair<string, string>> line_with_mac;       // pc, machine_code, line
bool Haltf = false;
void globe()
{
    opcode_1 = {// opcode for i
                {"data", 20},
                {"ldc", 0},
                {"adc", 1},
                {"ldl", 2},
                {"stl", 3},
                {"ldnl", 4},
                {"stnl", 5},
                {"adj", 10},
                {"SET", 21}};

    opcode_br = {// opcode for j
                 {"call", 13},
                 {"brz", 15},
                 {"brlz", 16},
                 {"br", 17}};

    opcode_0 = {// opcode for r
                {"add", 6},
                {"sub", 7},
                {"shl", 8},
                {"shr", 9},
                {"a2sp", 11},
                {"sp2a", 12},
                {"return", 14},
                {"HALT", 18}};
}

bool isNum(string s) // checking if the char is number or not
{
    for (auto &c : s)
    {
        if (c >= '0' && c <= '9')
            return true;
    }
    return false;
}

bool isAlpha(char c) // checking if the char is an alphabet
{
    if (c >= 'A' && c <= 'Z')
        return true;
    if (c >= 'a' && c <= 'z')
        return true;
    return false;
}

string dec_to_hex(int num) // some important conversions
{
    unsigned int numb = num;
    string out = "";
    for (int i = 0; i < 8; ++i, numb /= 16)
    {
        int rem = numb % 16;
        if (rem < 10)
            out += rem + '0';
        else
            out += rem + 'A' - 10;
    }

    reverse(out.begin(), out.end());

    return out;
}

int hex_to_dec(const string &hexStr)
{
    if (hexStr.empty())
    {
        throw invalid_argument("Empty string is not a valid hexadecimal number.");
    }

    size_t start = 0;
    int decimalValue = 0;

    if (hexStr.size() > 2 && hexStr[0] == '0' && (hexStr[1] == 'x' || hexStr[1] == 'X'))
    {
        start = 2;
    }

    for (size_t i = start; i < hexStr.size(); ++i)
    {
        char hexChar = hexStr[i];
        int digitValue;

        if (hexChar >= '0' && hexChar <= '9')
        {
            digitValue = hexChar - '0';
        }
        else if (hexChar >= 'A' && hexChar <= 'F')
        {
            digitValue = hexChar - 'A' + 10;
        }
        else if (hexChar >= 'a' && hexChar <= 'f')
        {
            digitValue = hexChar - 'a' + 10;
        }
        else
        {
            throw invalid_argument("Invalid character in hexadecimal number.");
        }

        // Accumulate the decimal value
        decimalValue = decimalValue * 16 + digitValue;
    }

    return decimalValue;
}

string dec_to_octal(int n)
{
    vector<char> s;
    string out;

    if (n == 0)
    {
        return "0";
    }

    while (n != 0)
    {
        int rem = n % 8;
        s.emplace_back(rem + '0');
        n /= 8;
    }

    reverse(s.begin(), s.end());

    for (int i = 0; i < s.size(); i++)
    {
        out.push_back(s[i]);
    }

    return out;
}

void comments_separate(string Line, int line_num) // reading each line to separate out comments
{

    if (Line.empty())
        return;

    vector<string> out;
    string curr; // current word
    bool found = false;
    string com; // string to store current comment

    int letter = 0;
    for (; letter < Line.size(); letter++)
    {
        if (Line[letter] != ' ')
            break;
    }
    Line.erase(0, letter);

    for (int i = 0; i < (int)Line.size(); ++i)
    {
        if (Line[i] == ';')
        { // Check for the start of a comment
            found = true;
            continue;
        }
        if (found)
        { // If we are in a comment, continue collecting comments
            com += Line[i];
            continue;
        }

        if (isspace(Line[i]))
        { // Handle space
            if (!curr.empty())
            {
                out.push_back(curr);
                curr.clear();
            }
            continue;
        }

        if (Line[i] == ':')
        { // Handle colon (':')
            curr += Line[i];
            out.push_back(curr);
            curr.clear();
            continue;
        }

        if (Line[i] == ';')
        { // Handle semicolon (';') at the end of the word
            if (!curr.empty())
            {
                out.push_back(curr);
                curr.clear();
            }
            break;
        }

        curr += Line[i]; // Accumulate the current character to the current word
    }

    if (!curr.empty())
    { // If there's any remaining word after the loop, add it
        out.push_back(curr);
    }

    if (found)
    {
        if (com.size() > 1)
        { // If a comment was detected during the loop, store it
            com = com.substr(1);
        }
    }
    if (!com.empty())
        comments.emplace_back((int)line_num, com);

    string output;
    for (auto &word : out)
    {
        output.append(word + " ");
    }

    lines.emplace_back(output); // line without comments
    return;
}

bool valid_label(string s, int line_num) // function to verify the validity of the label
{
    if (!isAlpha(s[0]))
    {
        // errors.emplace_back((int)line_num, "Label name is not correct.");
        return false;
    }

    for (int i = 1; i < s.size(); i++)
    {
        if (!isAlpha(s[i]) && !isdigit(s[i]) && s[i] != '_')
        {
            // errors.emplace_back((int)line_num, "Label name is not correct.");
            return false;
        }
    }
    return true;
}

bool data_label(string line, int line_num) // to separate data and text labels
{
    int i;
    for (i = 0;; i++)
    {
        if (line[i] == ':')
            break;
    }

    if (i == line.size() - 1)
        return false;

    string word = "";

    for (i = i + 1; i < line.size(); i++)
    {
        word.push_back(line[i]);
        if (isspace(i))
        {
            break;
        }
    }

    if (opcode_1[word] == 20)
        return true;

    return false;
}

void extract_label(string line, int pc, int line_num)
{
    int letter = 0;
    string label;
    for (; letter < line.size(); letter++)
    {
        if (line[letter] == ':')
        {
            label = line.substr(0, letter);

            break;
        }
    }
    if (label.size() == 0)
        return;

    if (!valid_label(label, line_num))
        ;

    else
    {
        if (label_add.count(label) && label_add[label].first != -1)
        {
            errors.emplace_back((int)line_num, "Repeated Label Definition");
        }
        else
        {
            label_add.insert({label, {pc, line_num}});
            labels[label] = line_num;
        }
    }
}

map<string, string> var_set; // to store SET variables

bool valid_file_name(string file_name)
{
    auto it = file_name.end() - 1;
    if (*it-- != 'm' || *it-- != 's' || *it-- != 'a' || *it-- != '.')
    {
        cout << "Error: incorrect file format";
        return false;
    }
    return true;
}

bool extract_mnemonic(string mnemonic, int line_num, string operand, int pc)
{
    int opnum; // Opcode type

    if (opcode_0.count(mnemonic))
    {
        opnum = 0; // Zero-operand mnemonic
    }
    else if (opcode_1.count(mnemonic))
    {
        opnum = 1; // One-operand mnemonic
    }
    else if (opcode_br.count(mnemonic))
    {
        opnum = 2; // Branch mnemonic (usually requiring a label)
    }
    else
    {
        errors.emplace_back(line_num, "Invalid mnemonic found: " + mnemonic);
        return false;
    }
    if (mnemonic == "HALT")
        Haltf = true;
    // Check operand presence and format based on mnemonic type
    if (opnum == 0)
    {
        if (!operand.empty())
        {
            errors.emplace_back(line_num, "Unexpected operand for zero-operand mnemonic");
            return false;
        }
    }
    else if (opnum == 1)
    {
        if (operand.empty())
        {
            errors.emplace_back(line_num, "Missing operand for one-operand mnemonic");
            return false;
        }
        else if (operand.find(',') != string::npos)
        { // Detect multiple operands
            errors.emplace_back(line_num, "Extra operand provided");
            return false;
        }
    }
    else if (opnum == 2)
    {
        if (operand.empty())
        {
            errors.emplace_back(line_num, "Missing label for branch mnemonic");
            return false;
        }
        else if (!label_add.count(operand))
        {
            // Add placeholder for unresolved label to be checked in the second pass
            label_add[operand] = {-1, line_num};
        }
    }

    return true;
}

void first_pass(vector<string> lines)
{
    int pc = 0;
    int line_num = 0;
    for (auto &line : lines)
    {
        line_num++;
        int letter = 0;
        string label, mnemonic, operand;

        // Check if the line starts with a label
        for (; letter < line.size(); letter++)
        {
            if (line[letter] == ':')
            {
                label = line.substr(0, letter);

                break;
            }
        }
        if (!label.empty())
        {
            letter += 1; // Move past ': '

            if (valid_label(label, line_num))
            {
                if (label_add.count(label))
                {
                    errors.emplace_back(line_num, "Duplicate label definition");
                }
                else
                {
                    label_add[label] = {pc, line_num};
                }
            }

            if (!valid_label(label, line_num))
            {
                errors.emplace_back(line_num, "Invalid Label");
                continue;
            }
        }
        else
            letter = 0;

        while (line[letter] == ' ')
        {
            letter++;
        }
        int begin = letter;

        // Parse mnemonic if available
        while (letter < line.size() && !isspace(line[letter]))
        {
            letter++;
        }

        mnemonic = line.substr(begin, letter - begin);

        string unused_label;
        // Log mnemonic or indicate missing mnemonic
        if (mnemonic.empty() && !label.empty())
        {
            unused_label = label;
        }

        if (!unused_label.empty())
        {
            if (label.empty() && !mnemonic.empty())
                ;
            else if (!label.empty() && !mnemonic.empty())
            {
                warn.emplace_back(line_num - 1, "Unused label: " + label);
            }
        }

        // Skip whitespace after mnemonic
        while (letter < line.size() && isspace(line[letter]))
        {
            letter++;
        }

        // Parse operand if available
        while (letter < line.size())
        {
            operand += line[letter++];
        }

        // Check if mnemonic is valid
        if (!mnemonic.empty() && !extract_mnemonic(mnemonic, line_num, operand, pc))
        {
            // Invalid mnemonic or operand, already reported in extract_mnemonic
            // errors.emplace_back(line_num, "Bogus mnemonic");
            continue;
        }

        if (mnemonic != "data" && !label.empty())
            unused_text_label[label] = line_num;

        // Handle SET mnemonic separately due to specific requirements
        if (mnemonic == "SET")
        {
            if (label.empty())
            {
                errors.emplace_back(line_num, "Missing variable name for SET");
            }
            if (!isNum(operand))
            {
                errors.emplace_back(line_num, "Invalid operand for SET");
            }
            var_set[label] = operand;
        }
        else if (!mnemonic.empty())
        {
            // Increment program counter for each valid instruction
            pc++;
        }
        parsed_line.insert({{line_num, pc}, {operand, mnemonic}});
    }
}

void AddToList(string label, string mnemonic, string operand, string machine, int pc)
{
    if (!mnemonic.empty())
        mnemonic += " : ";
    if (!label.empty())
        label += " : ";
    string stat = label + mnemonic + operand; // to store the complete statement
    line_with_mac.insert({dec_to_hex(pc), {machine, stat}});
}

int op_type(string mnemonic)
{
    if (opcode_0.count(mnemonic))
        return 0;
    if (opcode_1.count(mnemonic))
        return 1;
    if (opcode_br.count(mnemonic))
        return 2;
    return -1;
}

bool isHex(string operand)
{
    if (operand.empty())
        return false;

    size_t start = 0;

    if (operand.size() > 2 && operand[0] == '0' && (operand[1] == 'x' || operand[1] == 'X'))
    {
        start = 2;
    }
    else
        return false;

    if (start == operand.size())
        return false;

    for (size_t i = start; i < operand.size(); ++i)
    {
        if (!isxdigit(operand[i]))
        {
            return false;
        }
    }

    return true;
}

bool isDec(string operand)
{
    if (operand.empty())
        return false;

    size_t i = 0;

    if (operand[i] == '+' || operand[i] == '-')
    {
        i++;
        // If only a sign is present, return false
        if (i == operand.size())
            return false;
    }

    for (; i < operand.size(); ++i)
    {
        if (!isdigit(operand[i]))
        {
            return false;
        }
    }

    return true;
}

vector<string> MCodes;

void second_pass(map<pair<int, int>, pair<string, string>> parsed_line)
{
    for (auto &curr : parsed_line)
    {
        int pc = curr.first.first;
        int line_num = curr.first.second;
        string operand = curr.second.first;
        string mnemonic = curr.second.second;
        string label = "";

        // errors.emplace_back(line_num, operand+"," +mnemonic);
        if (!operand.empty())
        {
            if (operand[operand.size() - 1] == ' ')
                operand.pop_back();
        }

        //   errors.emplace_back(line_num, operand);

        if (!operand.empty() && valid_label(operand, line_num))
        {
            unused_text_label[operand] = 0; // Mark label as used
            if (!label_add.count(operand))
            {
                label_add[operand] = {-1, line_num};
                errors.emplace_back(line_num + 4, "Label: " + operand + " used but not defined");
            }
        }

        // Process numeric operand
        else if (operand[0] == '-' || operand[0] == '+')
        {
            string now = operand, sign = "";

            sign = now[0];
            now = now.substr(1);
            operand = "";

            operand += sign;

            if (isHex(now))
            {
                operand += hex_to_dec(now);
            }
            else if (isDec(now))
            {
                operand += now;
            }
        }
        // errors.emplace_back(line_num, operand);
        // Check if there's a label associated with the current line
        for (auto &it : label_add)
        {
            if (it.second == make_pair(pc, line_num))
            {
                label = it.first;
                break;
            }
        }

        int op_typ;
        if (!mnemonic.empty())
            op_typ = op_type(mnemonic);

        if (op_typ == -1)
            continue;

        string mac_code = "";
        if (mnemonic == "HALT")
            halt_counter = true;

        if (op_typ == 0)
        {
            int op_code = opcode_0[mnemonic];
            mac_code = "000000" + dec_to_hex(op_code).substr(6);
        }

        if (op_typ == 1)
        {
            if (mnemonic == "data" || mnemonic == "SET")
            {
                mac_code = dec_to_hex(stoi(operand));
            }

            else
            {
                int value = 0;
                if (label_add.count(operand))
                {
                    value = label_add[operand].first;
                }
                else if (isNum(operand))
                {
                    value = stoi(operand);
                }
                mac_code = dec_to_hex(value).substr(2) + dec_to_hex(opcode_1[mnemonic]).substr(6);
            }
        }
        else if (op_typ == 2)
        {
            // offset is required
            int offset;
            if (label_add.count(operand))
            {
                offset = label_add[operand].first - (pc + 1);
            }
            else
            {
                /* string temp = "";
                 for (auto &letter : operand)
                 {
                     if (isdigit(letter))
                     {
                         temp += letter;
                     }
                 }*/
                if (!operand.empty())
                    offset = stoi(operand);
            }
            //      (int)stoi(operand);

            mac_code = dec_to_hex(offset).substr(2) + dec_to_hex(opcode_br[mnemonic]).substr(6);
        }

        MCodes.emplace_back(mac_code);
        // Add line to the list for final output
        AddToList(label, mnemonic, operand, mac_code, pc);
    }
}

void generate_file(string file_name)
{
    ofstream file;
    file.open(file_name);
    if (!file.is_open())
    {
        cout << "Error in creating " << file_name << "!" << endl;
        return;
    }
    cout << file_name << " created successfully." << endl;
}

void issue_warn(ofstream &log_file)
{
    log_file << "Warnings:\n";
    for (auto &warning : warn)
    {
        log_file << "Warning: " << warning.second << " in line " << warning.first << "\n";
    }
    log_file << "\n";
}

void throw_error(ofstream &log_file)
{
    log_file << "Errors:\n";
    for (auto &error : errors)
    {
        log_file << "Error: " << error.second << " in line " << error.first << "\n";
    }
    log_file << "\n";
    log_file.close();
}

void generate_list_file(const string &list_file_name)
{
    ofstream list_file(list_file_name);
    if (!list_file.is_open())
    {
        cout << "Error in creating " << list_file_name << "!" << endl;
        return;
    }
    if (!errors.empty())
    {
        list_file << "Errors detected";
        list_file.close();
        return;
    }
    for (const auto &entry : line_with_mac)
    {
        list_file << "PC: " << entry.first << " | Machine Code: " << entry.second.first << " | Statement: " << entry.second.second << "\n";
    }
    list_file.close();
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cout << "Error: string not recognized.";
        return 1;
    }

    string file_name = argv[1];
    ifstream file(file_name);

    if (!file.is_open())
    {
        cout << "Error: Could not open file " << file_name << endl;
        return 1;
    }

    string lin;
    int i = 1;
    while (getline(file, lin))
    {
        comments_separate(lin, i);
        i++;
    }

    file.close();
    // Call the necessary functions
    globe();
    first_pass(lines);
    second_pass(parsed_line);
    for (auto it : unused_text_label)
    {
        if (it.second >= 1)
        {
            warn.emplace_back(it.second, "Unused label");
        }
    }
    if (!Haltf)
        warn.emplace_back(0, "Halt flag not found");

    file_name.erase(file_name.size() - 4, file_name.size() - 1);

    // Generate the log file
    ofstream log_file(file_name + ".log");
    if (!log_file.is_open())
    {
        cout << "Error in creating log file!" << endl;
        return 1;
    }

    issue_warn(log_file);
    throw_error(log_file);

    // Generate the list file
    generate_list_file(file_name + ".lst");

    ofstream coutMCode;
    coutMCode.open(file_name + ".o", ios::binary | ios::out);
    for (auto code : MCodes)
    {
        if (code.empty() || code == "        ")
            continue;
        unsigned int cur = (unsigned int)hex_to_dec(code);
        static_cast<int>(cur);
        coutMCode.write((const char *)&cur, sizeof(unsigned int));
    }
    coutMCode.close();
    cout << "Machine code object(.o) file generated" << endl;

    return 0;
}