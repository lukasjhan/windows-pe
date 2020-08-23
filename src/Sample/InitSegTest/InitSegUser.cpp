#include "stdio.h"

class MyUser
{
public:
	MyUser()
	{
		printf("MyUser()\n");
	}
	~MyUser()
	{
		printf("~MyUser()\n");
	}
};
#pragma init_seg(user)
MyUser G_USER;
