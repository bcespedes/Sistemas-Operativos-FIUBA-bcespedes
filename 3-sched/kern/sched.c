#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>

#define MIN_PRIORITY 0
#define BOOST_INTERVAL 20
#define MAX_EXECUTIONS_SLICE 3

void sched_halt(void);

struct SchedStats {
	int history[NENV];
	int scheduler_calls;
	int process_execution_count[NENV];
	int history_index;
	int total_boots;
};

struct SchedStats sched_stats = { 0 };

void
show_sched_stats(void)
{
	cprintf("Scheduler Stadistics:\n");
	cprintf("Number of calls to Scheculer: %d\n", sched_stats.scheduler_calls);

	cprintf("Process's history:\n");
	for (int i = 0; i < sched_stats.history_index; i++) {
		cprintf("Env: %d\n", sched_stats.history[i]);
	}

	cprintf("Process's executed:\n");
	for (int i = 0; i < NENV; i++) {
		int count = sched_stats.process_execution_count[i];
		if (count > 0) {
			cprintf("Env %d executed %d times\n", i, count);
		}
	}

	cprintf("Boots's count: %d\n", sched_stats.total_boots);
	cprintf("\n");
}

void
boost_all_envs(void)
{
	for (int i = 0; i < NENV; i++) {
		if (envs[i].env_status != ENV_FREE &&
		    envs[i].env_priority < MAX_PRIORITY) {
			envs[i].env_priority++;
		}
	}
}

void
add_counts_to_sched(int index)
{
	int index_sched = ENVX(envs[index].env_id);
	sched_stats.process_execution_count[index_sched]++;
	sched_stats.history[sched_stats.history_index++] =
			index_sched;
}

// Función principal del scheduler
void
sched_yield(void)
{
	sched_stats.scheduler_calls++;
	int start = (curenv) ? ENVX(curenv->env_id) : 0;

#ifdef SCHED_ROUND_ROBIN
	// Implement simple round-robin scheduling.
	//
	// Search through 'envs' for an ENV_RUNNABLE environment in
	// circular fashion starting just after the env this CPU was
	// last running. Switch to the first such environment found.
	//
	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	//
	// Never choose an environment that's currently running on
	// another CPU (env_status == ENV_RUNNING). If there are
	// no runnable environments, simply drop through to the code
	// below to halt the cpu.
	for (int i = 0; i < NENV; i++) {
		int index = (start + i) % NENV;
		if (envs[index].env_status == ENV_RUNNABLE) {
			add_counts_to_sched(index);
			env_run(&envs[index]);
		}
	}
#endif

#ifdef SCHED_PRIORITIES
	// Implement simple priorities scheduling.
	//
	// Environments now have a "priority" so it must be consider
	// when the selection is performed.
	//
	// Be careful to not fall in "starvation" such that only one
	// environment is selected and run every time.

	// Your code here - Priorities
	if (sched_stats.scheduler_calls >= BOOST_INTERVAL) {
		boost_all_envs();
		sched_stats.total_boots++;
	}

	struct Env *env_best_priority = NULL;
	int best_priority = MIN_PRIORITY;

	for (int i = 0; i < NENV; i++) {
		int index = (start + i) % NENV;
		struct Env *env = &envs[index];
		if (env->env_status == ENV_RUNNABLE) {
			if (!env_best_priority ||
			    env->env_priority > best_priority) {
				env_best_priority = env;
				best_priority = env->env_priority;
			}
		}
	}

	if (env_best_priority) {
		add_counts_to_sched(env_best_priority->env_id);
		if (sched_stats.process_execution_count[env_best_priority->env_id] %
		            MAX_EXECUTIONS_SLICE ==
		    0) {
			if (env_best_priority->env_priority > MIN_PRIORITY) {
				env_best_priority->env_priority--;
			}
		}

		env_run(env_best_priority);
	}

#endif

	if (curenv && curenv->env_status == ENV_RUNNING) {
		env_run(curenv);
	}

	sched_halt();
}


// Halt this CPU when there is nothing to do. Wait until the
// timer interrupt wakes it up. This function never returns.
//

void
sched_halt(void)
{
	int i;

	// For debugging and testing purposes, if there are no runnable
	// environments in the system, then drop into the kernel monitor.
	for (i = 0; i < NENV; i++) {
		if ((envs[i].env_status == ENV_RUNNABLE ||
		     envs[i].env_status == ENV_RUNNING ||
		     envs[i].env_status == ENV_DYING))
			break;
	}
	if (i == NENV) {
		cprintf("No runnable environments in the system!\n");
		show_sched_stats();
		while (1)
			monitor(NULL);
	}

	// Mark that no environment is running on this CPU
	curenv = NULL;
	lcr3(PADDR(kern_pgdir));

	// Mark that this CPU is in the HALT state, so that when
	// timer interupts come in, we know we should re-acquire the
	// big kernel lock
	xchg(&thiscpu->cpu_status, CPU_HALTED);

	// Release the big kernel lock as if we were "leaving" the kernel
	unlock_kernel();

	// Once the scheduler has finishied it's work, print statistics on
	// performance. Your code here

	// Reset stack pointer, enable interrupts and then halt.
	asm volatile("movl $0, %%ebp\n"
	             "movl %0, %%esp\n"
	             "pushl $0\n"
	             "pushl $0\n"
	             "sti\n"
	             "1:\n"
	             "hlt\n"
	             "jmp 1b\n"
	             :
	             : "a"(thiscpu->cpu_ts.ts_esp0));
}
