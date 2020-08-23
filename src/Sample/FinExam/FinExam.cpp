#include "stdafx.h"

int FinExam0()
{
	printf("FinExam0 ==========================================\n");
	__try
	{
		printf("\tFinExam0::__try block executed!!!\n");
	}
	__finally
	{
		printf("\tFinExam0::__finally block executed!!!\n");
	}

	printf("\tFinExam0 terminates!!!\n");
	return 0;
}

int FinExam1()
{
	printf("FinExam1 ==========================================\n");
	__try
	{
		printf("\tFinExam1::__try block executed!!!\n");
		return 1;
	}
	__finally
	{
		printf("\tFinExam1::__finally block executed!!!\n");
	}
	
	printf("\tFinExam1 terminates!!!\n");
	return 0;
}

int FinExam2()
{
	printf("FinExam2 ==========================================\n");
	__try
	{
		printf("\tFinExam2::__try block executed!!!\n");
		goto $LABEL_RET;
	}
	__finally
	{
		printf("\tFinExam2::__finally block executed!!!\n");
	}

$LABEL_RET:
	printf("\tFinExam2 terminates!!!\n");
	return 0;
}

int FinExam3()
{
	printf("FinExam3 ==========================================\n");
	int val = 0;

	while (val < 10)
	{
		__try
		{
			printf("\tFinExam3::__try block executed, val=%d\n", val);
			if (val == 2)
				continue;
			if (val == 3)
				break;
		}
		__finally
		{
			val++;
			printf("\tFinExam3::__finally block executed, val=%d\n", val);
		}

		val++;
		printf("\tFinExam3::end of while block, val=%d\n", val);
	}

	val++;
	printf("\tFinExam3 terminates, val=%d\n", val);
	return val;
}

int FinExam4()
{
	printf("FinExam4 ==========================================\n");
	__try
	{
		printf("\tFinExam4::__try block executed!!!\n");
		return 1;
	}
	__finally
	{
		printf("\tFinExam4::__finally block executed!!!\n");
		return 2;
	}

	printf("\tFinExam4 terminates!!!\n");
	return 0;
}

int FinExam5()
{
	printf("FinExam5 ==========================================\n");
	__try
	{
		printf("\tFinExam5::__try block executed!!!\n");
		int d = 0;
		int r = 3 / d;
	}
	__finally
	{
		printf("\tFinExam5::__finally block executed!!!\n");
	}

	printf("\tFinExam5 terminates!!!\n");
	return 0;
}

void _tmain(int argc, TCHAR* argv[])
{
	FinExam4();
	//FinExam2();
	//FinExam3();
	//FinExam4();
	//int r = FinExam5();
	//printf("FinExam5 returns %d\n", r);
}
