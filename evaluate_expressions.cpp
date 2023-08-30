#include "gen3addr.cpp"

struct expr_update_list  
{
	string var_name, module_name;
	vector<pair<string, string>> expression;
	int assign_state = 0;
};

int temp_no = 1, assign_var = 0;
void exp_to_3addr (struct expr_update_list eul, int nbastate = 0)
{
	vector <pair<string, string>> result;
	stack <pair<string, string>> st;
	for (int i =0; i < eul.expression.size(); i++)
	{
		pair <string , string> p = eul.expression[i];
		if (p.second == "binary" || p.second == "id")
			result.push_back(p);
		else if (p.second == "operator")
		{
			if (p.first == "(")
				st.push(p);
			else if (p.first == ")")
			{
				while(st.top().first != "(")
				{	
					result.push_back(st.top());
					st.pop();
				}
				st.pop();
			}

			else {
				while (!st.empty() && prec(p.first[0]) <= prec(st.top().first[0]))
				{
					result.push_back(st.top());
					st.pop();
				}
				st.push(p);
			}
		}
	}

	while(!st.empty()) 
	{
		result.push_back( st.top());
		st.pop();
	}
	if (eul.assign_state == 1)
	{
		assign_no++; insert_into_arr3addr(LABEL, "", "", "", "assign"+to_string(assign_no)); 
		modules[eul.module_name].processes.push_back(make_pair("ASSIGN","assign"+to_string(assign_no)));
		struct driver dr = {modules[eul.module_name].offset[eul.var_name], assign_no};
		driver_map[eul.module_name].push_back(dr);
	}
	for (auto a = result.begin(); a != result.end(); a++)
	{
		if (eul.assign_state == 1 && a->second == "id") ((valueholder*)(modules[eul.module_name].netlist[a->first].second))->triggers.push_back(make_pair("assign"+to_string(assign_no), null_void));
		if (a->second == "binary" || a->second == "id")
			st.push(*a);
		else if (a->second == "operator")
		{
			if (a->first[0]== '!' )
			{
				pair <string, string> p = st.top();
				st.pop();
				if (p.first[0] != 't')
				{
					insert_into_arr3addr (LOAD, "t"+to_string(temp_no), "ARCH+" + to_string(modules[eul.module_name].offset[p.first])); 
					insert_into_arr3addr (NOT, "t"+to_string(temp_no+1), "t"+to_string(temp_no)); temp_no++;
				}
				else insert_into_arr3addr (NOT, "t"+to_string(temp_no), p.first);
				p.first = "t"+to_string(temp_no);
				temp_no++ ;
				p.second = "id" ;
				st.push(p) ;
			}
			else
			{
				pair <string, string> p1 = st.top(); st.pop();
				pair <string, string> p2 = st.top(); st.pop();
				if (p1.first[0] != 't') {p1.first = "ARCH+" + to_string(modules[eul.module_name].offset[p1.first]); insert_into_arr3addr(LOAD, "t"+to_string(temp_no), p1.first);p1.first = "t"+to_string(temp_no); temp_no++; }
				if (p2.first[0] != 't') {p2.first = "ARCH+" + to_string(modules[eul.module_name].offset[p2.first]); insert_into_arr3addr(LOAD, "t"+to_string(temp_no), p2.first);p2.first = "t"+to_string(temp_no); temp_no++; }
				pair <string, string> p3 = make_pair("t"+to_string(temp_no), "id");
				temp_no++;
				switch (a->first[0])
				{
					case '&' : insert_into_arr3addr(AND, p3.first, p1.first , p2.first);
					st.push(p3);
					break;
					case '|' : insert_into_arr3addr(OR, p3.first, p1.first , p2.first);
					st.push(p3);
					break;
					case '^' : insert_into_arr3addr(XOR, p3.first, p1.first , p2.first);
					st.push(p3);
					break;

				}
					
			}
		}
	}
	if (nbastate == 0)
	{
		if (st.top().second == "binary")
		{
			if (eul.assign_state == 1 && modules[eul.module_name].netlist[eul.var_name].first == "wire") 
				insert_into_arr3addr(UPDATE_DRIVER , "ARCH+" + to_string(modules[eul.module_name].offset[eul.var_name]), st.top().first);
			else insert_into_arr3addr(STORE , "ARCH+" + to_string(modules[eul.module_name].offset[eul.var_name]), st.top().first);
		}
		if (st.top().second == "id")
		{
			if (st.top().first[0] != 't') st.top().first = "ARCH+" + to_string(modules[eul.module_name].offset[st.top().first]);
			int ii = modules[eul.module_name].offset[eul.var_name];
			if (eul.assign_state == 1 && modules[eul.module_name].netlist[eul.var_name].first == "wire") insert_into_arr3addr(UPDATE_DRIVER , "ARCH+"+to_string(ii), st.top().first);
			else insert_into_arr3addr(MOVE , "ARCH+"+to_string(ii), st.top().first);
		}
	}
	else
	{
		if (st.top().second == "binary")
			insert_into_arr3addr(STORE , "ARCH+" + to_string(modules[eul.module_name].offset[eul.var_name]), st.top().first, "", "NBA");
		if (st.top().second == "id")
		{
			if (st.top().first[0] != 't') st.top().first = "ARCH+" + to_string(modules[eul.module_name].offset[st.top().first]);
			int ii = modules[eul.module_name].offset[eul.var_name];
			if (eul.assign_state == 1 && modules[eul.module_name].netlist[eul.var_name].first == "wire") insert_into_arr3addr(UPDATE_DRIVER , "ARCH+"+to_string(ii), st.top().first, "", "NAB");
			else insert_into_arr3addr(MOVE , "ARCH+"+to_string(ii), st.top().first, "", "NBA");
		}
	}
}


class expressions
{
	vector <pair<string,string>> expr ;
	vector<struct expr_update_list> expression_update_list;
	vector<struct expr_update_list> expression_for_init;
	vector<struct expr_update_list> nba_for_init;
	vector<struct expr_update_list> always_expr_list;
	vector<struct expr_update_list> nba_for_always;
	public :
	void add_ops(string s, string type )
	{
		expr.push_back(make_pair(s, type));
	}

	vector <pair<string,string>> current_expr()
	{
		return expr;
	}

	void clear()
	{
		expr.clear();
	}

	void insert_into_expression_update_list (string var_name, string module_name, int initial_state, int nba_state)
	{
		struct expr_update_list eul;
		eul.var_name = var_name;
		eul.module_name = module_name;
		vector<pair<string, string>> vs = expr;
		eul.expression = expr;
		if (assign_var == 1) eul.assign_state= 1;
		if (initial_state == 0) {expression_update_list.push_back(eul);}
		else if (initial_state == 2) 
		{
			if (nba_state) nba_for_always.push_back(eul);
			else always_expr_list.push_back(eul);
		}
		else 
		{
			if (nba_state) nba_for_init.push_back(eul);
			else expression_for_init.push_back(eul);
		}
		clear();
	}

	void print()
	{
		for (auto a = expression_update_list.begin(); a != expression_update_list.end(); a++)
		{
			cout << a->var_name << " " << a->module_name << " " ;
			for (auto b = a->expression.begin(); b != a->expression.end(); b++) cout << b->first << " " << b->second << "\t" ;
			cout << endl;
		}
		cout << endl << "Init expressions" << endl ;
		for (auto a = expression_for_init.begin(); a != expression_for_init.end(); a++)
		{
			cout << a->var_name << " " << a->module_name << " " ;
			for (auto b = a->expression.begin(); b != a->expression.end(); b++) cout << b->first << " " << b->second << "\t" ;
			cout << endl;
		}
	}
	void trim_list()
	{
		vector<struct expr_update_list>  temp ;
		vector<struct expr_update_list>  temp2 ;
		for (auto a = expression_update_list.begin() ; a != expression_update_list.end(); a++)
		{
			if (modules[a->module_name].netlist.find(a->var_name) != modules[a->module_name].netlist.end())
			{
				temp.push_back(*a);
			}
		}
		expression_update_list = temp;

		for (auto a = expression_for_init.begin() ; a != expression_for_init.end(); a++)
		{
			if (modules[a->module_name].netlist.find(a->var_name) != modules[a->module_name].netlist.end())
			{
				temp2.push_back(*a);
			}
		}
		expression_for_init = temp2;

		for (auto a = nba_for_init.begin() ; a != nba_for_init.end(); a++)
		{
			if (modules[a->module_name].netlist.find(a->var_name) != modules[a->module_name].netlist.end())
			{
				temp2.push_back(*a);
			}
		}
		nba_for_init = temp2;

		for (auto a = always_expr_list.begin() ; a != always_expr_list.end(); a++)
		{
			if (modules[a->module_name].netlist.find(a->var_name) != modules[a->module_name].netlist.end())
			{
				temp2.push_back(*a);
			}
		}
		always_expr_list = temp2;
	}

	void update_variables ()
	{
        insert_into_arr3addr(LABEL, "", "", "", "before_init"); 
		for (auto a = expression_update_list.begin(); a != expression_update_list.end(); a++)
		{
			struct expr_update_list neweul = *a;
			exp_to_3addr(neweul);
			if (modules[a->module_name].netlist[a->var_name].first == "wire" )
			{
				if (a->expression[0].second == "binary")
				{
					char *c = new char;
					*c = a->expression[0].first[3];
					((wire *)(modules[a->module_name].netlist[a->var_name].second))->add_driver(c);
				}
				if (a->expression[0].second == "id")
				{
					if (modules[a->module_name].netlist.find(a->expression[0].first) != modules[a->module_name].netlist.end())
					{
						if ( modules[a->module_name].netlist[a->expression[0].first].first == "wire" || modules[a->module_name].netlist[a->expression[0].first].first == "reg" )
						{
							((wire *)(modules[a->module_name].netlist[a->var_name].second))->add_driver(((valueholder*)(modules[a->module_name].netlist[a->expression[0].first].second))->valptr());
						}
					}
				}
			}
			else if (modules[a->module_name].netlist[a->var_name].first == "reg" )
				update_reg_from_expression (a->module_name, ((reg*)(modules[a->module_name].netlist[a->var_name].second)), a->expression);
		}
	}

	void schedule_nba(initial init)
	{
		int curr_state = init.current_state();
		init.shift_to_state(2);
		for(auto a  = nba_for_init.begin(); a != nba_for_init.end(); a++)
		{
			struct expr_update_list neweul = *a;
			exp_to_3addr(neweul,1);
			if (modules[a->module_name].netlist[a->var_name].first == "reg" )
				init.add_event(((reg *)(modules[a->module_name].netlist[a->var_name].second)), a->module_name, a->expression);
		}
		init.shift_to_state(curr_state);
		nba_for_init.clear();
	}
	void schedule_initial(initial init)
	{
		for(auto a  = expression_for_init.begin(); a != expression_for_init.end(); a++)
		{
			struct expr_update_list neweul = *a;
			exp_to_3addr(neweul);
			if (modules[a->module_name].netlist[a->var_name].first == "reg" )
				init.add_event(((reg *)(modules[a->module_name].netlist[a->var_name].second)), a->module_name, a->expression);
		}
		expression_for_init.clear();
		schedule_nba(init);
	}

	void schedule_always(always *alw)
	{
		for(auto a  = always_expr_list.begin(); a != always_expr_list.end(); a++)
		{

			struct expr_update_list neweul = *a;
			exp_to_3addr(neweul);
			if (modules[a->module_name].netlist[a->var_name].first == "reg" )
				alw->add_event(((reg *)(modules[a->module_name].netlist[a->var_name].second)), a->module_name, a->expression);
		}
		always_expr_list.clear();
		for(auto a  = nba_for_always.begin(); a != nba_for_always.end(); a++)
		{

			struct expr_update_list neweul = *a;
			exp_to_3addr(neweul, 1);
			if (modules[a->module_name].netlist[a->var_name].first == "reg" )
				alw->add_event(((reg *)(modules[a->module_name].netlist[a->var_name].second)), a->module_name, a->expression);
		}
		nba_for_always.clear();
	}

};
