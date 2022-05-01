#include"types.h"
#include"stat.h"
#include "user.h"
#include "signal.h"
#include<stddef.h>

void fun1(int sig_no){
	printf(1,"Function1 is working %d\n",sig_no);
}
void fun2(int sig_no){
	printf(1,"Function2 is working %d\n",sig_no);
}


int main()
{
	int pid;
	/*
	printf(1,"Test 1: Sigaction test\n ");
	pid = fork();
	if(pid == 0){
		struct sigaction s1;
		s1.sa_handler = &fun1;
		sigaction(4,&s1,NULL);
		pid = getpid();
		printf(1,"child pid = %d",pid);
		while(1);
	}
	else{	
		sleep(200);
		kill(pid,4);
		sleep(200);
		kill(pid,9); // kill the child process 
		wait();
		printf(1,"Test 1 pass\n");
		exit();
	}*/
	
	//printf(1,"\n\n");
	/*
	printf(1,"Test 2: User handler and sig mask test\n");
	pid = fork();
		
	if(pid == 0){
		pid = getpid();
		struct sigaction s1;
		struct sigaction s2;
		sigprocmask(96); // mask the signals 
		s1.sa_handler = &fun1;
		s2.sa_handler = &fun2;
		sigaction(5,&s1,NULL);
		sigaction(6,&s2,NULL);
		while(1);

	}else{
		sleep(200);
		kill(pid,5);
		kill(pid,6);
		sleep(200);
		kill(pid,9); // kill the child process 
		wait();
		printf(1,"Test 2 pass\n");
		exit();
	}*/

	printf(1,"Test 3: SIGSTOP or pause and SIGCONT\n");
        pid = fork();
                
        if(pid == 0){
                pid = getpid();
                while(1);

        }else{
                sleep(200);
                kill(pid,19); // SIGSTOP
		printf(1,"child process stop\n");
                kill(pid,17); //SIGCONT
		printf(1,"child process continue\n");
                sleep(200);
                kill(pid,9); // kill the child process 
                wait();
                printf(1,"Test 3 pass\n");
                exit();
	}
}
