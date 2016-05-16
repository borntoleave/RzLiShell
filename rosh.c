/*RzLi, Jan.21,2016
    this is an operation system project in spring 2014
*/


//added error message, space recognition, resolved cat file>temp, and with multiple arguments.

#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pwd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <sys/wait.h>
#include <sys/stat.h>
#include <grp.h>
#include <libgen.h>
#include <signal.h>

#include <limits.h>

#define MAX_LENGTH 1024
#define CMD_LEN 50

enum {READ,WRITE};

void welcome();
void print_cmd_prompt();
char ** parse(char * cmdline,int * cmd_ct,int* flag);
void help_info();
int	exc_cmd(char **cmd_stk,int cmd_ct,int* flag);
void cd(char * cmdline);
void read_from_pipe (int file);
void write_from_pipe (int file);
void main()
{
	welcome();
	while(1)
	{	
		fflush(stdin);
		print_cmd_prompt();
		char *cmdline=malloc(CMD_LEN);
		gets(cmdline);
		if (cmdline[0]=='\0')//"Enter" key or extra space before command
			continue;
		int c;
		while(cmdline[0]==' '){cmdline+=1;}
		if	(!strncmp(cmdline,"exit",4)||!strcmp(cmdline,"quit"))
			exit(0);
		else if	(!strncmp(cmdline,"help",4))
			help_info();
		else if	(!strncmp(cmdline,"cd",2))
			cd(cmdline);
		else
		{	char **cmd_stk;int cmd_ct=0,ch=0,status,wait_sig,flag[4]={0,0,0,0};//bkgrd,pipe,streamin,streamout
			cmd_stk=parse(cmdline,&cmd_ct,flag);//get cmd stack seperated by "|" or "<>" and bkgrd flag
			/*for(ch=0;ch<cmd_ct;ch++)
				printf("%s\n",cmd_stk[ch]);
				printf("# of cmd: %d, bkgrd: %d\n",cmd_ct,flag[0]);//show parsing result*/
			if(!strncmp(cmd_stk[0],"create",6))//allow "create" to create a background procs 
				{flag[0]=1;
				strsep(&cmd_stk[0]," ");
				}
				pid_t pid=fork();//make clone of procs
				if(pid==0)		//child proc
        	    {
        	        //printf("child pid \t%d:\n",getpid());
                	exc_cmd(cmd_stk,cmd_ct,flag);
                	exit(0);
        		}
         		else 			//parent proc
         		{	
         			//printf("parent pid \t%d:\n",getpid());
        			if(flag[0])//require running in background 
       				{
       					printf("Run process %d in background....\n",pid);continue;}
       				else 
           			{
           				sleep(0.1);
           				wait_sig=waitpid(pid,&status,0);//wait for the child procs
           				signal(SIGCHLD, SIG_IGN);		//kill the zombie child procs
           			}
         		}
     		}
     }
}

void welcome()
{
	printf("\t********************************************\n\t*  Welcome using ROSH(ROngzhong's SHell)!  *\n\t********************************************\n");
}
void help_info()
{
	printf(" Use it as a normal shell. But don't use extra ' '(space) if possible.\n Type \"exit\" or \"quit\" to exit the program.\n");	
}

void print_cmd_prompt()
{
	char cwd[1024];
	char *lgn;
	struct passwd *pw;
	if ((lgn=getlogin())==(char *)NULL||(pw=getpwnam(lgn))==NULL)
	{
		fprintf(stderr, "Get of user information failed.\n");exit(1);
	}
	char hostname[1024];
	hostname[1023] = '\0';
	gethostname(hostname, 1023);
	if (getcwd(cwd, sizeof(cwd))!=NULL)
	   	printf("%s@%s:%s¥ ",pw->pw_name,hostname,cwd);//why not just use lgn?
	else
	   	{perror("getcwd() error");exit(-1);}
}

void cd(char * cmdline)
{
	errno=0;
	strsep(&cmdline," ");
	printf("Changing to directory:  %s\n",cmdline);
	chdir(cmdline);
	if(errno!=0)
		printf( "Error changing directory: %s\n", strerror(errno));
}
	


//-----------------parsing function!!----------------------------------
char ** parse(char *cmdline,int * cmd_ct,int * flag)
{
	int ch,cmd_num=0,cmd_num_i=0,cmd_len=strlen(cmdline);	
	for(ch=0;ch<cmd_len;ch++)
		{
		if		(cmdline[ch]=='&'){flag[0]=1;}
		else if	(cmdline[ch]=='|'){flag[1]=1;cmd_num++;}
		else if	(cmdline[ch]=='<'){flag[2]=1;cmd_num++;}
		else if	(cmdline[ch]=='>'){flag[3]=1;cmd_num++;}
		}
	*cmd_ct=++cmd_num;
	char ** cmd_stk=malloc(cmd_num);//dynamically allocate command stack
	for(cmd_num=0;cmd_num<*cmd_ct;cmd_num++)
		cmd_stk[cmd_num]=malloc(CMD_LEN);
	cmd_num=0;
	cmd_stk[cmd_num]=strtok(cmdline,"<>|&");
	while(cmd_stk[cmd_num]!=NULL)
	{
		cmd_stk[++cmd_num]=strtok(NULL,"<>|&");
	}
	return cmd_stk;
}

int exc_cmd(char **cmd_stk,int cmd_ct,int* flag)
{
	int i=0,f_des[3];pid_t pid;
	char ***args=malloc(cmd_ct);
	for(i=0;i<cmd_ct;i++)
	{	
		char *temp;temp=cmd_stk[i];int c=1,arg_ct=1,space_f=0;
		while(temp[c]!='\0')
			{if(temp[c]!=' ')space_f=1;
			if(space_f&&temp[c-1]==' '&&temp[c]!=' ')arg_ct++;
			c++;}
			printf("%d ",arg_ct);
		args[i]=malloc(arg_ct+1);
		for(c=0;c<arg_ct;c++)args[i][c]=malloc(CMD_LEN);
		args[i][0]=strtok(cmd_stk[i]," ");
		while(args[i][0][0]==' '){printf(",");args[i][0]=strtok(NULL," ");}
		for(c=1;c<arg_ct;c++)
		{//args[i][c]=malloc(CMD_LEN);
		args[i][c]=strtok(NULL," ");
		}		
		args[i][arg_ct]=NULL;		
	}
	if(cmd_ct==1)	//no need for pipe
		{if(execvp(args[0][0],args[0])<0)
			fprintf(stderr,"Error! Command or file not found!\n");
		fflush(stdout);
		}
	else if(flag[1]==1)			//create pipe
		for(i=1;i<cmd_ct;i++)
		{
			if(pipe(f_des)==-1)
			{
				fprintf(stderr,"Pipe failed.\n");
				return EXIT_FAILURE;
			}
			switch(fork())
			{
				case -1:
					perror("Fork");
					return 2;
				case 0://child
					dup2(f_des[WRITE],fileno(stdout));
					close(f_des[READ]);			//close(f_des[WRITE]);
					if(execvp(args[i-1][0],args[i-1])<0)
						fprintf(stderr,"Error! Command or file not found!\n");
					return 3;
				default:						//parent recieve from child
					dup2(f_des[READ],fileno(stdin));
					close(f_des[WRITE]);//close(f_des[READ]);
					if(execvp(args[i][0],args[i])<0)
						fprintf(stderr,"Error! Command or file not found!\n");
					return 4;
			}
		}
		
	else if(flag[2]==1)// for <
	{
		if(pipe(f_des)==-1)
			{
				fprintf(stderr,"Pipe failed.\n");
				return EXIT_FAILURE;
			}
			switch(fork())
			{
				case -1:
					perror("Fork");
					return 2;
				case 0://child
					dup2(f_des[WRITE],fileno(stdout));
					close(f_des[READ]);//close(f_des[WRITE]);
					if(execlp("cat","cat",args[1][0],(char*)NULL)<0)
						fprintf(stderr,"Error! File not found!\n");
					return 3;
				default://parent recieve from child
					dup2(f_des[READ],fileno(stdin));
					close(f_des[WRITE]);//close(f_des[READ]);
					if(execvp(args[0][0],args[0])<0)
						fprintf(stderr,"Error! Command or file not found!\n");
					return 4;
			}
	}
	
	
	else if(flag[3]==1)// for >
	{
		if(pipe(f_des)==-1)
			{
				fprintf(stderr,"Pipe failed.\n");
				return EXIT_FAILURE;
			}
			switch(fork())
			{
				case -1:
					perror("Fork");
					return 2;
				case 0://child
					dup2(f_des[WRITE],fileno(stdout));
					close(f_des[READ]);			//close(f_des[WRITE]);
					if(execvp(args[0][0],args[0])<0)
						fprintf(stderr,"Error! Command or file not found!\n");
					return 3;
				default:						//parent recieve from child
					dup2(f_des[READ],fileno(stdin));
					close(f_des[WRITE]);//close(f_des[READ]);
					FILE *fp;
					int nbytes;char buffer[32768];
					nbytes=read(f_des[READ],buffer,sizeof(buffer));
					if((fp=fopen(args[1][0],"w")) == NULL)
					{ 
  						printf("File failed to open");
    					exit(1);
					}
					fprintf(fp,"%s",buffer);
					fclose(fp);
   					return 4;
			}
	}
	return 0;		
	free(args);
	
}

void read_from_pipe (int file)//not used 
{
	FILE *stream;
	int c;
	stream = fdopen (file, "r");
	while ((c = fgetc (stream)) != EOF)
		putchar (c);
	fclose (stream);
}
/* Write some random text to the pipe. */
void write_to_pipe (int file)
{
	FILE *stream;
	stream = fdopen (file, "w");
	fprintf (stream, "hello, world!\n");
	fclose (stream);
}     
  
