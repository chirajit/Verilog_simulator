#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
using namespace std;

class valueholder 
{
	protected :
	string name;
	char val, temp;
	public :
	vector <pair<string, void *> > triggers;
	valueholder (string var_name) {	name = var_name;}
	char value(){return val;}
	char* valptr() {return &val;}
	string var_name(){return name;}
	void update_from_temp(){val = temp;}
	void update_temp(char ch) {temp = ch;}
	void print()
	{
		cout << "name = " << name << "val = " << val << "triggers = ";
		for (auto a = triggers.begin(); a != triggers.end(); a++) cout << a->first << " , " ;
		cout << endl;
	}
};

class wire : public valueholder
{
	set <char*> drivers;
	public :
	wire (string name, char v = 'z', char t = 'z') : valueholder (name)
	{
		val  = v ; temp = t ;
	}

	void update_wire()
	{
		char ch = resolution();
		val = ch;
	}

	void add_driver (char * c)
	{
		drivers.insert(c);
		char ch = resolution();
		val  = ch;
	}


	char resolution () 
	{
		char ch ;
		auto c = drivers.begin();
		if (c == drivers.end()) return val;
		ch = **c;
		for (c = drivers.begin(); c != drivers.end(); c++)
		{
			if(**c != 'z') 
			{
				if (ch != **c)
				{
					ch = 'x'; 
					break;
				}
			} 
		}
		return ch;
	}
};

class reg : public valueholder
{
	public : 
	int display_no = 0;
	void* global_display_var = NULL;
	reg (string var_name, char v = 'x', char t = 'x'): valueholder (var_name)
	{
		val  = v ; temp = t ;
	}

	void update_val(char ch)
	{
		if (ch != 'z') val= ch;	
		else val = 'x';
	}

};

/*int main()
{
	reg r("reg1"), r2("reg2"), r3("reg4");
	r.print(); r2.print(); r3.print();
	initial init ;
	init.add_event(&r,'1');
	init.add_event(&r2,'1');
	init.update_time(10);
	init.add_event(&r,'1');
	init.update_time(5);
	init.add_event(&r,'1');
	init.shift_next();
	init.add_event(&r3,'0');
	init.update_time(0);
	init.add_event(&r,'1');
	init.print();
	init.execute_all();
	cout << endl;
	r.print(); r2.print(); r3.print();
	init.print();
}*/