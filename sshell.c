#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#define MAX_COMMAND_LENGTH 512
#define MAX_NUM_ARGS 16
#define MAX_ARG_LENGTH 32
// command structure
typedef struct {
  char *args[MAX_NUM_ARGS];
  int num_args;
} Command;
struct vector {
    double* result;
    const double* vector1;
    const double* vector2;
    int size;
    int num_threads;
    int start;
    int part_of_vec;
};



void* dotprod(void* arg) {
    struct vector* v = (struct vector*)arg;
    int end=v->start + v->part_of_vec < v->size?v->start + v->part_of_vec:v->size;
    for (int i = v->start; i < end; i++) {
        v->result[i] = v->vector1[i] * v->vector2[i];
    }
    return NULL;
}

void* subvec(void* arg) {
    struct vector* v = (struct vector*)arg;
    int end=v->start + v->part_of_vec < v->size?v->start + v->part_of_vec:v->size;
    for (int i = v->start; i < end; i++) {
        v->result[i] = v->vector1[i] - v->vector2[i];
    }
    return NULL;
}

void* addvec(void* arg) {
    struct vector* v = (struct vector*)arg;
    int end=v->start + v->part_of_vec < v->size?v->start + v->part_of_vec:v->size;
    for (int i = v->start; i < end; i++) {
        v->result[i] = v->vector1[i] + v->vector2[i];
    }
    return NULL;
}
int execute_threads(const char* filename1, const char* filename2, int num_threads, char* function_name) {
    // Open and read the first input file 
    FILE* file1 = fopen(filename1, "r");
    FILE* file2 = fopen(filename2, "r");
    // check if file can be open or not 
    if (file1 == NULL) {
        fprintf(stderr, "Failed to open %s\n", filename1);
        return -1; 
    }
    if (file2 == NULL) {
        fclose(file1); // Close file1
        fprintf(stderr, "Failed to open %s\n", filename2);
        return -1; 
    }
    // Determine vector size from file data.
    int size = 0;
    double num1, num2;
    // read only same number of values
    while (fscanf(file1, "%lf", &num1) == 1 && fscanf(file2, "%lf", &num2) == 1) {
        size++;
    }
    rewind(file1);
    rewind(file2);
    // Check if the vector sizes match.
    if (size == 0) {
        fprintf(stderr, "No number/values in the list.\n");
        return -1;
    }
    double* result = (double*)malloc(size * sizeof(double));
    double* vector1 = (double*)malloc(size * sizeof(double));
    double* vector2 = (double*)malloc(size * sizeof(double));
    for (int i = 0; i < size; i++) {
        fscanf(file1, "%lf", &vector1[i]);
        fscanf(file2, "%lf", &vector2[i]);
    }
    fclose(file1);
    fclose(file2);
    // perform addition.
    pthread_t threads[MAX_size];
    struct vector list[MAX_size];
    // number of threads to create
    num_threads=num_threads>size?size:num_threads;
    for (int i = 0; i < num_threads; i++) {
        list[i].result = result;
        list[i].vector1 = vector1;
        list[i].vector2 = vector2;
        list[i].size = size;
        list[i].num_threads = num_threads;
        list[i].start=i*size/num_threads;
        list[i].part_of_vec=i==num_threads-1?(size-i*size/num_threads):size/num_threads;
        if(strcmp(function_name,"addvec")==0)
        pthread_create(&threads[i], NULL, addvec, &list[i]);
        if(strcmp(function_name,"subvec")==0)
        pthread_create(&threads[i], NULL, subvec, &list[i]);
        if(strcmp(function_name,"dotprod")==0)
        pthread_create(&threads[i], NULL,dotprod , &list[i]);
    }
    // Wait for all threads to complete.
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    // Print the result.
    printf("Result Vector:\n");
    for (int i = 0; i < size; i++) {
        printf("%lf ", result[i]);
    }
    printf("\n");
    free(result);
    free(vector1);
    free(vector2);
    return 0; // Return success code.
}


// parse the input and then seperate the command
void parse_input(char *input, Command *commands, int *num_commands,
                 int *background, int *p_error) {
  *background = 1;
  *num_commands = 0;
  const char delimiters[] = "|";

  char commands1[MAX_NUM_ARGS][MAX_COMMAND_LENGTH];
  memset(commands1, 0, sizeof(commands1));
  // Split input by pipe symbol and store each command in the commands array
  char *token = strtok(input, delimiters);
  while (token != NULL) {
    strcpy(commands1[*num_commands], token);
    (*num_commands)++;
    token = strtok(NULL, delimiters);
  }
  
  for (int i = 0; i < *num_commands; i++) {
    Command cmd;
    cmd.num_args = 0;
    char *command_token = commands1[i];
    const char arg_delimiters[] = " \t\r\n";
    char *arg_token = strtok(command_token, arg_delimiters);
    while (arg_token != NULL) {
       if (strcmp(arg_token, "&") == 0) {
        *background = 0;
      }  else {
        if (cmd.num_args == MAX_NUM_ARGS) {
          fprintf(stderr, "Error: too many process arguments\n");
          *p_error = 1;
          return;
        }
        cmd.args[cmd.num_args] = arg_token;
        cmd.num_args++;
      }
      arg_token = strtok(NULL, arg_delimiters);
    }
    cmd.args[cmd.num_args] = NULL;
    if (cmd.num_args > 0) {
      commands[i] = cmd;
    }
  }
}

int execute_command(Command *command, int input_fd, int output_fd,int background) {
  pid_t pid = fork();
  if (pid == -1) {
    fprintf(stderr, "Error: cannot fork process\n");
    return -1;
  } else if (pid == 0) {
    // Child process
    if (input_fd != -1) {
      dup2(input_fd, STDIN_FILENO);
      close(input_fd);
    }
    if (output_fd != -1) {
      dup2(output_fd, STDOUT_FILENO);
      close(output_fd);
    }

    // Execute the command
    execvp(command->args[0], command->args);
    fprintf(stderr, "Error: command not found\n");
    exit(0);
  } else {
    // Parent process
    if (input_fd != -1) {
      close(input_fd);
    }
    if (output_fd != -1) {
      close(output_fd);
    }
    int status;
    if(background){
         waitpid(pid, &status, 0);
         }
    return status;
  }
}


int execute_pipe(Command *commands, int num_commands, char *input) {

  int input_fd = -1, output_fd = -1, perror = 0;
  int command_status[num_commands];
  int pipe_fd[2];

  if (num_commands == 1) {
    // check input/output redirection
    for (int i = 0; i < commands[0].num_args; i++) {
      if (strcmp(commands[0].args[i], "<") == 0) {
        if (i == commands[0].num_args - 1) {
          fprintf(stderr, "Error: missing input file\n");
          perror = 1;
          break;
        }
        input_fd = open(commands[0].args[i + 1], O_RDONLY);
        if (input_fd < 0) {
          fprintf(stderr, "Error: cannot open input file\n");
          perror = 1;
          break;
        }
        commands[0].args[i] = NULL;
        commands[0].num_args = i;
        break;
      } else if (strcmp(commands[0].args[i], ">") == 0) {
        if (i == commands[0].num_args - 1) {
          fprintf(stderr, "Error: missing output file\n");
          perror = 1;
          break;
        }

        output_fd = open(commands[0].args[i + 1], O_CREAT | O_WRONLY | O_TRUNC,
                         S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (output_fd < 0) {
          fprintf(stderr, "Error: cannot open output file\n");
          perror = 1;
          break;
        }
        commands[0].args[i] = NULL;
        commands[0].num_args = i;
        break;
      }
    }
    // call to execute the commands that are non piped
    if (perror == 0)
      return execute_command(&commands[0], input_fd, output_fd,1);
    else
      return -2;
  }
  // check input/output redirection
   
  for (int i = 0; i < num_commands; i++) {
     if (i != num_commands - 1) {
      if (pipe(pipe_fd) == -1) {
        fprintf(stderr, "Error: cannot create pipe\n");
        return -1;
      }
      output_fd = pipe_fd[1];
    }
    else{
   
    // check input/output redirection
     
    for (int j = 0; j < commands[j].num_args-1; j++) {
       if (strcmp(commands[j].args[j], ">") == 0) {
        if (i == commands[j].num_args - 1) {
          fprintf(stderr, "Error: missing output file\n");
          perror = 1;
          break;
        }

        output_fd = open(commands[j].args[j + 1], O_CREAT | O_WRONLY | O_TRUNC,
                         S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (output_fd < 0) {
          fprintf(stderr, "Error: cannot open output file\n");
          perror = 1;
          break;
        }
        break;
      }
    }
    }
    
    command_status[i] = execute_command(&commands[i], input_fd, output_fd,1);
    
     if (input_fd != -1) {
      close(input_fd);
    }
    input_fd = pipe_fd[0];
    if (output_fd != -1) {
      close(output_fd);
    }
  }

  printf("completed '");


  for (int i = 0; i < num_commands; i++) {
    printf("[%d] ", WEXITSTATUS(command_status[i]));
    if (command_status[i] == -1) {
      printf(": execution failed\n");
    }
  }
  printf("\n");
  return 0;
}


int main() {
  char *input;
  char *input1;
  char *input2;
  Command commands[MAX_NUM_ARGS];
  int num_commands;
  int background;
  
  int p_error = 0;
  while (1) {
    input = readline("sshell>");
    if (!input || strlen(input)<=0) {
      continue;
    }
    strcpy(input2, input);
    int flag=0;
    while(input2[strlen(input2)-1]== '\\' ){
    if(flag==0){
    flag=1;
    
    input[strlen(input)-1]='\0';
    }
    else{
    	input2[strlen(input2)-1]='\0';
        strcat(input, input2);
    }
    input2 = readline("> ");
    }
    if(flag==1){
       strcat(input, input2);
    }

    strcpy(input1, input);
    add_history(input);
    if (strcmp(input1, "addvec") == 0) {
    
     printf("fine now ");
     continue;
    }
    if (strcmp(input1, "subvec") == 0) {
    
     printf("fine now ");
     continue;
    }
    if (strcmp(input1, "dotprod") == 0) {
    
     printf("fine now ");
     continue;
    }
    // parsing error in code
    
    parse_input(input, commands, &num_commands, &background, &p_error);
    
    if (p_error != 0) {
      p_error = 0;
      continue;
    }
    if (strcmp(commands[0].args[0], "exit") == 0) {
      int status1;
      fprintf(stderr, "completed '%s' [%d]\n", commands[0].args[0],
              WEXITSTATUS(status1));
      exit(0);
    }
    
    if (strcmp(commands[0].args[0], "cd") == 0) {
      if (commands[0].num_args < 1) {
        fprintf(stderr, "Error: missing argument for cd\n");
      } else {
        int status2;
        if (commands[0].num_args == 1) {
          status2 = chdir(commands[0].args[0]);
        } else {
          status2 = chdir(commands[0].args[1]);
        }
        if (status2 == -1) {
          fprintf(stderr, "Error: cannot cd into directory\n");
        }
        fprintf(stderr, "completed '%s' [%d]\n", input1,
                WEXITSTATUS(status2));
      }
      continue;
    }
    if (strcmp(commands[0].args[0], "help") == 0) {
      if (commands[0].num_args > 1) {
        fprintf(stderr, "Error: To many argument for help\n");
      } else {
      printf(" 1. pwd \n 2. cd <directory_name> \n 3. mkdir <directory_name> \n 4. ls <flag> \n 5. exit \n 6. help \n 7. for other command it will directly execute it.\n");
      continue;
    }
    }
     
    //execute_command(commands,-1,-1,background);
    int status = execute_pipe(commands, num_commands, input1);
    if (status == -1) {
      fprintf(stderr, "Error: execution failed\n");
    }
    if (status != -1 && status != -2 && num_commands == 1) {
      fprintf(stderr, "+ completed '%s' [%d]\n", input1, WEXITSTATUS(status));
    }
  }
  return 0;
}
