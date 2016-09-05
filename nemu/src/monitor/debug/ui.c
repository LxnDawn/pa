#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>


union ad{
char *addr_p;
uint32_t addr_u;
};
void cpu_exec(uint32_t);

static int cmd_si(char *arg)
{
uint32_t n=atoi(arg);
cpu_exec(n);
return 0;
}
extern CPU_state cpu;
/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}
static int cmd_info(char *args){
	if(*args=='r'){
	  printf("EAX:  0x%x\t%d\n",cpu.gpr[0]._32,cpu.gpr[0]._32);
	  printf("ECX:  0x%x\t%d\n",cpu.gpr[1]._32,cpu.gpr[1]._32);
	  printf("EDX:  0x%x\t%d\n",cpu.gpr[2]._32,cpu.gpr[2]._32);
	  printf("EBX:  0x%x\t%d\n",cpu.gpr[3]._32,cpu.gpr[3]._32);
	  printf("ESP:  0x%x\n",cpu.gpr[4]._32);
	  printf("EBP:  0x%x\n",cpu.gpr[5]._32);
	  printf("ESI:  0x%x\n",cpu.gpr[6]._32);
	  printf("EDI:  0x%x\n",cpu.gpr[7]._32);
	  printf("EIP:  0x%x\n",cpu.eip);
		}
	return 0;
}


static int cmd_x(char *args)
{
   char* arg1=strtok(args," ");
   char* arg2=strtok(args," ");
   if(arg2==NULL ||arg1==NULL)printf("Wrong usage!Type help for help\n");
   int k=atoi(arg1);
   union ad addr;
   addr.addr_u=strtol(arg2,NULL,16);
   printf("0x%d: ",addr.addr_u);
   int i=1;
   for(;i<=k;i++){
    printf("%d\t",*(addr.addr_p));
    if(i%4==0)printf(" ");
    if(i%16==0)printf("\n\t");
   }
  return 0;
}
static int cmd_q(char *args) {
	return -1;
}

static int cmd_help(char *args);

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
	{"si","Excute n instructions.Usage: si [n]",cmd_si},
	{"info","Print the state of the running program",cmd_info},
	{"x","Scanf the address. Usage x [n]",cmd_x},
	/* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}