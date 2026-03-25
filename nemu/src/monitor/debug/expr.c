#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256,
  TK_EQ,

  /* TODO: Add more token types */

  TK_NUM,
  TK_HEX,
  TK_REG,
  TK_NEQ,
  TK_AND,
  TK_OR,
  TK_DEREF,
  TK_NEG
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
  {"!=", TK_NEQ},			// 
  {"\\-", '-'},		// subtract
  {"\\*", '*'},		// multiply
  {"/", '/'},		// divide
  {"\\(", '('},		// left bracket
  {"\\)", ')'},		// right bracket

  {"[xX][0-9a-fA-F]+", TK_HEX}, //hexadecimal
  {"[0-9]+", TK_NUM},		//decimal integer
  {"\\$[a-zA-Z]+", TK_REG}	//register
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
	  case TK_NOTYPE:
	    break;
          default:
	    tokens[nr_token].type=rules[i].token_type;
	    if(rules[i].token_type==TK_NUM ||
	       rules[i].token_type==TK_HEX ||
	       rules[i].token_type==TK_REG){
	      assert(substr_len<32);
	      strncpy(tokens[nr_token].str, substr_start, substr_len);
	      tokens[nr_token].str[substr_len]='\0';
	    }
	    nr_token++;
	    assert(nr_token<sizeof(tokens)/sizeof(tokens[0]));
	    break;
	}

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

static bool check_parentheses(int p,int q){
  if(tokens[p].type!='('||tokens[q].type!=')'){
    return false;
  }
  int count=0;
  for(int i=p;i<=q;i++){
    if(tokens[i].type=='('){
      count++;
    }
    else if(tokens[i].type==')'){
      count--;
    }
    if(count==0&&i<q){
      return false;
    }
  }
  return true;
}

static int get_precedence(int type){
  switch(type){
    case '*': case '/': return 1;
    case '+': case '-': return 2;
    case TK_EQ: case TK_NEQ: return 3;
    case TK_AND: return 4;
    case TK_OR: return 5;
    default: return 0;
  }
}

static int find_dominant_operator(int p,int q){
  int op=-1;
  int max_prec=0;
  int count=0;
  for(int i=p;i<=q;i++){
    if(tokens[i].type=='('){
      count++;
    }
    else if(tokens[i].type==')'){
      count--;
    }
    else if(count==0){
      int prec=get_precedence(tokens[i].type);
      if(prec>0){
        if(prec>=max_prec){
	  max_prec=prec;
	  op=i;
	}
      }
    }
  }
  return op;
}

uint32_t eval(int p,int q){
  if(p>q){
    printf("Bad expression!\n");
    assert(0);
    return 0;
  }
  else if(p==q){
    uint32_t val=0;
    if(tokens[p].type==TK_NUM){
      sscanf(tokens[p].str,"%u",&val);
    }
    else if(tokens[p].type==TK_HEX){
      sscanf(tokens[p].str,"%x",&val);
    }
    else if(tokens[p].type==TK_REG){
      if(strcmp(tokens[p].str,"eax")==0) return cpu.eax;
      if(strcmp(tokens[p].str,"ecx")==0) return cpu.ecx;
      if(strcmp(tokens[p].str,"edx")==0) return cpu.edx;
      if(strcmp(tokens[p].str,"ebx")==0) return cpu.ebx;
      if(strcmp(tokens[p].str,"esp")==0) return cpu.esp;
      if(strcmp(tokens[p].str,"ebp")==0) return cpu.ebp;
      if(strcmp(tokens[p].str,"esi")==0) return cpu.esi;
      if(strcmp(tokens[p].str,"edi")==0) return cpu.edi;
      if(strcmp(tokens[p].str,"eip")==0) return cpu.eip;
    }
    return val;
  }
  else if(check_parentheses(p,q)==true){
    return eval(p+1,q-1);
  }
  else{
    int op=find_dominant_operator(p,q);
    if(op==-1){
      printf("Invalid expression!\n");
      assert(0);
    }
    uint32_t val1=eval(p,op-1);
    uint32_t val2=eval(op+1,q);
    switch(tokens[op].type){
      case '+': return val1+val2;
      case '-': return val1-val2;
      case '*': return val1*val2;
      case '/':
	if(val2==0){
	  printf("Division by zero!\n");
	  assert(0);
	}
	return val1/val2;
      case TK_EQ: return val1==val2;
      case TK_NEQ: return val1!=val2;
      case TK_AND: return val1&&val2;
      case TK_OR: return val1||val2;
      default: assert(0);
    }
  }
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  TODO();

  return 0;
}
