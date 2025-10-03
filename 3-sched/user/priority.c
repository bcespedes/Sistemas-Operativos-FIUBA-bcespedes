#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	int pid = fork();
	cprintf("Process priority: %d\n", sys_get_priority());
	int priority = sys_set_priority(3);

    if (sys_get_priority() == priority) {
        cprintf("Process have new priority: %d\n", priority);
    } else {
        cprintf("Error priority\n");
        return;
    }

	if (pid == 0) {
		if (sys_get_priority() == 3) {
			cprintf("Child process have same priority than father\n");
		} else {
			cprintf("Error child priority\n");
		}
	} else {
		if (sys_get_priority() == 3) {
            cprintf("Parent priorty is ok\n");
        } else {
            cprintf("Error parent priorty\n");
        }
	}
}