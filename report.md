To Impliment The Sshell i Divided the entire assignment in three part
1) parse The input and display the error if there are any parsing error and method that will perfrom this part is parse_input
2) Execute the pipe commands in this part take each of commands seperated with pipe and execute them one by one using execute_command method
3) Finally Execute commands will execute the commands using execvp command function name for this execute_command
4) main function that have some basic builtin command like cd that is executed using chdir and exit command using exit(0) and lastly to set the variable and also calling the parse_input function and pipe_command  function
parse_input() :- Before parsing i have created structure for commands and variables where command contains the num_arg for number of argument and argument array and variable conatins the variable name as char and array of char as variable value.
To Parse First i divided the entire input string seperated by | to see if there are any pipe command and since if it is not pipe command then obiously there are one command.
After that for each of command string again split it either by many spaces or by single spaces and store them in the command structure.
At any time if there is an parsing error it will marks p_error as 1 flagging that there is parsing error and display the error message (note:- with this i ensure that no need to execute the command  just need to do the continue for asking input for next command) and then return from parse_input.
With That if there are one or more commands they are stored in command structure.
pipe_command():- This check if the number of command is one or more if the number of command is one then it check of input and output redirection and based on that it set the input and output file descripter and with that it call the execute_command for execution.
if it have more then one command and there is pipe then it create the pipe using pipe() function and run the command one by one and it also change the input_fd based on last executed command for each of command in pipe it will do the same by calling execute_command function with input and output file descriptor At last it will print the completed message of the commands.
Execute_commands():- In This function will create a child process to execute the commands and in child process it check if there is valid input or output filedescripter given if it is then it duplicate it and execut the command by pasing the command and its argument to the execvp() function and it will execute the command and parrent process wait for child to finish and then it will close the file descipters and then return the status of command that executed.
For Extra Features I just create a varible and after the parsing is done i check if it is an set command then just update the varible i have created and if those variable are used in some other commands then while parsing i just update the argument in the command structure itself with this the first extra feature is implimented.
In Order to Test My Implimentation i run the commands given in assignment pdf and verify that it generate the same output.
With that i am able get most of answer are as same as it is in the assignment but not all.




