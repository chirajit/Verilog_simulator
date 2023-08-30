#include <stack>
int prec(char c)
{
    if (c == '!')
        return 3;
    else if (c == '|' || c == '&' || c == '^')
        return 2;
    else
        return -1;
}

string infix_to_postfix(string s)
{

    stack<char> st;
    string result;

    for (int i = 0; i < s.length(); i++) {
        char c = s[i];

        if (c == '0' || c == '1' || c == 'x' || c == 'z') result += c;

        else if (c == '(') st.push('(');

        else if (c == ')')
        {
            while (st.top() != '(')
            {
                result += st.top();
                st.pop();
            }
            st.pop();
        }

        else {
            while (!st.empty()
                && prec(s[i]) <= prec(st.top())) {
                result += st.top();
                st.pop();
            }
            st.push(c);
        }
    }

    while (!st.empty()) {
        result += st.top();
        st.pop();
    }

    return result;
}

map <char, char> not_table = { {'1', '0'}, {'0', '1'}, {'x', 'x'}, {'Z', 'Z'}};
map <char, int> char_to_int = { {'0', 0}, {'1', 1}, {'x', 2}, {'Z', 3}};
map<int, char> int_to_char {{0,'0'},{1,'1'},{2,'x'},{3,'z'}};
int or_table[4][4] = {{0,1,2,2},{1,1,1,1},{2,1,2,2},{2,1,2,2}};
int and_table[4][4] = {{0,0,0,0},{0,1,2,2},{0,2,2,2},{0,2,2,2}};
int xor_table[4][4] = {{0,1,2,2},{1,0,2,2},{2,2,2,2},{2,2,2,2}};

char evaluate_postfix(string exp)
{
    stack<char> st;
    for (int i = 0; i < exp.size(); ++i) {
        if ( exp[i] == '0' || exp[i] == '1' || exp[i] == 'x' || exp[i] == 'z')
            st.push(exp[i]);
        else {
            if (exp[i] == '!')
            {
                char val1 = st.top();
                st.pop();
                st.push(not_table[val1]);
            }
            else
            {
                int val1 = char_to_int[st.top()];
                st.pop();
                int val2 = char_to_int[st.top()];
                st.pop();
                switch (exp[i]) {
                case '&':
                    st.push(int_to_char[and_table[val1][val2]]);
                    break;
                case '|':
                    st.push(int_to_char[or_table[val1][val2]]);
                    break;
                case '^':
                    st.push(int_to_char[xor_table[val1][val2]]);
                    break;
                }
            }
        }
    }
    return st.top();
}

string expression_to_string (string module_name, vector <pair<string, string>> exp)
{
    string result = "";
    for (auto a = exp.begin(); a!= exp.end(); a++)
    {
        if (a->second == "binary" ) result += a->first[3];
        else if (a->second == "operator") result += a->first;
        else if (a->second == "id") 
        {
            if (modules[module_name].netlist.find(a->first) != modules[module_name].netlist.end())
            {
                result += ((valueholder*)(modules[module_name].netlist[a->first].second))->value();
            }
            else 
            {
                cout << "Variable doesn't exist in current context :" << a->first << "\t"<< module_name << endl;
                exit(1);
            }
        }
    }
    return result;
}

void update_reg_from_expression (string module_name, reg *r, vector<pair<string, string>> expression)
{
    string s = expression_to_string(module_name, expression);
    string s2 = infix_to_postfix(s);
    char ch = evaluate_postfix(s2);
    r->update_val(ch);
}

/*
int main()
{
    string s, s2;
    cin >> s;
    s2 = infix_to_postfix(s);
    cout << "postfix is : "<< s2 << endl;
    cout << evaluate_postfix (s2) << endl;
    return 0;
}
*/