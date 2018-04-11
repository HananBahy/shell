#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_SIZE 1024


void input_line_fun(char line[]);
void parsing_line_fun(char* parsing_line[],char line[]);
void serching_out(char line []);
void pipe_search(char line [],char* parsing_line[]);
void count_pipe();
void delete_spaces(char* out_file,char* out_file_d);

int status1;
int status2;
int in_red=0;
int out_red=0;
int pipe_f=0;
char *out_file;
char *out_file_d;
char *in_file;
char *in_file_d;
char line[MAX_SIZE];
char last_char;
pid_t pid, pid2;
//int fd_out,fd_in;    //used in as descriptor in i/p|o/p redirection in pipe;
int index_pipe[10]={0,0,0,0,0,0,0,0,0,0};
int ind=0;



void main()
{
char *parsing_line[10]; 



while(1)
{
	printf("sish:>");
	input_line_fun(line);

	if((strcmp(line,"exit")==0 || line=='\0')){break;}
	
	serching_out(line);

/////////child\\\\\\\ 

pid=fork();
if(pid==0)
 {  count_pipe(); 
    
    if ( pipe_f > 0 ){
	pipe_search(line,parsing_line); 

	}

    else{ 
         
	if (out_red==1 && pipe_f==0 && out_file!=NULL )
	{
	   strtok(line,">");
	//delete_spaces(out_file,out_file_d);
	dup2(open(out_file,O_RDWR | O_CREAT |O_SYNC,0777),1);
	}

	if (in_red==1 && pipe_f==0 && in_file !=NULL ){

            strtok(line,"<");
	//delete_spaces(in_file,in_file_d);
	dup2(open(in_file,O_RDWR | O_CREAT |O_SYNC,0777),0);}
          //////////handling error in input output file////////
         if (out_red>1 || (out_red==1 &&  *(out_file)=='\0') ){
		
        	 printf("ERROR:unlogical output \n");
          	continue;
         }
	 
	if (in_red>1 || (in_red==1 && *(in_file)=='\0')  ){
		
         	printf("ERROR:unlogical input\n");
          continue;
        }
       ///////////////////////////////////////////////////////
	parsing_line_fun(parsing_line,line);
        execvp(parsing_line[0],parsing_line); 
	perror("Error");} }


else if(pid<0){perror("ERROR");}

else
   {
     if(last_char!='&'){waitpid(pid,&status1,WUNTRACED|WCONTINUED);}
in_red=0;
out_red=0;
pipe_f=0;
out_file = NULL;
in_file = NULL;
close(open(in_file,O_RDWR,0777));
close(open(out_file,O_RDWR,0777));


}

}

}

////////read function/////////
void input_line_fun(char line[])
{
fgets(line,MAX_SIZE,stdin);
//remove "\n" at the end of input and make it "\0"
int i=0;
while(line[i] !='\n' ){i++;}
line[i]='\0';
last_char=line[i-1];
//delete & sign
if(last_char=='&')
     line[i-1]='\0';
}

///////parsing function//////
void parsing_line_fun(char* parsing_line[],char* line)
{
   int i =0;

 //serching_out(line);
	parsing_line[i]=strtok(line," ");
       
	while (line[i]!='\0'){
	i++;
	parsing_line[i]=strtok(NULL," ");
}
}

//////////searching '>' and '<' ///////
void serching_out(char line []){
in_red=0;
out_red=0;
pipe_f=0;
out_file = NULL;
in_file = NULL;
int i=0;

while(line[i]!='\0'){

if(line[i]=='>'){
out_red++;
out_file=& line[i+1];

}

if(line [i]=='<'){
in_red++;
in_file=&line[i+1];

}


i++;
}

}

//////////////////count pip////////

void count_pipe(){
int i=0;
int j=0;
	
while(line[i]!='\0'){
	if(line[i]=='|'){
  	pipe_f++;
         //printf("%d",pipe_f);
	index_pipe[j]=i;
	j++;
	}i++;}
}
void pipe_search(char line [],char* parsing_line[]){
int j=pipe_f; // no of pipes |
int pipefd[2];
if(pipe_f>=1){
	ind=0;
	char*before;
	char*after;
	int b,a,x;
	b = 0;
	while(ind<=j){
  		pipe(pipefd);
  		x=0;
		if(ind<j){
  		  if(ind>0)
      			 b=index_pipe[ind-1]+1; 
		   before = &line[b]; 
  		   while(b<index_pipe[ind]){
       			before[x]=line[b];
       			b++;
				x++;}
 		       b++;	
	/*	while((b<index_pipe[ind+1] | (line[b]!='\0' && ind==j-1)))
  			{after[a]=line[b];
     			b++;
      			a++;}	
		*/		
		     }
                else {   //this mean its final iteration and (final command after final pipe |)
		x = 0;
		before = &line[b];
		while(line[b] !='\0'){
			before[x] = line[b];
			b++;
			x++;}	
		}

		pid2=fork();
                ///////child/////////
		if(pid2==0)  
		{before[x]='\0';  //to make before string ended

		
                if(ind==0){////first command
                  serching_out(before);
                 if(in_file!='\0'&& in_red==1){
                  //delete_spaces(in_file,in_file_d);
                  dup2(open(in_file,O_RDWR | O_CREAT |O_SYNC,0777),0);
		  perror("ERORR");
                  strtok(before,"<");}}
                   
                if(ind==j){////////last iteration
                 serching_out(before);
                 if(out_file!='\0'&& out_red==1){
		 //delete_spaces(out_file,out_file_d);
                  dup2(open(out_file,O_RDWR | O_CREAT |O_SYNC,0777),1);
		  perror("ERORR");
                  strtok(before,">");}}
		parsing_line_fun(parsing_line,before);

		
		if(ind < j) dup2(pipefd[1],1);
		close(pipefd[0]);
		execvp(parsing_line[0],parsing_line); 
		perror(parsing_line[0]);
		} 
		 //////////parent/////////////// 
		else if (pid2>0){
		waitpid(pid2,&status2,WUNTRACED|WCONTINUED);
                dup2(pipefd[0],0);
		close(pipefd[1]);
                //if(ind==0){close(fd_in);}///first iteration    //logically they 2 lines of code have meaning,but by default in redirection with pipes ,they return by default to stdin and stdout
                //if(ind==j){close(fd_out);}//last iteration
		 
		}
		else{     ///pid2<0
			exit(0);
		}

		ind++;
		}


	}
}

///////////delete spaces from start of out_file or in_file ////////
/*void delete_spaces(char* out_file,char* out_file_d){

  if(out_file[0]==' '){
    int i,j=0;
    while(out_file[i]!='\0'){
     if(out_file[i]!=' '){
	out_file_d[j]=out_file[i];
	j++;}

    i++;}
    out_file_d[j]='\0';}
     
  else out_file_d=out_file;


}*/



