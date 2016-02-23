/////////////////////////////////////
//      Author : Dhruv Khattar    //
///////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

//Global Variables

char host[1000];
char user[1000];
char homedir[1010]="/home/";
char line[1000006];
char execpath[1000];
char execdir[1000];

int INFILE=0;
int OUTFILE=1;
int JOBCTR=1;
int PIPEIN=0;
pid_t SHELLPID;

typedef struct jobs
{
    char name[100];
    pid_t pid;
}jobs;

jobs job[1000];

// Used to tokenize lines into commands
void parse( char *line , char **tokens , char* delim)
{
    char *temp;

    //Splitting line by delim and saving it into temp
    temp = strtok( line , delim );
    int i = 0;
    while( temp != NULL )
    {
        tokens[i++] = temp;
        temp = strtok( NULL , delim );
    }
}

void deleteJob(int jobpid)
{
    int i;
    int flag=0;
    for(i=1;i<JOBCTR;i++)
    {
        if( job[i].pid == jobpid )
            flag=1;

        if(flag==1)
            job[i]=job[i+1];
    }
}

//Function to invoke shell
void invokeShell()
{
    char home[1000];
    getcwd(home,1000);
    int flag=0;
    char changehome[1000]="~";

    int i;

    // Checking for the directory to be printed
    // Changing home dir to ~
    if ( strlen(execdir) <= strlen(home) )
        for( i = 0 ; execdir[i]!='\0' ; i++ )
        {
            if( execdir[i] != home[i] )
            {
                flag=1;
                break;
            }
        }
    else
        flag=1;

    if( flag == 0 )
    {
        int j,k;
        for( j=i , k=1 ; home[j]!='\0' ; j++,k++ )
            changehome[k] = home[j];
        printf("%s@%s:%s>",user,host,changehome);
    }
    else
        printf("%s@%s:%s>",user,host,home);
}

void sig_handler(int sig)
{
    if(sig == SIGINT)
    {
        printf("\n");
        invokeShell();
        fflush(stdout);
    }
    if(sig == SIGTSTP)
    {
    //    pid_t pid = getpid();
      //  if(pid != SHELLPID )
        //{
          //  kill(pid,SIGKILL);
           // job[JOBCTR++].pid = pid;
            //     wait();
       // }
        printf("\n");
        invokeShell();
        fflush(stdout);
    }
    if(sig == SIGQUIT)
    {
        printf("\n");
        invokeShell();
        fflush(stdout);
    }
    if(sig == SIGCHLD)
    {
        union wait wstat;
        pid_t pid;

        while(1)
        {
            pid = wait3(&wstat,WNOHANG,(struct rusage *)NULL);
            if(pid == 0 )
                return;
            else if(pid == -1)
                return;
            else
            {
                fprintf(stderr,"\nProcess with PID : %d exited with return value: %d\n",pid,wstat.w_retcode);
                deleteJob(pid);
            }
        }
    }
}

// Making built-in "echo" command
void executeECHO(char **tokens)
{
    int i = 1;
    int j = 0;
    char input[1000006];

    int flag1 = 0;
    int flag2 = 0;

    //checking for inverted commas
    while( tokens[i] != NULL )
    {
        int k;
        for ( k=0 ; tokens[i][k]!='\0' ; k++ )
        {
            if (flag1 == 1)
            {
                if(tokens[i][k] == '\"' )
                    flag1=0;
                else
                    input[j++]=tokens[i][k];
            }
            else if (flag2 == 1)
            {
                if(tokens[i][k] == '\'' )
                    flag2=0;
                else
                    input[j++]=tokens[i][k];
            }
            else
            {
                if(tokens[i][k] == '\"' )
                    flag1=1;
                else if(tokens[i][k] == '\'' )
                    flag2=1;
                else
                    input[j++]=tokens[i][k];

            }
        }
        i++;
    }
    input[j]='\0';

    //Checking if a inverted comma is left open
    if(flag1 == 0 && flag2 == 0)
        printf("%s\n",input);
    else
        printf("Error: Wrong Input\n");
}

// Making built-in "cd" command
void executeCD(char **tokens)
{
    char home[1000];

    strcpy(home,execdir);

    int i;
    int len = strlen(execdir);

    //If command is 'cd' , change to ~
    //Changing ~ to home directory of executable
    //Then using chdir to change directory
    if( tokens[1] == NULL )
        chdir(execdir);
    else if ( tokens[1][0] == '~' )
    {
        for( i=1 ; tokens[1][i]!='\0' ; i++ )
        {
            home[i+len-1] = tokens[1][i];
        }
        home[i+len-1]='\0';
        if( chdir(home) != 0 )
            perror("Error");
    }
    else if ( chdir(tokens[1]) != 0)
        perror("Error");
}

//Making built-in "pwd" command
void executePWD()
{
    char home[1000];
    getcwd(home,1000);
    printf("%s\n",home);
}

//Making "pinfo" user-defined command
void executePINFO(char **tokens)
{

    char status[10000] = "cat /proc/";
    int j = strlen(status);

    //Checking if pid is given or not
    if(tokens[1] == NULL)
    {
        char buff[1000] = "" ;
        ssize_t len = readlink("/proc/self/exe" , buff , sizeof(buff)-1);
        printf("Executable Path -- %s\n",buff);

        char a[20] = "self/status";
        int i;

        for( i=0 ; a[i]!='\0' ; i++ )
            status[j++] = a[i];
    }
    else
    {
        int i;
        char b[10] = "/status";

        char a[1000] = "/proc/";
        char c[10] = "/exe";

        int k = 6;

        for( i=0 ; tokens[1][i]!='\0' ; i++ )
            a[k++] = tokens[1][i];
        for( i=0 ; c[i]!='\0' ; i++ )
            a[k++] = c[i];

        char buff[1000] = "";
        ssize_t len = readlink("/proc/self/exe" , a , sizeof(buff)-1);
        printf("Executable Path -- %s\n",buff);

        for( i=0 ; tokens[1][i]!='\0' ; i++ )
            status[j++] = tokens[1][i];
        for( i=0 ; b[i]!='\0' ; i++ )
            status[j++] = b[i];
    }

    //Seperating commands and arguments
    char *final[1000] = {NULL} ;
    parse(status,final," ");

    //Forking to run cat /proc/[pid]/status
    pid_t  pid;
    int flag;

    pid = fork();
    if( pid < 0 )
    {
        perror("Forking Error ");
    }
    else if(pid  == 0 )
    {
        if( execvp(*final , final) < 0)
        {
            perror("Error ");
            exit(0);
        }
    }
    else
    {
        wait(&flag);
    }

}


void executeJOBS()
{
    int i;
    if(JOBCTR==1)
        printf("No background processes running\n");
    for( i=1 ; i < JOBCTR ; i++ )
        printf("[%d] %s [%d]\n",i,job[i].name,job[i].pid);
}

void executeKJOB(char **tokens)
{
    if(tokens[2]==NULL)
        printf("Less number of arguments given\n");
    else
    {
        if( atoi(tokens[1]) > JOBCTR )
            printf("Job number does not exist.");
        else
        {
            kill(job[atoi(tokens[1])].pid,atoi(tokens[2]));
            if(atoi(tokens[2])==9)
            {
                deleteJob(job[atoi(tokens[1])].pid);
                JOBCTR--;
            }
        }
    }
}

void executeOVERKILL()
{
    int i;

    if(JOBCTR>1)
        for(i=JOBCTR-1;i>0;i--)
        {
            kill(job[i].pid,9);
            signal(SIGCHLD,sig_handler);
        }
    else
        printf("No Background Jobs detected.\n");
    JOBCTR=1;
}

void executeFG(char **tokens)
{
    if(tokens[1]==NULL)
        printf("Job number not specified\n");
    else
    {
        int jobno = atoi(tokens[1]);
        if( jobno < JOBCTR)
        {
            kill(job[jobno].pid,SIGCONT);
            JOBCTR--;
            deleteJob(job[jobno].pid);
            wait();
        }
        else
            printf("Job number does not exist\n");
    }

}

void checkInfile(char *tokens)
{
    int i=0;
    char *temp[100]={NULL} ;

    parse( tokens , temp , "<" );

    if(temp[1]!=NULL)
    {
        char *temp1[100]={NULL};
        parse( temp[1] , temp1 , " " );

        INFILE = open(temp1[0],O_RDONLY);
    }
}

void checkOutfile(char *tokens)
{
    int i=0;
    char *temp[100]={NULL} ;

    parse( tokens , temp , ">" );

    if(temp[1]!=NULL)
    {
        char *temp1[100]={NULL};
        parse( temp[1] , temp1 , " " );

        OUTFILE = open(temp1[0],O_TRUNC | O_WRONLY | O_CREAT, S_IRWXU);
    }
    else
        OUTFILE=1;
}

int backgroundCheck(char ** tokens)
{

    int i=0;
    while(tokens[i]!=NULL)
    {   
        if(strcmp(tokens[i],"&")==0)
        {
            tokens[i]=NULL;
            return 1;
        }
        i++;
    }   
    return 0;
}

//Executes all the commands in a single line
void executeCommand()
{
    char *temp[100]={NULL};
    char *temppipe[100]={NULL};

    // Checking if there are multiple commands seperated by ;
    parse( line , temppipe , ";" );
    int j=0;
    while(temppipe[j]!=NULL)
    {

        parse( temppipe[j] , temp , "|" );
        int i = 0;
        int fd[2];
        while( temp[i] != NULL )
        {
            char *tokens[100] = {NULL};
            char *temp1[100]={NULL};
            char *temp2[100]={NULL};

            checkOutfile(temp[i]);
            checkInfile(temp[i]);


            // Seperating commands and various arguments
            parse( temp[i] , temp1 , "<" );
            parse( temp1[0] , temp2 , ">" );
            parse( temp2[0] , tokens , " " );

            if(tokens[0]==NULL)
                return;

            int bg = backgroundCheck(tokens);
            signal(SIGCHLD,sig_handler);

            // Checking if command is builtin or not
            if ( strcmp(tokens[0],"quit") == 0)
                _exit(0);
            else if ( strcmp(tokens[0],"cd") == 0)
                executeCD(tokens);
            else if ( strcmp(tokens[0],"pwd") == 0)
                executePWD();
            else if ( strcmp(tokens[0],"echo") == 0)
                executeECHO(tokens);
            else if ( strcmp(tokens[0],"pinfo") == 0)
                executePINFO(tokens);
            else if ( strcmp(tokens[0],"jobs") == 0)
                executeJOBS();
            else if ( strcmp(tokens[0],"kjob") == 0)
                executeKJOB(tokens);
            else if ( strcmp(tokens[0],"overkill") == 0)
                executeOVERKILL();
            else if ( strcmp(tokens[0],"fg") == 0)
                executeFG(tokens);
            else
            {
                pipe(fd);

                if(bg)
                    strcpy(job[JOBCTR].name,tokens[0]);

                pid_t  pid;
                int flag;

                pid = fork();
                if( pid < 0 )
                {
                    perror("Forking Error ");
                }
                else if(pid  == 0 )
                {

                    if(INFILE != 0)
                    {
                        dup2(INFILE,STDIN_FILENO);
                        close(INFILE);
                    }
                    if(OUTFILE != 1)
                    {
                        dup2(OUTFILE,STDOUT_FILENO);
                        close(OUTFILE);
                    }
                    if(temp[i+1]!=NULL) 
                    {
                        dup2(fd[1],STDOUT_FILENO);
                        close(fd[1]);
                    }

                    // executing command
                    if(execvp(*tokens,tokens) < 0)
                    {
                        perror("Error ");
                        exit(0);
                    }
                }
                else
                {
                    //Waiting for child process to end
                    if(!bg)
                        wait(&flag);
                    else
                    {
                        job[JOBCTR++].pid = pid;
                        printf("Process started: %s [%d]\n",tokens[0],pid);
                    }
                    INFILE=fd[0];
                    close(fd[1]);
                }
            }
            i++;
        }
        INFILE=0;
        OUTFILE=1;
        j++;
    }
}

int spaceCheck()
{
    int i=0;
    while( line[i] != '\0' )
    {
        if(line[i]!=' ' && line[i]!='\t')
            return 0;
        i++;
    }
    return 1;    
}

int main( int argc , char *argv[] )
{
    SHELLPID = getpid();
    //Getting hostname and Username
    gethostname(host,1000);
    getlogin_r(user,1000);

    getcwd(execdir,1000);

    //Loop to take input repeatedly
    while( 1 )
    {
        line[0] = '\0';

        signal(SIGINT,SIG_IGN);
        signal(SIGQUIT,SIG_IGN);
        signal(SIGTSTP,SIG_IGN);
        if ( signal(SIGINT,sig_handler) == 0 )
            continue;
        if ( signal(SIGQUIT,sig_handler) == 0 )
            continue;
        if ( signal(SIGTSTP,sig_handler) == 0 )
            continue;

        //invoking shell
        invokeShell();

        if(scanf("%[^\n]s",line)!=EOF )
        {
            getchar();
            if(spaceCheck())
                continue;
        }

        else
        {
            putchar('\n');
            continue;
        }

        //executing commands
        executeCommand();
    }
    return 0;
}
