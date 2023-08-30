#include <map>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream> 
#define stringify(name) #name
using namespace std;

vector <string> split_str(string const &, const char )  ;
map <string ,  map < string, void *>* > design;
vector <string> parameters;
//module_type, module_name, base
void* globalv = NULL ;

enum myen {INIT_ARCH, CREATE_REG, CREATE_WIRE, CREATE_INITIAL_PROC, CREATE_ALWAYS_PROC,
		CREATE_ASSIGN_PROC, ADD_TRIGGERS, PORT_CONNECTION, ADD_DRIVER , CREATE_MONITOR};
myen func_name;

enum oper {STORE , MOVE, LOAD, OR, AND, NOT, UPDATE_DRIVER, NBA_MOVE, NBA_STORE, NBA_UPDATE_DRIVER, DISPLAY, END_DISPLAY};
map <string, oper> string_to_op  = { {"STORE",STORE } , {"MOVE",MOVE }, {"LOAD" , LOAD}, {"OR",OR }, {"AND", AND}, {"NOT" ,NOT }, {"UPDATE_DRIVER", UPDATE_DRIVER}, {"DISPLAY", DISPLAY}, {"END_DISPLAY", END_DISPLAY} };

enum proc {initial, always, assign , port_conn};
proc track_inial;

struct process
{
	proc type;
	int entry, codestart = 0;
	void *base;
};

struct operation 
{
	oper op;
	vector <string> operands;
};

struct driver
{
	char value;
	int process_no;
};

struct port_connection
{
	void * base1, *base2;
	int offset1, offset2;
};

struct nba
{
	void * base;
	int offset, value;
};

vector <struct process *> process_list;
vector <struct port_connection> port_connection_list;
int assign_process_no=0;

class monitor 
{
	vector <char> prev_vals;
	vector <char> values_to_print;
	public :
	vector <pair<void*, int>> monitor_vars;
	void show_monitor(int t)
	{
		prev_vals = values_to_print;
		values_to_print.clear();
		for (int i = 0; i < monitor_vars.size(); i++)
		{
			values_to_print.push_back(*((char*)(monitor_vars[i].first)+monitor_vars[i].second));
		}
		if (values_to_print != prev_vals)
		{
			cout << "time = " << t  << " : ";
			for (int i = 0; i< values_to_print.size(); i++) cout << values_to_print[i] << " ";
			cout << endl;
		}
	}
};
monitor mymonitor;
class process_que

{
	map <char, char> not_table = { {'1', '0'}, {'0', '1'}, {'x', 'x'}, {'Z', 'Z'}};
	map <char, int> char_to_int = { {'0', 0}, {'1', 1}, {'x', 2}, {'Z', 3}};
	vector <char> int_to_char = {'0','1','x','z'};
	int or_table[4][4] = {{0,1,2,2},{1,1,1,1},{2,1,2,2},{2,1,2,2}};
	int and_table[4][4] = {{0,0,0,0},{0,1,2,2},{0,2,2,2},{0,2,2,2}};
	int resolution_table[4][4] = {{0,2,2,0}, {2,1,2,1}, {2,2,2,2}, {0,1,2,3}};
	vector <struct nba*> nba_list;
	struct nba* nb = new struct nba;
	public :
	int time = 0, priority = 0;
	map <string, char> temps;
	vector <vector <struct process>> proc_que;
	map <int, vector<struct process>> future_que;
	process_que()
	{
		proc_que.resize(3);
	}

	void schedule0()
	{
		for (auto a = process_list.begin(); a!= process_list.end(); a++)
		{
			if((*a)->type == assign) proc_que[0].push_back(**a);
		}	
		for (auto a = process_list.begin(); a!= process_list.end(); a++)
		{
			if((*a)->type == initial) proc_que[0].push_back(**a);
		}
	}

	void print_proc_que()
	{
		for (auto a = proc_que.begin(); a!= proc_que.end(); a++)
		{
			for (auto b = a->begin(); b!= a->end(); b++)
			{
				printf( " proesss = %d , entry = %d, codestart = %d\n", b->type, b->entry, b->codestart);
			}
		}
	}

	void execute_all(string str)
	{
		int exec_time = 0;
		ifstream fp (str);
		while(proc_que[0].size())
		{
			int exec=  0;
			string st;
			while(exec < proc_que[0].size())
			{
				//printf ("\n\n executing process : %d, entry : %d, file offset = %d\n", proc_que[0][exec].type, proc_que[0][exec].entry, proc_que[0][exec].codestart);
				fp.seekg(proc_que[0][exec].codestart, ios::beg);
				int attempts = 0;
				int position = proc_que[0][exec].codestart;
				while(fp.tellg() == -1 && attempts < 3) 
				{
					fp.close();
					fp.open(str);
					fp.seekg(proc_que[0][exec].codestart, ios::beg);
					attempts ++;
				}
				while(getline(fp, st))
				{
					position += st.length()+2;
					vector <string> vec = split_str(st, ' ');
					if (vec[0] == "->") 
					{
						if (proc_que[0][exec].type == initial || proc_que[0][exec].type == always)
						{
							if (vec[1].length() > 5 && vec[1].substr(0,5) == "delay")
							{
								int i = stoi(vec[1].substr(5,vec[1].length()-5));
								struct process pr = {proc_que[0][exec].type, proc_que[0][exec].entry, position, proc_que[0][exec].base};
								future_que[i+exec_time].push_back(pr);
							}
						}
						break;
					}
					//for (int iii = 0; iii < vec.size(); iii++) cout << vec[iii] << " ";
					struct operation op;
					if (vec[0] == "NBA")
					{
						if (vec[1] == "STORE") op.op = NBA_STORE;
						else if (vec[1] == "MOVE") op.op = NBA_MOVE;
						else if (vec[1] == "UPDATE_DRIVER") op.op = NBA_UPDATE_DRIVER;
						vec.erase(vec.begin());
					}
					else op.op = string_to_op[vec[0]];
					vec.erase(vec.begin());
					op.operands = vec;
					perform_operations(op, proc_que[0][exec].base, proc_que[0][exec].entry);
					//cout << endl;
				}				
				exec ++;
			}
			proc_que[0].clear();
			mymonitor.show_monitor(exec_time);
			if (future_que.size() == 0 || exec_time != -1*((future_que.begin())->first))
			{
				execute_nba();
				nba_list.clear();
			}
			if (future_que.size() > 0)
			{
				proc_que[0] = (future_que.begin())->second;
				exec_time = ((future_que.begin())->first);
				future_que.erase(future_que.begin());
			}
		}
		fp.close();
	}
	void activate_triggers(void* base, int offset)
	{
		vector<struct process*>* pr = (*((vector<struct process*>**)((char*)(base)+ offset+1)));
		for (int i = 0; i < pr->size(); i++)
		{
			if (((*pr)[i])->type == port_conn)
			{
				char ch = *(((char*)((((*pr)[i])->base))+((*pr)[i])->codestart));
				*(((char*)(((*pr)[i])->base))+((*pr)[i])->codestart) = *((char*)(base)+offset);
				if (ch != *((char*)(base)+offset))
					activate_triggers (((*pr)[i])->base, ((*pr)[i])->codestart);
			}
			else proc_que[0].push_back(*((*pr)[i]));
			//cout<< "inserted_trigger : " << (*((*pr)[i])).type << " entry = " << (*((*pr)[i])).entry << endl;
		}
	}
	void perform_operations(struct operation op, void* base, int ass_process_no)
	{
		char ch;
		switch (op.op)
		{
			case STORE :ch = *((char*)(base)+stoi(op.operands[0]));
						*((char*)(base)+stoi(op.operands[0])) = op.operands[1][3];
						if (ch != op.operands[1][3]) activate_triggers(base, stoi(op.operands[0]));
						//printf (" updated value : %c -> %c", ch, op.operands[1][3]);
						break;

			case MOVE :	ch = *((char*)(base)+stoi(op.operands[0]));
						if (op.operands[1][0] != 't') *((char*)(base)+stoi(op.operands[0])) = *((char*)(base)+stoi(op.operands[1])) ;
						else *((char*)(base)+stoi(op.operands[0])) = temps[op.operands[1]];
						if (ch != *((char*)(base)+stoi(op.operands[0]))) activate_triggers(base, stoi(op.operands[0]));
						//printf (" updated value : %c -> %c", ch,*((char*)(base)+stoi(op.operands[0])));
						break;

			case LOAD : temps[op.operands[0]] = (*((char*)(base)+stoi(op.operands[1]))); break;

			case OR : 	temps[op.operands[0]] = int_to_char[or_table[char_to_int[temps[op.operands[1]]]][char_to_int[temps[op.operands[2]]]]]; break;

			case AND :	temps[op.operands[0]] = int_to_char[and_table[char_to_int[temps[op.operands[1]]]][char_to_int[temps[op.operands[2]]]]]; break;

			case NOT: 	temps[op.operands[0]] = not_table[temps[op.operands[1]]]; break;

			case UPDATE_DRIVER : ch = *((char*)(base)+stoi(op.operands[0]));
								for (auto a = (*((vector<struct driver*>**)((char*)(base)+ stoi(op.operands[0])+5)))->begin(); a != (*((vector<struct driver*>**)((char*)(base)+ stoi(op.operands[0])+5)))->end(); a++)
								{
									if ((*a)->process_no == ass_process_no)
									{
										if (op.operands[1][0] != 't') (*a)->value = *((char*)(base)+stoi(op.operands[1])) ;
										else (*a)->value = temps[op.operands[1]];
										break;
									}
								}
								update_wire(base, stoi(op.operands[0]));
								if (ch != *((char*)(base)+stoi(op.operands[0]))) activate_triggers(base, stoi(op.operands[0]));
								//printf (" updated value : %c -> %c", ch,*((char*)(base)+stoi(op.operands[0])));
								break;
			case NBA_MOVE : nb->base = base; nb->offset = stoi(op.operands[0]); nb->value = *((char*)(base)+stoi(op.operands[1]));
							nba_list.push_back(nb);
							break;
			case NBA_STORE :nb->base = base; nb->offset = stoi(op.operands[0]); nb->value = op.operands[1][3];
							nba_list.push_back(nb);
							break;
			case NBA_UPDATE_DRIVER : for (auto a = (*((vector<struct driver*>**)((char*)(base)+ stoi(op.operands[0])+5)))->begin(); a != (*((vector<struct driver*>**)((char*)(base)+ stoi(op.operands[0])+5)))->end(); a++)
									{
										if ((*a)->process_no == ass_process_no)
										{
											if (op.operands[1][0] != 't') (*a)->value = *((char*)(base)+stoi(op.operands[1])) ;
											else (*a)->value = temps[op.operands[1]];
											break;
										}
									}
									nb->base = base; nb->offset = stoi(op.operands[0]); nb->value = 'd';
			case DISPLAY :  cout << (*((char*)(base)+stoi(op.operands[0]))) << " ";
							break;
			case END_DISPLAY : cout << endl;
								break;
			default :	break;
		}
	}

	void update_wire(void * base, int offset)
	{
		vector<struct driver*> *driver_vec = (*((vector<struct driver*>**)((char*)(base)+ offset+5)));
		if (driver_vec->size() == 0) return ;
		int valnow = char_to_int[(*(driver_vec->begin()))->value];
		for (auto a = driver_vec->begin(); a!= driver_vec->end(); a++)
		{
			int i = char_to_int[((*a)->value)];
			valnow = resolution_table[valnow][i];
		}
		*((char*)(base)+offset) = int_to_char[valnow];
	}
	void execute_nba()
	{
		for (auto a= nba_list.begin(); a != nba_list.end(); a++)
		{
			if((*a)->value != 'd')
				*((char*)((*a)->base)+(*a)->offset) = (*a)->value;
			else update_wire((*a)->base, (*a)->offset);
		}
	}
};

process_que pque;

void create_initial_proc (proc proc_type, int entry)
{
	struct process *pr = new struct process;
	pr->type = proc_type;
	pr->entry = entry;
	pr->codestart = 0;
	pr->base = globalv;
	*pr =  { proc_type, entry, 0,globalv };
	process_list.push_back(pr);
}
void create_assign_proc (proc proc_type, int entry)
{
	struct process *pr = new struct process;
	pr->type = proc_type;
	pr->entry = entry;
	pr->codestart = 0;
	pr->base = globalv;
	*pr =  { proc_type, entry, 0,globalv };
	process_list.push_back(pr);
}
void create_always_proc (proc proc_type, int entry)
{
	struct process *pr = new struct process;
	pr->type = proc_type;
	pr->entry = entry;
	pr->codestart = 0;
	pr->base = globalv;
	*pr =  { proc_type, entry, 0,globalv };
	process_list.push_back(pr);
}

void init_arch (int size, string module_type, string module_name)
{
	void * v = malloc(size);
	globalv = v;
	if (design.find(module_type) == design.end())
	{
		map <string, void *> *mp = new map<string, void*>;
		design[module_type] = mp;
	}
	
	design[module_type]->insert({module_name, v});
}

void create_reg (int offset)
{
	*((char*)(globalv)+ offset) = 'x';
	vector<struct process> *v = new vector<struct process>;
	*((vector<struct process>**)((char*)(globalv)+ offset+1)) = v;
	vector<struct driver*> *d = new vector<struct driver*>;
	*((vector<struct driver*>**)((char*)(globalv)+ offset+5)) = d;
}

void create_wire (int offset)
{
	*((char*)(globalv)+ offset) = 'z';
	vector<struct process*> *v = new vector<struct process*>;
	*((vector<struct process*>**)((char*)(globalv)+ offset+1)) = v;
	vector<struct driver*> *d = new vector<struct driver*>;
	*((vector<struct driver*>**)((char*)(globalv)+ offset+5)) = d;
}

void add_triggers (int offset, string te)
{
	if (te.length() > 7 && te.substr(0,7) == stringify(initial))
	{
		for (int a = 0; a < process_list.size(); a++)
		{
			if(process_list[a]->type == initial && process_list[a]->entry == stoi(te.substr(7, te.length()-7))) 
				(*((vector<struct process*>**)((char*)(globalv)+ offset+1)))->push_back(process_list[a]);
		}	
	}
	else if (te.length() > 6 && te.substr(0,6) == stringify(always))
	{
		for (int a = 0; a < process_list.size(); a++)
		{
			if(process_list[a]->type == always && process_list[a]->entry == stoi(te.substr(6, te.length()-6))) 
				(*((vector<struct process*>**)((char*)(globalv)+ offset+1)))->push_back(process_list[a]);
		}
	}
	else if (te.length() > 6 && te.substr(0,6) == stringify(assign))
	{
		for (int a = 0; a < process_list.size(); a++)
		{
			if(process_list[a]->type == assign && process_list[a]->entry == stoi(te.substr(6, te.length()-6))) 
				(*((vector<struct process*>**)((char*)(globalv)+ offset+1)))->push_back(process_list[a]);
		}
	}
/*		vector<struct process*>* pr = (*((vector<struct process*>**)((char*)(globalv)+ offset+1)));
		for (int i = 0; i < pr->size(); i++)
		{
			//proc_que[0].push_back(*((*pr)[i]));
			cout<< "inserted_trigger : " << (*((*pr)[i])).type << " entry = " << (*((*pr)[i])).entry << endl;
		}*/
}

void add_driver (int offset, int process_no)
{
	struct driver *dr = new struct driver;
	dr->value= 'z'; dr->process_no = process_no;
	(*((vector<struct driver*>**)((char*)(globalv)+ offset+5)))->push_back(dr);
}

void add_port_connection(void * base1, void * base2, int offset1, int offset2)
{
	struct port_connection pc = {base1, base2, offset1, offset2};
	port_connection_list.push_back(pc);
}

void port_to_trig()
{
	for (int i =0; i < port_connection_list.size(); i++ )
	{
		struct process *pr1 = new struct process;
		pr1->type = port_conn; pr1->entry = -1; pr1->codestart= port_connection_list[i].offset1; pr1->base= port_connection_list[i].base1;
		struct process *pr2 = new struct process;
		pr2->type = port_conn; pr2->entry = -1; pr2->codestart= port_connection_list[i].offset2; pr2->base= port_connection_list[i].base2;
		(*((vector<struct process*>**)((char*)(port_connection_list[i].base1)+ port_connection_list[i].offset1+1)))->push_back(pr2);
		(*((vector<struct process*>**)((char*)(port_connection_list[i].base2)+ port_connection_list[i].offset2+1)))->push_back(pr1);
	}
}

void analyse_label (string te, int codestart)
{
	
if (te.length() > 7 && te.substr(0,7) == stringify(initial))
	{
		track_inial = initial;
		for (auto a = process_list.begin(); a!= process_list.end(); a++)
		{
			if ((*a)->type == initial && (*a)->entry == stoi(te.substr(7, te.length()-7)))
				(*a)->codestart = codestart;
		}
	}
	else if (te.length() > 6 && te.substr(0,6) == stringify(always))
	{
		track_inial = always;
		for (auto a = process_list.begin(); a!= process_list.end(); a++)
		{
			if ((*a)->type == always && (*a)->entry == stoi(te.substr(6, te.length()-6)))
				(*a)->codestart = codestart;
		}
	}
	else if (te.length() > 6 && te.substr(0,6) == stringify(assign))
	{
		track_inial = assign;
		for (auto a = process_list.begin(); a!= process_list.end(); a++)
		{
			if ((*a)->type == assign && (*a)->entry == stoi(te.substr(6, te.length()-6)))
				(*a)->codestart = codestart;
		}
	}
	
}

/*void store_operations()
{
	struct operation op;
	op.op = myops;
	op.operands = parameters;
	parameters.clear();
	switch (track_inial)
	{
		case initial : init.insert_operation(op); break;
		case always : alw.insert_operation(op); break;
		case assign : ass[assign_process_no].push_back(op); break;
		default : break;
	}
}*/

void execute_function()
{
	proc myproc; 
	int t;
	switch (func_name)
	{
		case INIT_ARCH : 	init_arch(stoi(parameters[0]), parameters[1], parameters[2]) ;
		 					parameters.clear();
		 					break;
		case CREATE_REG : 	create_reg(intval);
					 		parameters.clear();
					 		break;
		case CREATE_WIRE : 	create_wire(intval);
					  		parameters.clear();
					  		break;
		case CREATE_INITIAL_PROC :	myproc = initial;
									t = stoi(parameters[0].substr(7, parameters[0].length()-7));
									create_initial_proc(myproc, t);
									parameters.clear();
									break;
		case CREATE_ALWAYS_PROC :	myproc = always;
									t = stoi(parameters[0].substr(6, parameters[0].length()-6));
									create_always_proc(myproc,t);
									parameters.clear();
									break;		
		case CREATE_ASSIGN_PROC :	myproc = assign;
									t = stoi(parameters[0].substr(6, parameters[0].length()-6));
									create_assign_proc(myproc, t);
									parameters.clear();
									break;
		case ADD_TRIGGERS : add_triggers (stoi(parameters[0]), parameters[1]);
							parameters.clear();
							break;
		case ADD_DRIVER : 	add_driver(stoi(parameters[0]), stoi(parameters[1]));
							parameters.clear();
							break;
		case PORT_CONNECTION : 	void* v1, *v2;
								v1 = (*(design[parameters[0]]))[parameters[1]];
								v2 = (*(design[parameters[3]]))[parameters[4]];
								add_port_connection(v1, v2, stoi(parameters[2]), stoi(parameters[5]));
								parameters.clear();
								break;
		case CREATE_MONITOR : 	for (int i =0; i < parameters.size(); i++)
								{
									mymonitor.monitor_vars.push_back(make_pair(globalv, stoi(parameters[i]))) ;
								}
								parameters.clear();
								break;
		default : 	parameters.clear();
					break;
	}
}
//STORE , MOVE, LOAD, OR, AND, NOT, UPDATE_DRIVER 

void print_arch ()
{
	for (auto a = design.begin(); a!= design.end(); a++)
	{
		cout << a->first << endl;
		for (auto b = a->second->begin(); b!= a->second->end(); b++)
		{
			cout << "\t" << b->first << endl;
		}
	}
	for (auto a = process_list.begin(); a != process_list.end(); a++)
	{
		cout << (*a)->type << " , entry = " << (*a)->entry<< " , code_start = "<< (*a)->codestart << " , base = ";
		printf("%ld\n", (*a)->base);
	}
	for (auto a = port_connection_list.begin();a != port_connection_list.end(); a++)
	{
		printf(" %ld, %d, %ld, %d \n", a->base1, a->offset1, a->base2, a->offset2);
	}

}

vector <string> split_str(string const &str, const char delim)  
{  
    vector <string> out; 
    stringstream s(str);  
      
    string s2, s3;  
    while (getline (s, s2, delim) )  
    {    
        stringstream ss2(s2);
        while (getline (ss2, s3, '\t') )
        	if (s3 != "") 
        	{
        		if (s3.length() > 5 && s3.substr(0,4) == "ARCH") s3 = s3.substr(5, s3.length()-5);
        		out.push_back(s3);
        	}
    }  
    return out;
}  

void second_read(string str)
{
	ifstream fp (str);
	string st;
	vector <int> inst;
	int position= 0;
	while(getline(fp, st))
	{
		position += st.length()+2;
		vector <string> vect;
		vect = split_str(st,' ');
		if (vect[0] == "->")
		{
			analyse_label(vect[1], position);
		}
	}
	fp.close();
}

/*int main ()
{
	return 0;
}*/