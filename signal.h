struct sigaction
{
  void (*sa_handler)(int);
  int sigmask;
};

#define NULL ((void*)0)
