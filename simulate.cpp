#include "calculate_expression.cpp"
void update_all_wires ();
void execute_all_triggers(string , void* , void* , string );

class monitor
{
	vector <char> values_to_print;
	vector <char> prev_values;
	public :
	vector <pair <string, string>> monitor_vars;
	void populate_values_to_print ()
	{
		prev_values = values_to_print;
		values_to_print.clear();
		for (auto a = monitor_vars.begin(); a != monitor_vars.end(); a++)
		{
			if (modules[a->first].netlist.find(a->second) == modules[a->first].netlist.end())
			{
				cout << "monitor varibles not declared in this scope" <<endl;
				exit (1);
			}
			values_to_print.push_back(((valueholder *)(modules[a->first].netlist[a->second].second))->value());
		}
	}

	void add_monitor_var(string module_name, string var_name)
	{
		monitor_vars.push_back(make_pair(module_name, var_name));
	}

	void print(int t)
	{
		if (prev_values != values_to_print)
		{
			cout << "$time = " << t << endl;
			for (auto a = values_to_print.begin(); a != values_to_print.end(); a++) cout << (*a) << " ";
			cout << endl;
		}
	}
};
monitor mymonitor;

class display
{
	vector <char> values_to_print;
	public :
	unordered_map <int, vector <pair<string, string>>> display_vars;
	void add_display_var(string module_name, string var_name, int display_name)
	{
		if (display_vars.find(display_name) == display_vars.end())
		{
			vector <pair<string, string>> ssv;
			display_vars[display_name] = ssv;
		}
		display_vars[display_name].push_back(make_pair(module_name,var_name));
	}
	void populate_values_to_print (int display_name)
	{
		values_to_print.clear();
		for (auto a = display_vars[display_name].begin(); a != display_vars[display_name].end(); a++)
		{
			if (modules[a->first].netlist.find(a->second) == modules[a->first].netlist.end())
			{
				cout << "display varibles not declared in this scope" <<endl;
				exit (1);
			}
			values_to_print.push_back(((valueholder *)(modules[a->first].netlist[a->second].second))->value());
		}
	}
	void print(int display_name)
	{
		populate_values_to_print (display_name);
		for (auto a = values_to_print.begin(); a != values_to_print.end(); a++) cout << (*a) << " ";
		cout << endl;
	}
};

struct one_event
{
	reg * reg_ptr;
	string module_name;
	vector<pair<string, string>> expression;
};

typedef vector < struct one_event > events;

class always_now  
{
	int state = 0;
	public :
	vector <events> act;
	friend class always;
	always_now(){act.resize(3);}
	void add_event (reg *r, string ch, vector<pair<string, string>> exp) {struct one_event ev = {r, ch, exp}; act[state].push_back (ev); }
	void shift_next () { if (state < 3) state += 1; }
	void shift_to_state(int i) {state =i;}
	void start_initial() { state = 0;}
	void print()
	{
		for (int i = 0; i < 3; i ++)
		{
			cout <<i <<endl;
			for (auto j = act[i].begin(); j != act[i].end(); j++) cout << j->reg_ptr->var_name() << " = " << ((j->module_name)) << endl;
			cout << endl;
		}
	}
};

class always
{
	int time = 0; 
	public :
	map <int, always_now*> all_eve;
	always(){all_eve.insert({0, new always_now});}
	void add_event (reg *r, string ch, vector<pair<string, string>> exp) {all_eve[time]->add_event(r,ch,exp);}
	void update_time(int t) 
	{
		time = t;
		if (all_eve.find(time) == all_eve.end()) all_eve.insert({time, new always_now});
		all_eve[time]->start_initial();
	}

	int current_state()
	{
		return all_eve[time]->state;
	}
	void shift_to_state (int stateno) { all_eve[time]->shift_to_state(stateno);}
	void print()
	{
		cout << "Always events are :" << endl;
		for (auto i = all_eve.begin(); i != all_eve.end(); i++) 
		{
			cout << "$time = " << i->first <<endl;
			i->second->print();
		}
	}
	void analyse_delay_tok (string timer)
	{
		int i = stoi(timer);
		if (i == 0) shift_to_state(1);
		else
		{
			update_time (i + time);
		}
	}
};

class initial_now
{
	vector <events> act;
	int state = 0;
	public :
	friend class initial;
	initial_now(){act.resize(4);}
	void add_event (reg *r, string ch, vector<pair<string, string>> exp) {struct one_event ev = {r, ch, exp}; act[state].push_back (ev); }
	void shift_next () { if (state < 3) state += 1; }
	void shift_to_state(int i) {state =i;}
	void start_initial() { state = 0;}
	void execute_all() 
	{
		for (int i = 0; i < 4; i ++) 
		{
			while(act[i].begin() != act[i].end())
			{
				char prev_val = act[i].begin()->reg_ptr->value();
				if (prev_val == 'd')
				{
					((display*)(act[i].begin()->reg_ptr->global_display_var))->print(act[i].begin()->reg_ptr->display_no);
					act[i].erase(act[i].begin());
					continue;
				}
				update_reg_from_expression(act[i].begin()->module_name, act[i].begin()->reg_ptr, act[i].begin()->expression);
				//act[i].begin()->first->update_val(*(act[i].begin()->second));
				char updated_val = act[i].begin()->reg_ptr->value();
				if(prev_val != updated_val)
				{
					for (auto ab = act[i].begin()->reg_ptr->triggers.begin(); ab != act[i].begin()->reg_ptr->triggers.end(); ab++)
					{
						execute_all_triggers(ab->first, ab->second, (void*)(act[i].begin()->reg_ptr), act[i].begin()->module_name);
					}
				}
				act[i].erase(act[i].begin());
				update_all_wires();
			}
		}
	
	}
	void print()
	{
		for (int i = 0; i < 4; i ++)
		{
			cout <<i <<endl;
			for (auto j = act[i].begin(); j != act[i].end(); j++) cout << j->reg_ptr->var_name() << " = " << ((j->module_name)) << endl;
			cout << endl;
		}
	}
};

class initial
{
	map <int, initial_now*> all_eve;
	int time = 0; 
	public :
	initial(){all_eve.insert({0, new initial_now});}
	int  execution_time = 0;
	void add_event (reg *r, string ch, vector<pair<string, string>> exp = {{}}) {all_eve[time]->add_event(r,ch,exp);}
	void update_time(int t) 
	{
		time = t;
		if (all_eve.find(time) == all_eve.end()) all_eve.insert({time, new initial_now});
		all_eve[time]->start_initial();

	}
	void execute_all(monitor m)
	{
		while(all_eve.begin() != all_eve.end())
		{
			execution_time = all_eve.begin()->first;
			all_eve.begin()->second->execute_all();
			m.populate_values_to_print();
			m.print(all_eve.begin()->first);
			all_eve.erase(all_eve.begin());
		}
	}
	int current_state()
	{
		return all_eve[time]->state;
	}
	void shift_to_state (int stateno) { all_eve[time]->shift_to_state(stateno);}
	void print()
	{
		cout << "Scheduled events are :" << endl;
		for (auto i = all_eve.begin(); i != all_eve.end(); i++) 
		{
			cout << "$time = " << i->first <<endl;
			i->second->print();
		}
	}
	void analyse_delay_tok (string timer)
	{
		int i = stoi(timer);
		if (i == 0) shift_to_state(1);
		else
		{
			update_time (i + time);
		}
	}
};

void update_all_wires ()
{

	for (auto a = modules.begin(); a!= modules.end(); a++)
	{
		for (auto b = a->second.netlist.begin(); b!= a->second.netlist.end(); b++)
		{
			if (b->second.first == "wire")
			{
				char prev_val = ((wire*)b->second.second)->value();
				((wire*)b->second.second)->update_wire();
				char updated_val = ((wire*)b->second.second)->value();
				if (prev_val != updated_val)
				{
					for (auto ab = ((wire*)b->second.second)->triggers.begin(); ab != ((wire*)b->second.second)->triggers.end(); ab++)
					{
						execute_all_triggers(ab->first, ab->second, b->second.second, a->first);
					}
				}
			}
		}
	}
}

void execute_all_triggers(string typecheck, void* v, void *v2, string module_name)
{
	if (typecheck.substr(0,6) == "always")
	{
		always *alw = (always*)(v);
		for (auto i = alw->all_eve.begin(); i != alw->all_eve.end(); i++)
		{
			for (auto a = i->second->act.begin(); a != i->second->act.end(); a++)
			{
				int t = ((initial *)(global_void_ptr_for_init))->execution_time;
				((initial *)(global_void_ptr_for_init))->update_time(t+i->first);
				for (auto b= a->begin(); b!=a->end(); b++)
				{
					((initial *)(global_void_ptr_for_init))->add_event(b->reg_ptr, b->module_name, b->expression);
				}
				((initial *)(global_void_ptr_for_init))->shift_to_state(1);
			}
		}
	}
	else if (typecheck == "port_conn")
	{
		string t = ((pair <string, void*> *)(v))->first;
		if (t == "wire") 
		{
			((wire *)(((pair <string, void*> *)(v))->second))->add_driver(((valueholder*)v2)->valptr());
		}
		else if (t == "reg")
		{
			((reg *)(((pair <string, void*> *)(v))->second))->update_val(((valueholder*)v2)->value());
		}
	}
}

