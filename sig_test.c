#include "types.h"
#include "stat.h"
#include "user.h"
#include "signal.h"
#include "param.h"
#include <stddef.h>

void fun1(int sig_no)
{
	printf(1, "inside user defined handler: %d\n", sig_no);
}
void fun2(int sig_no)
{
	printf(1, "\nfunction2 is working -->  %d\n", sig_no);
}

void sigaction_test()
{
	int pid;
	printf(1, "sigaction test\n");
	pid = fork();
	if (pid == 0)
	{
		struct sigaction s1;
		s1.sa_handler = &fun1;
		sigaction(4, &s1, NULL);
		pid = getpid();
		printf(1, "child pid = %d\n", pid);
		while (1);
	}
	else
	{
		sleep(200);
		kill(pid, 4);
		sleep(200);
		kill(pid, 9);
		wait();
		printf(1, "sigaction OK\n");
	}
}

void sigmask_test()
{
	int pid;
	printf(1, "sigmask test\n");
	pid = fork();

	if (pid == 0)
	{
		pid = getpid();
		struct sigaction s1;
		struct sigaction s2;
		sigprocmask(96);
		s1.sa_handler = &fun1;
		s2.sa_handler = &fun2;
		sigaction(5, &s1, NULL);
		sigaction(6, &s2, NULL);
		while (1);
	}
	else
	{
		sleep(200);
		kill(pid, 5);
		kill(pid, 6);
		sleep(200);
		kill(pid, 9);
		wait();
		printf(1, "sigmask OK\n");
	}
}

void default_handler_test()
{
	int pid;
	printf(1, "default handler test\n");
	pid = fork();
	if (pid == 0)
	{
		pid = getpid();
		while (1);
	}
	else
	{
		sleep(200);
		kill(pid, 19); // SIGSTOP
		printf(1, "child process stopped\n");
		kill(pid, 17); // SIGCONT
		printf(1, "child process started\n");
		sleep(200);
		kill(pid, 9);
		wait();
		printf(1, "default handler OK\n");
                exit();
	}
}

void sigkill_test()
{
  int pid;
  printf(1,"sigkill and sigmask test\n");
  pid = fork();   
  if(pid == 0){
    struct sigaction act = {fun1, 1 << SIGKILL};
    struct sigaction act2 = {fun2, 1 << SIGKILL};
    // Trying to mask SIGKILL
    sigaction(4, &act, NULL);
    sigaction(5, &act2, NULL);
    int mask_4_kill = (1<<4) | (1<<SIGKILL);
    sigprocmask(mask_4_kill);
    printf(1, "sending kill signal OK\n");
    kill(getpid(), 4);
    kill(getpid(), 5);
    kill(getpid(), SIGKILL);
    sleep(5);
    printf(1, "sigkill-sigmask FAILED: SIGKILL can't be blocked!\n");
  }
  else
  {
    wait();
  }
}

void sigkill_sigaction_test()
{
  int pid;
  printf(1,"sigkill sigaction test\n");
  pid = fork();
  if(pid == 0){
    int num = 2;
    struct sigaction act = {fun1, 0};
    struct sigaction act2 = {fun2, 0};
    if((sigaction(SIGSTOP, &act, NULL)) < 0)
      num--;
    if((sigaction(SIGKILL, &act2, NULL)) < 0)
      num--;
    if(!num){
      printf(1,"sigkill sigaction OK\n");
    }
    else{
      printf(1,"sigkill sigaction FAILED\n");
    }
  }
  else
  {
    wait();
  }
}

int main()
{
	sigaction_test();
	sigmask_test();
	sigkill_test();
        sigkill_sigaction_test();
	default_handler_test();
}
