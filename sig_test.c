#include "types.h"
#include "stat.h"
#include "user.h"
#include "param.h"

void fun1(sig_no){
	printf(1,"Function1 is working %d\n",sig_no);
}
void fun2(sig_no){
	printf(1,"Function2 is working %d\n",sig_no);
}


int main()
{
	printf(1,"Test 1. SIGKILL\n");
	if(fork() == 0){
		printf(1,"In child Process");
		sigaction(4,&fun1,NULL);
		sigaction(5,&fun2,NULL);
		printf(1,"sending kill singal, only one function should run \n");
		kill(getpid(),4);
		kill(getpid(),5);
		kill(getpid(), SIGKILL);
		sleep(5);
		printf(1,"Test 2 Failed , sigkill cant be blocked\n");
		exit();
	}
}
