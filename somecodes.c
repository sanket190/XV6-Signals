int sys_signal(void){
	signo = argint(...);
	void *ptr = argptr(...);
	sighandler_t sh = (sighandler_t*) ptr;
	note_handler(signo,sh);
}

note_handler()
{
	currproc = writeCodeForCurrProc();
	currproc -> handlers[signo] = sh;
}


// Per-process state
struct proc {
  uint sz;                     // Size of process memory (bytes)
  pde_t* pgdir;                // Page table
  char *kstack;                // Bottom of kernel stack for this process
  enum procstate state;        // Process state
  int pid;                     // Process ID
  struct proc *parent;         // Parent process
  struct trapframe *tf;        // Trap frame for current syscall
  struct context *context;     // swtch() here to run process
  void *chan;                  // If non-zero, sleeping on chan
  int killed;                  // If non-zero, have been killed
  struct file *ofile[NOFILE];  // Open files
  struct inode *cwd;           // Current directory
  char name[16];               // Process name (debugging)
  sighandler_t handlers[NSIGS]; // Rushikesh: Array of sighandlers
  int pending[NSIGS];           // Rushikesh: Array of pending signals
};


int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  argint(...);
  return kill(pid, signo);
}


/*
 * originally kill only set killed = 1 and killed gets checked at every operation
 * [200~0 proc.h    <global>     48 int killed;
 * 1 console.c consoleread 246 if(myproc()->killed){
 * 2 pipe.c    pipewrite    86 if(p->readopen == 0 || myproc()->killed){
 * 3 pipe.c    piperead    107 if(myproc()->killed){
 * 4 proc.c    wait        296 p->killed = 0;
 * 5 proc.c    wait        304 if(!havekids || curproc->killed){
 * 6 proc.c    kill        492 p->killed = 1;
 * 7 sysproc.c sys_sleep    70 if(myproc()->killed){
 * 8 trap.c    trap         40 if(myproc()->killed)
 * 9 trap.c    trap         44 if(myproc()->killed)
 * a trap.c    trap         94 myproc()->killed = 1;
 * b trap.c    trap        100 if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
 * c trap.c    trap        110 if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
 * */



int
kill(int pid, int signo)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
	if(signo == SIGKILL || signo == SIGINT)
      		p->killed = 1;
	else
		p->pending[signo] = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}


/* Now that we got pending signals the task is how to handle the pending signals  */
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;

  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state != RUNNABLE)
        continue;

      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      c->proc = p;
      // Switch TSS and h/w page table to correspond to process p.
      switchuvm(p);
      p->state = RUNNING;

      // Rushikesh: swtch(old, new)
      swtch(&(c->scheduler), p->context);
      // Rushikesh: switch to kernel table
      // Switch h/w page table register to the kernel-only page table,
      // for when no process is running.
      switchkvm();

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      c->proc = 0;
    }
    release(&ptable.lock);

  }
}


/* Process goes from scheduler to sched */
void
sched(void)
{
  int intena;
  struct proc *p = myproc();

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(mycpu()->ncli != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = mycpu()->intena;
  swtch(&p->context, mycpu()->scheduler);
  mycpu()->intena = intena;
}




/* Code goes to where sched is called i.e. yield, exit or sleep */

// Sleep --> Can be a Place to invoke signal handler

void
sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();

  if(p == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }
  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched();

  /* Invoke Signal Handler Here */

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}


void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  myproc()->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

/* Last lines of trap() */
  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running
  // until it gets to the regular system call return.)
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(myproc() && myproc()->state == RUNNING &&
     tf->trapno == T_IRQ0+IRQ_TIMER)
    yield();
    // Rushikesh: Do something here to ensure that the process starts in sigHandler
    if(proc -> contains a pending signal){
	remember the trapframe into TRFME
    	change the trapframe! to make process run in signal handler
	change the eip in trapframe (to a minimum, you may have to change some other things)
	also ensure that the sighandler returns into sys_sigreturn();	
    }

  // Check if the process has been killed since we yielded
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();
}



/* Process never calls signals and never sets handlers */
/* Therefore we need to set all the handlers to their default code when a process is created */

int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();

  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Copy process state from proc.
  if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = curproc->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));

  pid = np->pid;

  acquire(&ptable.lock);

  np->state = RUNNABLE;

  release(&ptable.lock);
	
  // Set the signal and signal handlers
  for (int i = 0 ; i < NSIGS ; i++){
  	proc->sh[i] = default-function();
	// Should default-function be a part of kernel code or process code?
  }
  return pid;
}

// Code Below fork()
int defhandler1(void){}
int defhandler2(void){}
int defhandler3(void){}
int defhandler4(void){}


/* What will the signal handler return ? */
/* It will return to the process */
/* How to achive it? */
int sys_sigreturn(void)
{
	ensure that you start from the next line of user code using the contents saved in TRFME;
	return from trap again;
}

/* Note: for eip --> change the assembly code */
