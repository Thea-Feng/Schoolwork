#include <linux/err.h>
#include <linux/fs.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/kmod.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/pid.h>
#include <linux/printk.h>
#include <linux/sched.h>
#include <linux/slab.h>

// About signal value
// refer to
// https://mzhan017.blog.csdn.net/article/details/125153030?spm=1001.2101.3001.6661.1&utm_medium=distribute.pc_relevant_t0.none-task-blog-2%7Edefault%7ECTRLIST%7EPayColumn-1-125153030-blog-53150890.pc_relevant_aa_2&depth_1-utm_source=distribute.pc_relevant_t0.none-task-blog-2%7Edefault%7ECTRLIST%7EPayColumn-1-125153030-blog-53150890.pc_relevant_aa_2&utm_relevant_index=1
#define my_WEXITSTATUS(status) (((status) >> 8) && 0xff)
#define my_WIFEXITED(status) (((status)&0x7f) == 0)
#define my_WIFSIGNALED(status) (((signed char)(((status)&0x7f) + 1) >> 1) > 0)
#define my_WTERMSIG(status) ((status)&0x7f)
#define my_WIFSTOPPED(status) (((status)&0xff) == 0x7f)
#define my_WSTOPSIG(status) ((status)&0x7f)

MODULE_LICENSE("GPL");

extern pid_t kernel_clone(struct kernel_clone_args *args);
extern struct filename *getname_kernel(const char __user *filename);
struct wait_opts {
	enum pid_type wo_type;
	int wo_flags;
	struct pid *wo_pid;
	struct waitid_info *wo_info;
	int wo_stat;
	struct rusage *wo_rusage;
	wait_queue_entry_t child_wait;
	int notask_error;
};

extern long do_wait(struct wait_opts *wo);
extern int do_execve(struct filename *filename,
		     const char __user *const __user *__argv,
		     const char __user *const __user *__envp);
static struct task_struct *thr;
// implement fork function

int my_wait(pid_t pid)
{
	int status;
	// printk("THIS IS WAIT\n");
	struct wait_opts wo;
	struct pid *wo_pid = NULL;
	enum pid_type type;
	type = PIDTYPE_PID;
	wo_pid = find_get_pid(pid);
	// look up PID form hash table
	wo.wo_type = type;
	wo.wo_pid = wo_pid;
	wo.wo_flags = WEXITED | WUNTRACED;
	wo.wo_info = NULL;
	wo.wo_stat = status;
	wo.wo_rusage = NULL;
	do_wait(&wo);
	int a = wo.wo_stat;
	// printk("in status is %d %d\n", wo.wo_stat, a);
	// process exit status (signal)
	put_pid(wo_pid);
	return a;
}

int my_exec(void)
{
	int ret;
	const char path[] = "/tmp/test";

	struct filename *fileName = getname_kernel(path);

	printk("[program2] : child process");
	// printk("filename is %s\n", *fileName);
	ret = do_execve(fileName, NULL, NULL);
	// printk("ret is %d\n", ret);
	if (!ret)
		return 0;
	do_exit(ret);
}

int i;
int my_fork(void *argc)
{
	// set default sigaction for current process

	struct k_sigaction *k_action = &current->sighand->action[0];
	for (i = 0; i < _NSIG; i++) {
		k_action->sa.sa_handler = SIG_DFL;
		k_action->sa.sa_flags = 0;
		k_action->sa.sa_restorer = NULL;
		sigemptyset(&k_action->sa.sa_mask);
		k_action++;
	}
	i += 1;
	// printk("[program2] : This is %d round\n", i);
	/* fork a process using kernel_clone or kernel_thread */
	struct kernel_clone_args args;
	args.flags = SIGCHLD, args.exit_signal = SIGCHLD, args.stack_size = 0;
	args.parent_tid = NULL, args.child_tid = NULL;
	args.stack = (unsigned long)&my_exec;
	args.tls = 0;

	pid_t pid = kernel_clone(&args);
	printk("[program2] : The Child process has pid = %d\n", pid);
	printk("[program2] : This is the parent process, pid = %d\n",
	       (int)current->pid);
	/* execute a test program in child process */

	/* wait until child process terminates */
	int status;
	status = my_wait(pid);

	if (my_WIFEXITED(status)) {
		printk("[program2] : Parent receive SIGCHLD signal\n");
		printk("[program2] : Normal termination with EXIT STATUS = %d\n",
		       my_WEXITSTATUS(status));
	} else if (my_WIFSIGNALED(status)) {
		int sig_bit = my_WTERMSIG(status);
		// printk("sig_bit is %d\n", sig_bit);

		char *info, *SIG;
		switch (sig_bit) {
		case 6:
			SIG = "SIGABRT", info = "aborted";
			break;
		case 14:
			SIG = "SIGALRM", info = "terminated by alarm signal";
			break;
		case 7:
			SIG = "SIGBUS", info = "bus error";
			break;
		case 8:
			SIG = "SIGFPE", info = "floating point exception";
			break;
		case 1:
			SIG = "SIGHUP", info = "hang up";
			break;
		case 4:
			SIG = "SIGILL", info = "illegal instruction";
			break;
		case 2:
			SIG = "SIGINT", info = "interupted";
			break;
		case 9:
			SIG = "SIGKILL", info = "killed";
			break;
		case 13:
			SIG = "SIGPIPE", info = "fail to write to pipe";
			break;
		case 3:
			SIG = "SIGQUIT", info = "quited";
			break;
		case 11:
			SIG = "SIGSEGV", info = "segmentation fault ";
			break;
		case 15:
			SIG = "SIGTERM",
			info = "terminated by termination signal";
			break;
		case 5:
			SIG = "SIGTRAP", info = "terminated by trap signal";
			break;
		}
		printk("[program2] : Parent receive %s signal\n", SIG);

		printk("[program2] : CHILD EXECUTION FAILED: %d (%s)\n",
		       sig_bit, info);
	} else if (my_WIFSTOPPED(status)) {
		printk("[program2] : Parent receive SIGSTOP signal\n");
		printk("[program2] : CHILD PROCESS STOPPED\n");
	} else {
		printk("[program2] : CHILD PROCESS CONTINUED\n");
	}
	return 0;
}

static int __init program2_init(void)
{
	printk("------------------------------------[program2]-----------------------"
	       "--\n");
	printk("[program2] : module_init {Feng Yutong} {120090266}\n");
	printk("[program2] : module_init create kthread start\n");

	/* write your code here */

	/* create a kernel thread to run my_fork */
	thr = kthread_create(&my_fork, NULL, "Mythread");

	// if(!iS_ERR(thr)){
	printk("[program2] : module_init Kthread start\n");
	wake_up_process(thr);
	// }
	return 0;
}

static void __exit program2_exit(void)
{
	printk("[program2] : Module_exit\n");
}

module_init(program2_init);
module_exit(program2_exit);
