#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <pthread.h>
#include <curses.h>
#define HOSTLEN 256
#define BACKLOG 1
#define plane "M"
#define bullet "I"
#define reboot "W"
#define W 80
#define H 23

int connect_to_server(char *host, int portnum);
void *talk_s(void * fd);
int listen_s(int fd);
int show(int );
int w(int fd);
void showing(char *,int );
void showingbullet(char * p);
int welcome();

int room;

int main(){
	int fr;
	cbreak();
        initscr();
        clear();
        refresh();
        noecho();
        cbreak();
	curs_set(0);
	int fd ,port =8888;
	pthread_t listen, talk;
	char * host ="computer-INVALID",im[10];
	fr =welcome();
	while(fr){
		//printf("com in cycle %d\n",room);
		fflush(NULL);
		strcpy(im,"");
		//close(fd);
		fd=connect_to_server(host, port); 
		if(fd==-1){
			perror("conncet is error");
			exit(1);
		}
		listen_s(fd);
		fr =welcome();
	}
	write(fd,"0",1);
	curs_set(1);
	nocbreak();
	echo();
	endwin();
	return 0;
}

// send message to service
void* talk_s(void * fd){

	char buf[1024];
	int n ,*p = (int *)fd;
	while(1){
		printf("\ntalk to service %d \n",n);
		scanf("%s" ,buf);
		write(*p,buf,strlen(buf));
		fflush(NULL);
		
	}
	
}

// listen to service
int listen_s(int fd){
	int i =0 ;
	int  retval,r=0;
	char im[10];
	struct timeval timeout ;
	fd_set readfds ;
	sprintf(im,"#%d",room);
	write(fd,im,10);
	//printf("send %s\n",im);
	while(r!=-1){
		FD_ZERO(&readfds);
		//FD_SET(getpid(),&readfds);
		//printf("%d",getpid());
		FD_SET(fd,&readfds);
		FD_SET(0,&readfds);	
		timeout.tv_sec = 0;
		timeout.tv_usec =1 ;
		retval = select(fd+1,&readfds , NULL, NULL,&timeout);
		
		if(retval == -1)
			perror("select");
		if(retval>0){
			//printf(" somr occure\n");
		        if(FD_ISSET(fd,&readfds))
		               r=show(fd);
		        if(FD_ISSET(0,&readfds)){
		               	r= w(fd);
			}

	        }

	}
}

int w(int fd){
	int n;
	char buf[1024];	
	n = read(0,&buf,1024);
	if(strncmp(buf,"w",1)==0 ||strncmp(buf,"s",1)==0 ||strncmp(buf,"a",1)==0 ||strncmp(buf,"d",1)==0 ||strncmp(buf,"j",1)==0||strncmp(buf,"b",1)==0||strncmp(buf,"0",1)==0|strncmp(buf,"r",1)==0){
		write(fd,buf,n);
		fflush(NULL);
	}
	if(strncmp(buf,"0",1)==0){
		return -1;
	}
		return 0;
}
/*
n
play 1
*/
int show(int fd){
	clear();
        erase();
        refresh();	
	refresh();
	static int l =0;
	int n,i,f;
	char buf[999999] ,*ptr,*t,mes[999999];
	strcpy(buf,"");
	n = read(fd,&buf,1024);
	//printf("rev :%s\n",buf);
		fflush(NULL);
	// get control number ;0 is show message;1 is wanning
	ptr = strtok_r(buf, ";",&t);
	if(ptr != NULL)
		n =  atoi( ptr);
	//printf("control:%d \n",n);
	if(0==n){
		//printf("com int 0");
		// get players number
		ptr = strtok_r(NULL, ";",&t);
		if(ptr != NULL)
			n =  atoi( ptr);
		// show players
		for(i=0;i<n;i++){
			ptr = strtok_r(NULL, ";",&t);
			if(ptr != NULL)
				showing(ptr,0);
		}
		// get reboots number
		ptr = strtok_r(NULL, ";",&t);
		if(ptr != NULL)
			n =  atoi( ptr);
		// show reboots
		for(i=0;i<n;i++){
			ptr = strtok_r(NULL, ";",&t);
			if(ptr != NULL)
				showing(ptr,1);
			
		}
		// get bullet numbers
		ptr = strtok_r(NULL, ";",&t);
		if(ptr != NULL)
			n =atoi(ptr);
		// show bullet

		//printf("%d",n);
		for(i=0;i<n;i++){
			ptr = strtok_r(NULL, ";",&t);
			if(ptr != NULL)
			 	showingbullet(ptr);	
		}
		ptr = strtok_r(NULL, ";",&t);
		if(ptr != NULL){
		move(1,1);  
		addstr("                                         ");
		move(1,1); 
		addstr(ptr);
      		refresh(); }
	}else{// need more
		//printf("com in 1");
		ptr = strtok_r(NULL, ";",&t);
		if(ptr != NULL)
		move(10,10);
		addstr(ptr);
      		refresh();
		sleep(2);
		//close(fd);
		return -1;
	}
	return 0;
}

//show bullte ; x,y
void showingbullet(char * p){
	char *t,*ptr;
	int x=0, y=0, life,r;
	ptr = strtok_r(p, " ", &t);
	if(ptr !=NULL)
	x = atoi( ptr);
	
	ptr = strtok_r(NULL, " ", &t);
	if(ptr !=NULL)
	y = atoi( ptr);

	if( x != -1 && y != -1){
		move(x, y);
       		addstr(bullet);
       		refresh();     
	}
}

// show air ;x,y,life
void showing(char * p, int index){
	char *t,*ptr;
	int x, y, life,f,r;
	char *now;
	if(index == 0)
		now = plane;
	else {
		now = reboot;
	}
	ptr = strtok_r(p, " ", &t);
	if(ptr !=NULL)
	x = atoi( ptr);
	
	ptr = strtok_r(NULL, " ", &t);
	if(ptr !=NULL)
	y = atoi( ptr);

	if( x>=0 && y>=0 && x<23 && y<80){//!!!!!!!!!!!!!!!
		move(x,y);
        	addstr(now);
        	refresh();
	}
  
}

int connect_to_server(char *host, int portnum){
	int sock;
	struct sockaddr_in servadd;
	struct hostent
		*hp;
	sock = socket( AF_INET, SOCK_STREAM, 0 );
	if ( sock == -1 ){
		perror("sock is null\n");
		return -1;
	}
	bzero( &servadd, sizeof(servadd) );
	hp = gethostbyname( host );
	if (hp == NULL){
		perror("hp is null\n");
		return -1;
	}
	bcopy(hp->h_addr, (struct sockaddr *)&servadd.sin_addr, hp->h_length);

	servadd.sin_port = htons(portnum);
	servadd.sin_family = AF_INET ;
	if ( connect(sock,(struct sockaddr *)&servadd, sizeof(servadd)) !=0){
		perror("connet is null\n");
		return -1;
	}
	return sock;
}

int welcome(){
	char im1[]="w :up, s:down, d:right, a:left, j:fire ,b:begin,r: restart ;0:exit game ;q exit\n",r,num[10];

	clear();
        erase();
        refresh();
	move(0,0);
	addstr(im1);
	refresh(); 

	//printf("%s\n",im1);
	int i=0;
	strcpy(num,"          ");
	while(1){
		
		//scanf("%s ",read);
		i+=read(0,&r,1);
		if(strncmp(&r,"z",1)==0){
			i--;
			move(5,i);
			addstr(" ");			
			refresh(); 
			num[i]=' ';
			move(7,0);
			addstr(num);
			refresh();
			if(i)
				i--;	
		}else if(strncmp(&r,"q",1)==0){
			return 0;
		}else if(strncmp(&r,"b",1)==0){
			room = atoi(num);
			break;
		}else{
			move(5,i);
			addch(r);
			num[i]=r;
			move(7,0);
			addstr(num);
			refresh();
		}
	}
	return 1;
}
