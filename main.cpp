#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <algorithm>
#include <utility>


/**
 * a hashmap to map an operator with a function to check a constraint
 */
std::unordered_map<char, std::function<bool(int, int)>> opMap = {
        {'=', [](int a, int b) { return a == b; }},
        {'!', [](int a, int b) { return a != b; }},
        {'>', [](int a, int b) { return a > b; }},
        {'<', [](int a, int b) { return a < b; }},
};


/**
 * struct to represent a constraint
 */
struct Constraint {
    char var1;
    char var2;
    char op;
};


class CSP {
public:
    // hashmap to hold assigned variables and their values. If a variable is unassigned it won't exist in this hashmap
    std::unordered_map<char, int> assignment{};
    // holds the domain of an instance of a CSP problem
    std::unordered_map<char, std::vector<int>> domain;
    // holds all the constraints of a csp. This does not change at all during any of the search states
    std::vector<Constraint> constraints;
    // a flag to indicate whether to do forward checking or not
    std::string mode;

    CSP(std::unordered_map<char, std::vector<int>> variables,
        std::vector<Constraint> constraints,
        std::string mode)
            : domain(std::move(variables)), constraints(std::move(constraints)), mode(std::move(mode)) {}


    bool is_complete_assignment()
    {
        /**
         * all variables have been assigned values; solution possible
         */
        return assignment.size() == domain.size();
    }

    bool IsSolution() {
        /**
         * Check if every constraint passes the current assignment of variables
         * This function is only called whenever we have a complete assignment of variables
         */

        for (const auto& constraint: constraints)
        {
            if (!opMap.at(constraint.op)(assignment.at(constraint.var1), assignment.at(constraint.var2)))
            {
                return false;
            }

        }

        return true;
    }


    bool IsConsistent(char variable, int value) const
    {
        /**
         * Check that the variable and value assigned to it passes the constraint it involves.
         * If the variable is on one side of a constraint then that constraint has to be checked
         * against all other assigned variables.
         */
        for (const auto& constraint : constraints) {
            // Check if the current variable is involved in the constraint
            if (constraint.var1 == variable || constraint.var2 == variable) {
                char other_var = constraint.var1 == variable ? constraint.var2 : constraint.var1;

                // Check if the other variable is assigned
                auto it = assignment.find(other_var);
                if (it != assignment.end()) {
                    int other_value = it->second; // Get the assigned value for the other variable

                    // Perform the check based on who is var1 and who is var2 in the constraint
                    if ((constraint.var1 == variable && !opMap.at(constraint.op)(value, other_value)) ||
                        (constraint.var2 == variable && !opMap.at(constraint.op)(other_value, value))) {
                        return false;
                    }
                }
            }
        }
        return true;
    }


    std::vector<char> get_un_assigned_variables () const
    {
        /**
        * get all the unassigned variables
        */
        std::vector<char> un_assigned_vars;
        for (const auto& variable: domain)
        {
            // if a variable is not assigned then take it: not in assignment hashmap and its domain is not empty
            if (assignment.find(variable.first) == assignment.end() && !domain.at(variable.first).empty())
            {
                un_assigned_vars.push_back(variable.first);
            }
        }
        return un_assigned_vars;
    }


    int get_domain_count (char variable) const
    {
        int domain_count = -1;
        if (domain.find(variable) != domain.end())
        {
            domain_count = static_cast<int>(domain.at(variable).size());
        }
        else
        {
            std::cerr << "error - variable doesn't exist in the domain\n";
        }
        return domain_count;
    }


    int get_constraint_count(char variable) const
    {
        int constraint_count = 0;
        for (const auto& constraint: constraints)
        {
            if ((constraint.var1 == variable && assignment.find(constraint.var2) == assignment.end()) ||
                    (constraint.var2 == variable && assignment.find(constraint.var1) == assignment.end()))
            {
                constraint_count += 1;
            }
        }
        return constraint_count;
    }


    char SelectVariable() const
    {

        /**
        * select variables based on most constrained variable and breaking ties with most constraining variable
        * to check most constrained variable check number of available domains for a variable that is un-assigned
        * to check most constraining variable check the number of unassigned variables that have a relationship with the variable
        */

        std::vector<char> un_assigned_variables = get_un_assigned_variables();
        std::vector<char> most_constrained_variables;
        for (const auto& curr_var: un_assigned_variables)
        {
            if (most_constrained_variables.empty())
            {
                most_constrained_variables.push_back(curr_var);
            }
            else
            {
                int curr_var_domain_count = get_domain_count(curr_var);

                char prev_selected_var = most_constrained_variables.back();
                int prev_selected_var_count = get_domain_count(prev_selected_var);

                if(curr_var_domain_count == prev_selected_var_count)
                {
                    most_constrained_variables.push_back(curr_var);
                }
                else if (curr_var_domain_count < prev_selected_var_count)
                {
                    most_constrained_variables.clear();
                    most_constrained_variables.push_back(curr_var);
                }
            }
        }

        if (most_constrained_variables.size() == 1)
        {
            return most_constrained_variables.back();
        }

        char most_constraining_variable = most_constrained_variables[0];
        int max_constraints = get_constraint_count(most_constrained_variables[0]);

        for (int i = 1; i < most_constrained_variables.size(); ++i) {
            int curr_constraints = get_constraint_count(most_constrained_variables[i]);
            if (curr_constraints > max_constraints) {
                max_constraints = curr_constraints;
                most_constraining_variable = most_constrained_variables[i];
            }
            else if (curr_constraints == max_constraints && most_constrained_variables[i] < most_constraining_variable) {
                // Tie-break by lexicographical order
                most_constraining_variable = most_constrained_variables[i];
            }
        }

        return most_constraining_variable;
    }


    std::vector<Constraint> get_constraints(char variable) const
    {
        std::vector<Constraint> involved_constraints;
        for (const auto& constraint: constraints)
        {
            if ((constraint.var1 == variable && assignment.find(constraint.var2) == assignment.end()) ||
                (constraint.var2 == variable && assignment.find(constraint.var1) == assignment.end()))
            {
                involved_constraints.push_back(constraint);
            }
        }
        return involved_constraints;
    }


    std::vector<int> SelectValues(char variable) const
    {
        /**
         * Given a variable check all of the values in its domain and assign a ranking based on the least constraining value
         * if you choose a value from the domain of that variable, how many choices will remain for the rest of the unassigned variables in the variable domain
         */
        std::unordered_map<int, int> choices;
        std::vector<Constraint> involved_constraints = get_constraints(variable);

        for (int curr_value : domain.at(variable)) {
            int constraint_satisfaction_count = 0;
            for (const auto& constraint : involved_constraints) {
                char other_var = (variable == constraint.var1) ? constraint.var2 : constraint.var1;
                for (int other_value : domain.at(other_var)) {
                    if ((variable == constraint.var1 && opMap.at(constraint.op)(curr_value, other_value)) ||
                        (variable == constraint.var2 && opMap.at(constraint.op)(other_value, curr_value))) {
                        constraint_satisfaction_count++;
                    }
                }
            }
            choices[curr_value] = constraint_satisfaction_count;
        }

        std::vector<std::pair<int, int>> sortable(choices.begin(), choices.end());
        std::sort(sortable.begin(), sortable.end(), [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
            return a.second > b.second; // Sorting in ascending order
        });

        std::vector<int> sortedValues;
        for (const auto& pair : sortable) {
            sortedValues.push_back(pair.first);
        }

        return sortedValues;
    }


    std::unordered_map<char, std::vector<int>> ForwardChecking(char variable, int value) {
        /**
         * Given a variable and a value eliminate values from the domain of the unassigned variables that have a constraint with the chosen variable
         * if one of the unassigned variables ends up having 0 values in it's domain then do nothing and return an empty domain
         * if we don't reach a dead end return the old domain and update the current domain to the new restricted domain
         */
        std::unordered_map<char, std::vector<int>> oldDomain = domain;
        std::unordered_map<char, std::vector<int>> newDomain = domain;

        std::vector<Constraint> involved_constraints = get_constraints(variable);
        for (const auto& constraint: involved_constraints) {
            char other_var = (variable == constraint.var1) ? constraint.var2 : constraint.var1;

            std::vector<int> other_var_new_domain;
            for (int other_value: domain.at(other_var)) {
                if ((variable == constraint.var1 && opMap.at(constraint.op)(value, other_value)) ||
                    (variable == constraint.var2 && opMap.at(constraint.op)(other_value, value))) {
                    other_var_new_domain.push_back(other_value);
                }
            }

            if (other_var_new_domain.empty()) {
                return {};
            } else {
                newDomain[other_var] = other_var_new_domain;
            }
        }

        domain = newDomain;
        return oldDomain;
    }


    void restore_domain(std::unordered_map<char, std::vector<int>> oldDomain)
    {
        /**
         * restore the domain for when we backtrack for searches with forward checking
         */
        domain = std::move(oldDomain);
    }


    void assign_variable(char variable, int value) {
        /**
         * assign a variable and update the domain of that variable
         */
         assignment[variable] = value;
         domain[variable].clear();
    }


    void un_assign_variable(char variable) {
        /**
         * un-assign a variable and update the domain of that variable
         */
         assignment.erase(variable);
    }


    void print_domain ()
    {
        /**
         * test function to see if the input was read properly
         */
        for (const auto& variable: domain)
        {
            std::cout << variable.first<< ": ";
            for (const auto& value: variable.second)
            {
                std::cout << std::to_string(value) << " ";
            }
            std::cout << std::endl;
        }
    }


    void print_constraints()
    {
        /**
         * test function to see if the input was read properly
         */
        for (const auto& constraint: constraints)
        {
            std::cout << constraint.var1 << " " << constraint.op << " " << constraint.var2 << std::endl;
        }
    }


    void print_failure(const std::vector<char>& var_ordering, int i)
    {
        /**
         * print a failure with the consistent variable ordering and the correct index of the failure branch
         */
         std::cout << std::to_string(i) << ". ";
         for (int j = 0; j < var_ordering.size(); j++)
         {
             std::cout << var_ordering[j] << "=" << std::to_string(assignment.at(var_ordering[j]));
             if (j == var_ordering.size()-1)
             {
                 std::cout << "  failure\n";
             }
             else
             {
                 std::cout << ", ";
             }
         }
    }


    void print_success(const std::vector<char>& var_ordering, int i)
    {
        /**
         * print a success with the consistent variable ordering and the correct index of the failure branch
         */

        std::cout << std::to_string(i) << ". ";
        for (int j = 0; j < var_ordering.size(); j++)
        {
            std::cout << var_ordering[j] << "=" << std::to_string(assignment.at(var_ordering[j]));
            if (j == var_ordering.size()-1)
            {
                std::cout << "  solution\n";
            }
            else
            {
                std::cout << ", ";
            }
        }
    }

};



bool RecursiveBacktrackSearch(int& i, std::vector<char>& order_vars_assigned, CSP& csp) {
    // Here we check if the assignment is complete
    if (csp.is_complete_assignment() && csp.IsSolution()) {
        // we need to print variables in order
        i++;
        csp.print_success(order_vars_assigned, i);
        std::cout << " solution\n";
        return true;
    }

    // select the next variable from the domain based on un-assigned variables and the current domain
    char variable = csp.SelectVariable();
    order_vars_assigned.push_back(variable);

    // create value selection vector based on the least constraining value heuristic
    std::vector<int> values_least_cnst_hstc = csp.SelectValues(variable);

    for (int value : values_least_cnst_hstc) {
        // a variable and a value was available in the domain

        // does the new variable=value assignment pass all the constraints...?
        if (csp.IsConsistent(variable, value)) {
            // ... YES it does so let's continue searching

            // if we are performing forward checking then see if we reach a dead end or not
            // the forward checking function should only update the domain if it does not lead to a dead end
            // if it returns a non-empty oldDomain it means we can continue with our search...POGGERS
            // if it returns an empty oldDomain we cannot continue this search ... :((
            std::unordered_map<char, std::vector<int>> old_domain;

            if (csp.mode == "fc") {
                old_domain = csp.ForwardChecking(variable, value);
                if (old_domain.empty())
                {
                    // ... we can't continue the search
                    // un-assign the value from the variable and move on
                    // the domain has remained the same
                    continue;
                }
            }

            // assign the variable=value in our current assignment
            csp.assign_variable(variable, value);

            // at this point we know for sure we can have one more branch in the search tree
            // so increment the i
            i++;
            if (RecursiveBacktrackSearch(i, order_vars_assigned, csp)) {
                return true;
            }

            if (csp.mode == "fc")
            {
                // restore the domain first
                csp.restore_domain(old_domain);
            }
            // un-assign the variable
            csp.un_assign_variable(variable);
        }
        // we can print assignment here + the value that was just chosen
        // we need to print variables in order
        i++;
        csp.print_failure(order_vars_assigned, i);
        std::cout <<"  failure\n";
    }
    // at this point we reached a failure. We can print it
    i++;
    return false;
}


void BacktrackSearch(CSP& csp) {
    std::vector<char> order_vars_assigned;
    int i = 0;
    RecursiveBacktrackSearch(i, order_vars_assigned, csp);
}

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


int main(int argc, char *argv[]) {

    // Check for at least 3 arguments (excluding the program name)
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <path_to_var_file> <path_to_con_file> <none|fc>" << std::endl;
        return 1;
    }

    // Assigning the arguments to domain for easier access
    std::string path_to_var_file = argv[1];
    std::string path_to_con_file = argv[2];
    std::string mode = argv[3];

    if (mode != "none" && mode != "fc") {
        std::cerr << "Invalid mode. Use 'none' or 'fc'." << std::endl;
        return 1;
    }

    std::unordered_map<char, std::vector<int>> variables = GetVariables(path_to_var_file);
    std::vector<Constraint> constraints = GetConstraints(path_to_con_file);


    CSP csp (variables, constraints, mode);
    csp.print_domain();
    std::cout << std::endl;
    std::cout << std::endl;

    csp.print_constraints();

    return 0;
}
