#include<iostream>
#include<cstdio>
#include<cstdlib>

using namespace std;

struct node

{
    std::string ip_addr;
    struct node *next1;
    struct node *next2;
    struct node *prev1;
    struct node *prev2;
};

//keeping count of nodes
int count = 1;

class virtual_ring 

{
	public: 
		struct node* introducer;
		node * create_node(std::string);
		void display();
		void insert();

		virtual_ring()
		{
			introducer = new node();
			introducer->ip_addr = "192.168.1.1"; //add introducer's IP address here
			introducer->next1=NULL;
			introducer->next2=NULL;
			introducer->prev1=NULL;
			introducer->prev2=NULL;

		}





};

int main()
{
	int choice;
	virtual_ring ring;

	while(1)
	{
		cout << "1 for join \n";
		cout << "2 for display \n";
		cout << "3 foe exit \n";
		cin >> choice;
		switch(choice)
		{
			case 1:
			ring.insert();
			break;
			case 2:
			ring.display();
			break;
			case 3:
			exit(1);

		}
 	}
 	
}

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

void virtual_ring::insert()
{	count +=1;
	std::string ip_addr_value;
	cout << "Enter IP address of joining node";
	cin >> ip_addr_value;
	struct node *temp;
	temp = create_node (ip_addr_value);
	if (count==2)
	{
		introducer->next1 = temp;
		introducer->prev1 = temp;
		temp->next1 = introducer;
		temp->prev1 = introducer;
		temp->next2 = NULL;
		temp->prev2 = NULL;
		//cout << "coount == 2";
	}
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

}

void virtual_ring::display()
{
	int i;
	struct node *s;
	s=introducer;
	for(i=1;i<=count;i++)
	{
		cout << s->ip_addr << "\tn1:" << s->next1->ip_addr << "\tn2:" << s->next2->ip_addr << "\tp1:" << s->prev1->ip_addr << "\tp2:" << s->prev2->ip_addr << "\n";
		s=s->next1;
	}
		

}