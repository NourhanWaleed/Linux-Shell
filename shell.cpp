#include <iostream>
#include <string.h>
#include <sys/wait.h>
#include <sys/unistd.h>
#include <fstream>
#include <sys/types.h>
#include <limits.h>

#define MAX_args 30

using namespace std;

const string WHITESPACE = " \n\r\t\f\v";
//creating log file
ofstream log_file;
// trimming the start of the string by removing whitespace
string ltrim(const string &s){
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == string::npos) ? "" : s.substr(start);
}
// trimming the end of string by removing whitespace
string rtrim(const string &s){
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == string::npos) ? "" : s.substr(0, end + 1);
}
//calling  both in one
string trim(const string &s) {
    return rtrim(ltrim(s));
}

//signal handler
void handle1(int signal){
	pid_t pid ;
	int wstat;
    if(log_file.is_open() &&(pid =  
    waitpid(-1,&wstat,WNOHANG ) ) > 0)
           log_file << "Child PID: "<< pid << " has terminated\n";
}



int main(void){
    //sending signal to parent
    signal(SIGCLD,handle1);
    signal(SIGCHLD,handle1);

    //opening log file
    log_file.open("run_log.txt");

    while(true){
        string temp0;
        cout << "Shell >";
        getline(cin,temp0);
        temp0 = trim(temp0);

        if(temp0.length() == 0)
            continue;
        //input command
        char input[temp0.length()+1];
        char *args [MAX_args];
        //creating null array for input and max args
        memset(input,0,(temp0.length()+1)*sizeof(char));
        memset(args,0,MAX_args*sizeof(char *));
        strcpy(input,temp0.c_str());


        //tokenizing by space
        char * temp1 = strtok(input, " ");

        if(temp1 == NULL)
            continue;

        int i =0;
        while(temp1!=NULL){
            args[i++] = temp1;
            temp1 = strtok(NULL, " ");
        }


     // if input is exit, file closes and exit the program
        if(strcmp(args[0],"exit") == 0){
            log_file.close();
            exit(0);
        }
        //executing cd command
        else if(strcmp(args[0],"cd") == 0) {
            if(args[i-1][0] =='.'){
                if(args[i-1][1] == '.'){
                    chdir("..");
                }else{
                    chdir(args[i-1]);
                }
            } else {
            	chdir(args[i-1]);
            }
            continue;
        }
// create a child process identical to parent process
        pid_t childpid = fork();
        int wstat;

        if(childpid < 0 ) {
            cerr << "Failed to create child process"<< endl;
            continue;
        }
        // program [arguments/flag] ... | & , if args>0 and last arg is &, do not wait on child process
        if(i > 1 && *args[i-1] == '&') {
            //inside child
            if(childpid == 0 ) {
                *args[i-1] = '\0';
                args[i-1]  = NULL;
            }
        } else {
            //inside parent and if fork is successful and no &,parent waits
            if (childpid > 0 ){
                pause(); // parent sleeps until a signal is recived by the parent process
            }
        }
//execute command if child
        if(childpid == 0){
            execvp(args[0],args);
            //printed if child process failed
            cerr << "Failed to execute program" << endl;
            exit(-1);
        }
    }
    return 0;
}
