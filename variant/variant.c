#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int is_number_chek(char *number)
{
	for(int i = 0; i < strlen(number); i++)
	{
		if(number[i] < '0' || number[i] > '9')
		{
			return 0;
		}
	}
	
	return 1;
}

int main(int arg_amount, char *args[])
{
	int isHaveError = 0;
	if (arg_amount != 3)
	{
		fputs("ОШИБКА: неверное количество переданных параметров\n", stderr);
		isHaveError = 1;
	}
	
	int phone_number = 0;
	int labs_amount = 0;
		
	if(is_number_chek(args[1]))
	{
		phone_number = atoi(args[1]);
	}
	else
	{
		fputs("ОШИБКА: неверный тип 1-го параметра: номер телефона\n", stderr);
		isHaveError = 1;
	}
	
	if(strlen(args[1]) != 7)
	{
		fputs("ОШИБКА: неправильная длинна 1-го параметра: номер телефона должен содержать 7 символов\n", stderr);
		isHaveError = 1;
	}
	
	if(is_number_chek(args[2]))
	{
		labs_amount = atoi(args[2]);
	}
	else
	{
		fputs("ОШИБКА: неверный тип 2-го параметра: количество вариантов\n", stderr);
		isHaveError = 1;
	}
	
	if(!isHaveError)
	{	
		printf("Номер вашего телефона: %d\n",phone_number);
		printf("Количество вариантов: %d\n",labs_amount);
		int var_number = phone_number % labs_amount + 1;
	 	printf("Номер вашего варианта: %d\n", var_number);
	}
	
	return 0;
}
