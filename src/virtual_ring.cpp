#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <boost/algorithm/string.hpp>
#include "virtual_ring.hpp"
#include <vector>


using namespace std;




node* virtual_ring::create_node(std::string ip_addr_val){

	struct node *temp;

    temp = new(struct node);

    temp->ip_addr = ip_addr_val;

    temp->next1 = NULL;

    temp->next2 = NULL;

    temp->prev1 = NULL;

    temp->prev2 = NULL;

    return temp;

}
void virtual_ring::leave(std::string ip)
{
	
	int i,flag=0;
	struct node *s;
	s=introducer;

	// case when the node leaving is the introducer

	if(ip==s->ip_addr)
	{	count--;
		introducer_exists = 0;
		introducer = s->next1;
		s->prev1->next1 = s->next1;
		s->prev1->next2 = s->next2;

		s->prev2->next2 = s->next1;
		
		s->next1->prev1=s->prev1;
		s->next1->prev2 = s->prev2;

		s->next2->prev2=s->prev1;
	
		

	}

	

	else if(ip!=s->ip_addr && count >2) {

		for(i=1;i<=count;i++)
	{
		if(s->ip_addr == ip )
		{	
			count -- ;
			//if(count == 	
			struct node *s2;
			s2 = s;
			cout << s->ip_addr+'\n';
			struct node *s1;
			s1 = s -> prev1;
			s1 ->next1 = s->next1;
			s1->next2 = s->next2;

			s1->prev1->next2 = s -> next1;

			s->next1->prev1 = s1;
			s->next1->prev2 = s1->prev1;

			s->next2->prev2 = s1;
			//s = s-> next1;

			s2->next1=NULL;
			s2->next2=NULL;
			s2->prev1=NULL;
			s2->prev2=NULL;
			break;

		}
		s = s-> next1;
		
	}

	}

}

std::string virtual_ring::put_file(std::string file_name)
{
	std::cout << "filename\t"<< file_name<<"\n";
	int Val = int(file_name.at(0)) -'a' + 37;

	std::cout << "value of first alphabet\n";
	std::cout << Val;	
	std::cout << "\n";
	
	node * temp = introducer;
	int Val1 = 1;
	int i=1;
	while(Val > Val1 and i<count)
	{
		std::string line(temp->ip_addr);
		vector<std::string> strs;
		boost::split(strs,line,boost::is_any_of("."));
		Val1 = atoi(strs[3].c_str()); 
		if(Val1==Val){
			return temp->ip_addr;
		}
		temp = temp ->next1;
		i++;
	}

	cout << temp->ip_addr;
	return temp->ip_addr;
}






node* virtual_ring::insert(std::string ip_addr_value)
{	
	node *itr = introducer;
	for(int i=0; i < count;++i){
		if(ip_addr_value.compare(itr->ip_addr) == 0)
			return itr;
		itr = itr->next1;
	}
	count +=1;
	node *temp;
	temp = create_node (ip_addr_value);
	
	std::string line(ip_addr_value);
	vector<std::string> strs;
	boost::split(strs,line,boost::is_any_of("."));
	int val = atoi(strs[3].c_str()); 
	std::cout << "The val of ip inserted \n";
	std::cout << val;
	std::cout << "\n";
	int i = 1;
	int val1 = 1;
	

	node *temporary = introducer;


	if (count==2)
	{
		introducer->next1 = temp;
		introducer->prev1 = temp;
		temp->next1 = introducer;
		temp->prev1 = introducer;
		temp->next2 = NULL;
		temp->prev2 = NULL;
		//cout << "coount == 2";
		return temp;
	}

	while(val1<val and i<count )
	{
		//std::cout << "val 1 is " << val1 << " val is " << val << std::endl;
		//std::cout << temporary->ip_addr << std::endl;
		temporary= temporary->next1;
		
		std::string line1(temporary->ip_addr);
		vector<std::string> strs1;
		boost::split(strs1,line1,boost::is_any_of("."));
		val1 = atoi(strs1[3].c_str()); 
		i++;

	}

	std::cout << temporary->ip_addr;
	std::cout << "\n";
	node *original_introducer = introducer;
	introducer = temporary->prev1;

	if(count == 3)
	{	

		struct node *temp1;
		temp1 = new(struct node);
		temp1 = introducer -> next1;
		introducer->next1 = temp;
		introducer->next2 = temp1;
		//introducer->prev1 = temp1;
		introducer->prev2 = temp;

		temp->next1 = temp1;
		temp->next2 = (temp->next1)->next1;
		temp->prev1=introducer;
		temp->prev2 = introducer->prev1;

		temp1->next2 = introducer->next1;
		temp1->prev1=temp;
		temp1->prev2=introducer;
		//cout << "count == 3";

	}
	if (count==4)
	{
		struct node *temp2;
		temp2 = new (struct node);
		temp2 = introducer->next1;
		introducer->next1 = temp;
		introducer->next2=temp2;

		temp->next1 = temp2;
		temp->next2 = temp2->next1;
		temp->prev1 = introducer;
		temp->prev2 = introducer->prev1;

		temp2->prev2 = temp2->prev1;
		temp2->prev1 = temp;

		struct node *temp3;
		temp3 = new (struct node);
		temp3 = introducer->prev1;
		temp3->prev2 = (temp3->prev1)->prev1;
		temp3 -> next2 = introducer->next1;
		//cout << "count == 4";



	}
	else if(count>4)
	{
		struct node *temp4;
		temp4 = new (struct node);
		temp4 = introducer->next1;
		introducer->next1 = temp;
		introducer->next2 = temp4;

		temp->next1 = temp4;
		temp->next2 = temp4->next1;
		temp->prev1 = introducer;
		temp->prev2 = introducer->prev1;

		temp4->prev1 = temp;
		temp4->prev2 = temp->prev1;

		temp4->next1->prev2 = temp;

		introducer->prev1->next2 = temp;




	}
	introducer = original_introducer;
	return temp;

}

void virtual_ring::display()
{
	int i;
	struct node *s;
	s=introducer;
	cout << "#######################################################\n";
	for(i=1;i<=count;i++)
	{
		
		cout << s->ip_addr ;
		if(s->next1 != NULL)
			cout  << "\tn1:" << s->next1->ip_addr ;
		if(s->next2 != NULL)
			cout << "\tn2:" << s->next2->ip_addr ;
		if(s->prev1 != NULL)
			cout << "\tp1:" << s->prev1->ip_addr ;
		if(s->prev2 != NULL)
			cout << "\tp2:" << s->prev2->ip_addr;
		cout << endl;
		s=s->next1;
	}
	cout << "#######################################################\n";

}



