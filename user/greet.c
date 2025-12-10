#include <stdio.h>

int main () {
	printf("Enter your name: ");
	char name[50];
	getline(name, sizeof(name));
	printf("Hello, %s!\n", name);
	return 0;
}