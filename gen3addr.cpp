#include <iostream>
#include "simulate.cpp"
using namespace std;

enum ops {LOAD, STORE, MOVE, CMP, AND, OR, XOR, NOT, GOTO, LABEL, UPDATE_DRIVER, DISPLAY, END_DISPLAY};
unordered_map <int, string> printing = {{0, "LOAD"}, {1, "STORE"}, {2, "MOVE",},{3, "CMP"}, {4,"AND"}, {5,"OR"},
{6, "XOR"}, {7, "NOT"}, {8, "GOTO"}, {9, "LABEL"}, {10, "UPDATE_DRIVER"}, {11, "DISPLAY"}, {12, "END_DISPLAY"}};


class node3addr
{
	public :
	int oper;
	string operand1, operand2, operand3;
	string label;

	node3addr (int op, string op1 ="", string op2 = "", string op3 = "", string lb = "")
	{
		oper = op; operand1 = op1; operand2 = op2; operand3 = op3; label = lb;
	}
};

struct port_conn 
{
	string module_type1, module_name1, module_type2 , module_name2;
	int offset1, offset2;
};
 vector <struct port_conn> port_conn_vect;
vector <node3addr> arr3addr;

struct driver
{
	int var_to_update, process_no;
};

unordered_map <string , vector<struct driver>> driver_map;

void insert_into_arr3addr (int op, string op1 ="", string op2 = "", string op3 = "", string lb = "")
{
	//fptr << lb << endl;
	node3addr node(op, op1, op2, op3, lb);
	arr3addr.push_back (node);
}

void recur_elab (string module_type, string module_name ="")
{
	auto a  = modules[module_type];
	fptr << "INIT_ARCH ( " << (a.netlist.size()*9) << " , " << module_type << " , " << module_name << " )" << endl;
	for (auto b= a.netlist.begin(); b != a.netlist.end(); b++)
	{
		if (b->second.first == "reg") fptr << "\tCREATE_REG ( "<< b->first << " , ARCH+"<< a.offset[b->first] << " ) " << endl;
		else if (b->second.first == "wire") fptr << "\tCREATE_WIRE ( "<< b->first << " , ARCH+"<< a.offset[b->first] << " ) " << endl;
	}
	for (auto b = a.processes.begin(); b!= a.processes.end(); b++) fptr << "\tCREATE_" << b->first << "_PROC ( " << b->second << " ) "<< endl;
	for (auto c = a.netlist.begin(); c!= a.netlist.end(); c++)
	{
		valueholder * r = ((valueholder *)(c->second.second));
		for (auto d = r->triggers.begin(); d!= r->triggers.end(); d++)
		{
			fptr << "\tADD_TRIGGERS ( " << "ARCH+"+to_string(a.offset[(r->var_name())]) << " , " << (d->first) << " ) " << endl;

		}
	}

	for (auto b = driver_map[module_type].begin(); b!= driver_map[module_type].end(); b++)
	{
		fptr << "\tADD_DRIVER ( " << b->var_to_update << " , " << b->process_no << " )" << endl;
	}

	for (auto b = a.instancelist.begin(); b != a.instancelist.end(); b++)
	{
		for (int i = 0; i < a.connections[b->second].size(); i++)
		{
			struct port_conn pc;
			pc.module_type1 = module_type;
			pc.module_name1 = module_name;
			pc.offset1 = a.offset[a.connections[b->second][i]];
			pc.module_type2 = b->first;
			pc.module_name2 = b->second;
			pc.offset2 = modules[b->first].offset[modules[b->first].port_ordering[i]];
			if (pc.module_name1 == "") pc.module_name1 = pc.module_type1;
			port_conn_vect.push_back(pc);
		}
	}
	vector <int> offset_vect ; 
	for (auto au = mymonitor.monitor_vars.begin(); au!= mymonitor.monitor_vars.end();au++)
	{
		if(au->first == module_type) 
		{
			offset_vect.push_back(a.offset[au->second]);
		}
	}

	if (offset_vect.size() > 0 )
	{
		fptr << "\tCREATE_MONITOR ( ";
		for(int i =0; i< offset_vect.size(); i++) 
		{
			fptr << offset_vect[i] ;
			if (i ==  offset_vect.size()-1) fptr << " ) ";
			else fptr << " , ";
		}
		fptr << endl;
	}

	for (auto b = a.instancelist.begin(); b != a.instancelist.end(); b++) recur_elab (b->first, b->second);
}

void print_port_conn ()
{
	for (auto a = port_conn_vect.begin(); a != port_conn_vect.end(); a++)
	{
		fptr << "PORT_CONNECTION ( " << a->module_type1 << " ," << a->module_name1 << " , " << a->offset1 << " , " ;
		fptr << a->module_type2 << " ," << a->module_name2 << " , " << a->offset2 << " ) " << endl ;
	}
}

void print_arr_3addr ()
{
	for (auto a = arr3addr.begin(); a!= arr3addr.end(); a++)
	{
		if (a->label == "NBA") fptr << "\tNBA";
		if (a->oper == 9) {fptr << "-> " << a-> label << " :" << endl; continue;}
		fptr <<"\t"<< printing[a->oper] << "\t" << a->operand1 << " " << a->operand2 << " " << a->operand3 << " ";
		fptr << endl;
	}
}

