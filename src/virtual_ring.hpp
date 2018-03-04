struct node

{
    std::string ip_addr;
    struct node *next1;
    struct node *next2;
    struct node *prev1;
    struct node *prev2;
};

//keeping count of nodes


class virtual_ring 

{
	public: 
		struct node* introducer;
		node* create_node(std::string);
		void display();
		node* insert(std::string ip_addr_value);
		void leave(std::string ip);
		std::string put_file(std::string file_name);
		int count;
		int introducer_exists;
		virtual_ring(std::string ip)
		{
			introducer = new node();
			introducer->ip_addr = ip; //add introducer's IP address here
			introducer->next1=NULL;
			introducer->next2=NULL;
			introducer->prev1=NULL;
			introducer->prev2=NULL;
			count = 1;
			introducer_exists = 1;
		}





};
