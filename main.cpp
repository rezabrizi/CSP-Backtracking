#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <unordered_map>


struct Constraint {
    char var1;
    char var2;
    char op;
};


class CSP {
public:

    void assign_variable (char variable, int value)
    {

    }

    void un_assign_variable (char variable)
    {

    }

    bool IsSolution ()
    {

    }

    void PrintAssignment()
    {

    }


    bool IsConsistent(char variable, int value)
    {

    }


    char SelectVariable ()
    {

    }

    std::vector<int> SelectValues(char variable)
    {

    }

    void RecoverDomain(std::unordered_map<char, std::vector<int>> oldDomain)
    {
        variables = oldDomain;
    }

    std::string GetMode()
    {
        return mode;
    }





private:
    std::unordered_map<char, std::vector<int>> variables;
    std::unordered_map<char, int> assignment;
    std::vector<Constraint> constraints;
    std::string mode;
};


std::unordered_map<char, std::vector<int>> GetVariables (const std::string& var_file_path)
{
    std::unordered_map<char,std::vector<int>> variables;
    std::ifstream file(var_file_path);
    std::string line;

    while (std::getline(file, line))
    {
        std::istringstream input_stream(line);
        char var, colon;
        int value;
        std::vector<int> domain;

        input_stream >> var >> colon;

        while (input_stream >> value)
        {
            domain.push_back(value);
        }

        variables[var] = domain;
    }
    return variables;
}


std::vector<Constraint> GetConstraints (const std::string& const_file_path)
{
    std::vector<Constraint> constraints;

    std::ifstream file(const_file_path);
    std::string line;

    while (std::getline(file, line))
    {
        std::istringstream input_stream(line);
        Constraint c{};
        input_stream >> c.var1 >> c.op >> c.var2;
        constraints.push_back(c);
    }

    return constraints;
}



std::unordered_map<char, std::vector<int>> ForwardChecking (CSP& csp, char variable, int value)
{

}






bool RecursiveBacktrackSearch(CSP& csp) {
    if (csp.IsSolution()) {
        return true; // Found a solution
    }

    char variable = csp.SelectVariable();
    for (int value : csp.SelectValues(variable)) {
        if (csp.IsConsistent(variable, value)) {
            csp.assign_variable(variable, value);

            std::unordered_map<char, std::vector<int>> oldDomain;
            bool proceed = true;

            if (csp.GetMode() == "FC") {
                oldDomain = ForwardChecking(csp, variable, value); // Assuming ForwardChecking modifies the CSP state and returns old domains for restoration
                proceed = !oldDomain.empty(); // Assuming ForwardChecking returns an empty map if FC leads to a dead end
            }

            if (proceed && RecursiveBacktrackSearch(csp)) {
                return true;
            }

            csp.un_assign_variable(variable); // Undo the assignment

            if (csp.GetMode() == "FC") {
                csp.RecoverDomain(oldDomain);
            }
        }
    }

    return false;
}



void BacktrackSearch (CSP& csp)
{
    RecursiveBacktrackSearch(csp);
}




int main(int argc, char *argv[]) {
    // Check for at least 3 arguments (excluding the program name)
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <path_to_var_file> <path_to_con_file> <none|fc>" << std::endl;
        return 1;
    }

    // Assigning the arguments to variables for easier access
    std::string path_to_var_file = argv[1];
    std::string path_to_con_file = argv[2];
    std::string mode = argv[3];

    // Optional: Add logic to validate the mode (none|fc)
    if (mode != "none" && mode != "fc") {
        std::cerr << "Invalid mode. Use 'none' or 'fc'." << std::endl;
        return 1;
    }

    std::unordered_map<char, std::vector<int>> vars_and_domains = GetVariables(path_to_var_file);
    std::vector<Constraint> constraints = GetConstraints(path_to_con_file);


    CSP csp {vars_and_domains, constraints, mode};
    BacktrackSearch (csp);

    return 0;
}
