#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <set>
#include "asol_part.cpp"
using namespace std;
static int number_of_tabs = 0;
void * null_void = NULL;
ofstream fptr("output.txt");
class node
{
public:
	unordered_map <string, pair<string, void*>> netlist;
	unordered_map <string, int> offset; 
	vector <string> input_list;
	vector <string> output_list;
	vector <string> port_ordering;
	vector <pair<string,string>> instancelist;
	unordered_map <string, vector<string>> connections;
	vector <pair<string, string>> processes;
};

static unordered_map <string, node> modules;

void insert_to_netlist(string modulename, string var_name, string inout_type = "none", string data_type = "none")
{
	if (modules[modulename].netlist.find(var_name) == modules[modulename].netlist.end())
	{
		if (inout_type == "input") 
		{
			modules[modulename].input_list.push_back(var_name);
		}
		if (inout_type == "output")
		{
			modules[modulename].output_list.push_back(var_name);
		}
		if (module_port == 1)
		{
			modules[modulename].port_ordering.push_back(var_name);
		}

		if (data_type == "wire" )
		{
			wire *w = new wire (var_name);
			modules[modulename].netlist[var_name] = make_pair(data_type, (void*)w);
		}
		else if (data_type == "reg" )
		{
			reg *r = new reg (var_name);
			modules[modulename].netlist[var_name] = make_pair(data_type, (void*)r);
		}
		else modules[modulename].netlist[var_name] = make_pair(data_type, null_void);
		if(modules[modulename].offset.find(var_name) == modules[modulename].offset.end())
			modules[modulename].offset[var_name] = modules[modulename].offset.size()*9 -9;
	}

}

void insert_to_instancelist(string modulename, string inst_type, string var_name)
{
	modules[modulename].instancelist.push_back(make_pair(inst_type, var_name));
}

void add_instance_connections(string modulename, string instancename, string connections)
{
	modules[modulename].connections[instancename].push_back(connections);
}

void print_tabs()
{
	for (int i = 0; i < number_of_tabs; i++) fptr << "\t";
}
void printmodules(string modulename, string module_type)
{
	print_tabs();
	fptr <<"("<< modulename <<" inputs= "<< modules[module_type].input_list.size() << " outputs = "<<modules[module_type].output_list.size() << endl;
	number_of_tabs++;
	print_tabs();
	fptr <<"(NET "<< endl;
	number_of_tabs++;
	for (auto j = modules[module_type].netlist.begin(); j != modules[module_type].netlist.end(); j++) 
	{
		print_tabs();
		fptr << "(" << (j->first) << " , type =  " << (j->second.first);
		if (j->second.first == "wire" || j->second.first == "reg"  )
		{
			valueholder *v = ((valueholder*)(j->second.second));
			fptr << " , value  = "<< v->value() << " ";
			for (auto a_var = v->triggers.begin(); a_var != v->triggers.end(); a_var++)
			{
				fptr << a_var->first << " , " ;
			}
		}
		fptr  << ")" << endl;
	}
	number_of_tabs--;
	print_tabs();
	fptr << ")"<<endl;
	for (auto j = modules[module_type].instancelist.begin(); j != modules[module_type].instancelist.end(); j++) 
	{
		printmodules(j->second, j->first);
	}
	for (auto i = modules[module_type].connections.begin(); i!=modules[module_type].connections.end() ; i++)
	{
		print_tabs();
		fptr << i-> first << " : " ;
		for (auto j = i->second.begin(); j!= i->second.end(); j++)
		{
			fptr << (*j) << " , ";
		}
		fptr << endl;
	}
	number_of_tabs--;
	print_tabs();
	fptr << ")"<<endl;
}

void print_all_modules ()
{
	for (auto itr = modules.begin(); itr != modules.end(); itr++) printmodules(itr->first, itr->first);
}

void connect_modules ()
{
	for (auto a = modules.begin(); a!= modules.end(); a++)
	{
		for (auto b = a->second.instancelist.begin(); b != a->second.instancelist.end(); b++)
		{
			vector <string> v1 = a->second.connections[b->second];
			vector <string> v2 = modules[b->first].port_ordering;
			int s = v1.size();
			for (int i = 0; i<s; i++) 
			{
				valueholder *r1 = ((valueholder*)(modules[b->first].netlist[v2[i]].second));
				valueholder *r2 = ((valueholder*)(a->second.netlist[v1[i]].second)) ;
				if (r1 != NULL)
				r1->triggers.push_back(make_pair("port_conn", ((void*)(&(a->second.netlist[v1[i]])))));
				if (r2 != NULL)
				r2->triggers.push_back(make_pair("port_conn",((void*)(&(modules[b->first].netlist[v2[i]])))));
			}
		}
	}

}