
// Test case for C++ exceptions: generating .eh_frame info.
#include <iostream>

class classa {
public:
	classa();
	~classa();
	char *access() ;
	void insert(char *s);
private:
	char *data;
};

classa::classa()
{ data = new char[12];
  data[0] = 'a';
  data[1] = 0;
}
classa::~classa() 
{ 
	delete [] data; 
}

char *
classa::access() 
{ 
	return data; 
}

void
classa::insert(char *s)
{ 
	strncpy(data,s,11); 
 	data[11] = 0; 
}

int main()
{

	classa a;

	try {
	    a.insert("abc");
	    std::cout<<"pass" <<std::endl;
	}
	catch ( ... )
	{
	    std::cout<<"fail" <<std::endl;
	}

        return 0;
}
