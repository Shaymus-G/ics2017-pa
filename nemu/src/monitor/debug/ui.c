#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

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

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args){
  int steps=1;
  if(args!=NULL){
    sscanf(args,"%d",&steps);
  }
  cpu_exec(steps);
  return 0;
}

static int cmd_info(char *args){
  if(args==NULL){
    printf("Missing argument for info!\n");
    return 0;
  }
  if(strcmp(args,"r")==0){
    printf("eax\t0x%08x\t%d\n",cpu.eax,cpu.eax);
    printf("ecx\t0x%08x\t%d\n",cpu.ecx,cpu.ecx);
    printf("edx\t0x%08x\t%d\n",cpu.edx,cpu.edx);
    printf("ebx\t0x%08x\t%d\n",cpu.ebx,cpu.ebx);
    printf("esp\t0x%08x\t%d\n",cpu.esp,cpu.esp);
    printf("ebp\t0x%08x\t%d\n",cpu.ebp,cpu.ebp);
    printf("esi\t0x%08x\t%d\n",cpu.esi,cpu.esi);
    printf("edi\t0x%08x\t%d\n",cpu.edi,cpu.edi);
    printf("eip\t0x%08x\t%d\n",cpu.eip,cpu.eip);
  }
  else{
    printf("Unknown argument '%s' for info!\n",args);
  }
  return 0;
}

static int cmd_x(char *args){
  if(args==NULL){
    printf("Missing arguments for x!\n");
    return 0;
  }
  char *arg1=strtok(args," ");
  if(arg1==NULL) return 0;
  int n=atoi(arg1);
  char *arg2=arg1+strlen(arg1)+1;
  if(arg2==NULL) return 0;
  bool success=false;
  vaddr_t addr=expr(arg2,&success);
  if(!success) return 0;
  for(int i=0;i<n;i++){
    uint32_t data=vaddr_read(addr+i*4,4);
    printf("0x%08x:\t0x%08x\n",addr+i*4,data);
  }
  return 0;
}

static int cmd_p(char *args){
  if(args==NULL){
    printf("Missing expression!\n");
    return 0;
  }
  bool success=true;
  uint32_t res=expr(args,&success);
  if(success){
    printf("%u\n",res);
  }
  else{
    printf("Bad expression!\n");
  }
  return 0;
}

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Make the program execute one instruction at a time and then pause.If N is not specified,it defaults to 1", cmd_si },
  { "info", "Print register status;Print monitoring point information", cmd_info },
  { "x", "Calculate the value of the expression EXPR,use the result as the starting memory address,and output N consecutive 4-bite values in hexadecimal format", cmd_x},
  { "p", "Calculate the value of the expression EXPR", cmd_p}


  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
