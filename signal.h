struct sigaction
{
  void (*sa_handler)(int);
  uint sigmask;
};

#define NULL ((void*)0)
