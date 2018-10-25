#include <stdio.h>
#include <fcntl.h>
#include <netdb.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
 #include <sys/stat.h>
#include <signal.h>
#define HOSTLEN 256
#define BACKLOG 1
#define WW 80
#define HH 23
#define TOP 2  
#define L 9999 //bullte number
#define TOTALREBOOT 10 // total reboot needs to kill
#define ONECREBONCE 7 // once show reboots numbers
#define PLAYERX 20   //init player.x
#define PLAYERY 20   //init player.y
#define PLAYERLIFE 5 //init player.life
#define REBOOTX 5   // init reboot.x
#define REBOOTY 0   // init reboot.y
#define REBOOTLIFE 1  // init reboot.life
#define REBOOTO 1 //init reboot.to
#define MAXROOM 2 // the max number about room
#define MAXFD 2 //the max number of a room's fds


int mk_service_socket(int port);
void * change(void * fd);
void * listen_c(void *);
void * sending(void *);
void proccess(char * cmd,int fd);
void auto_ops();
void sendms(int fd);
int set_ticker();
int randint(int );


pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
struct ROOM{
	int id;// room number
	int reboot;
	int totalReboot;
	int up;
	int down;
	int player;//player :real number
	int num;// total player
	int fds[MAXFD];
	int using;// 1 :is using ;0 not using
	pthread_mutex_t lock;
struct point{
	int x;
	int y;
};
struct air{
	struct point p;
	int to; // 1 :to right ;0 to left
	int life;
}players[MAXFD],reboots[TOTALREBOOT];
struct bullet{
	struct point p;
	int toward;//0 up ,1 down
	int over;// 0 :can use ;i=1 using
}ups[L],downs[L];
	
//pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
};
void showroom(struct ROOM r);
struct  ROOM rooms[MAXROOM];
int fds[MAXFD*MAXROOM];
int num=0;//fds number ,include game over
int realnum=0;
int room =0;//the real number about rooms

//pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_lock
int fdtoindex(int fd,int sit[]){
	int i,j;
	for(i=0;i<MAXROOM  ;i++){
		if(rooms[i].using==1){
			for(j=0;j<MAXFD;j++){
				if(rooms[i].fds[j] == fd){
					sit[0]=i;
					sit[1]=j;
					return 0;
				}
			}
		}
	}
	return 1;
}

int fdtoindex1(int fd){
	int i;
	for(int i=0;i<num;i++)
		if(fds[i]==fd)
			return i;
}
void init(){

int i,j;
for(i=0;i<MAXFD*MAXROOM;i++)
fds[i]=0;
for(i=0;i<MAXROOM;i++){
rooms[i].id=0;
rooms[i].using=0;
}

}
int main(){
	pthread_t p,pp;
	init();
	int sock =mk_service_socket(8888),i;
	if(sock==-1)
		exit(1);
	perror("sock");
	fflush(NULL);
	pthread_create(&p,NULL,listen_c ,NULL);
	perror("over");
	fflush(NULL);
	while(1){
		int fd=accept(sock, NULL, NULL); 
		printf("come in fd:%d \n",fd);
		fflush(NULL);
		pthread_mutex_lock(& lock);
		for(i=0;i<num&& fds[i]!=0;i++);		
		fds[i] = fd;
		num++;	
		realnum++;
		if(num ==1){
			pthread_create(&pp,NULL,sending ,NULL);
		}
		pthread_mutex_unlock(& lock);
	}
	return 0;
}

int findmax(int f[],int l){
	int i,max=0;
	for(i=0;i<l;i++){
		if(max<f[i])
			max=f[i];
	}
	return max;
}
void * sending(void * a){
	pthread_detach(pthread_self());
	signal(SIGALRM,auto_ops);
	set_ticker();
}
void  *listen_c(void * v){
	pthread_t pp;
	int i =0 ,number =0;
	int  retval;
	struct timeval timeout ;
	fd_set readfds ;
	struct stat buf;
	while(1){
		pthread_mutex_lock(& lock);
		number = num;//num	
		pthread_mutex_unlock(& lock);
		FD_ZERO(&readfds);
		for (i=0;i<number;i++){
		   if(fds[i]!=0&&fstat(fds[i],&buf)==0)
		   	FD_SET(fds[i],&readfds);
		   if(fstat(fds[i],&buf)!=0){
			close(fds[i]);
		   	fds[i]=0;
		   }
		}
		timeout.tv_sec = 0;
		timeout.tv_usec =1 ;
		retval = select(findmax(fds,number)+1,&readfds , NULL, NULL,&timeout);
		
		fflush(NULL);
		if(retval == -1)
			perror("select");
		else if(retval>0){
		       for(i =0 ;i< number ;i++)
		            if(fds[i]!=0 && FD_ISSET(fds[i],&readfds)){
				 printf("occure some fd:%d\n\n",fds[i]);				 
				 pthread_create(&pp,NULL,change ,&fds[i]);
				 sendms(0);
				}

		}
	}
}


void auto_ops(){
	printf("come into auto\n\n");
	fflush(NULL);
	int i=0 ,j,k;
	for(i =0;i<MAXROOM;i++){
		if(rooms[i].using == 0)
			continue;
		pthread_mutex_lock(&(rooms[i].lock));
		showroom(rooms[i]);
		//move bullet
		// ups
		for(j=0;j<rooms[i].up%L;j++){
			if(rooms[i].ups[j].over ==1){
				for(k=0;k<rooms[i].totalReboot;k++){
					if( rooms[i].reboots[k].life>0 && rooms[i].reboots[k].p.y== rooms[i].ups[j].p.y && rooms[i].reboots[k].p.x==rooms[i].ups[j].p.x){
						rooms[i].reboots[k].life--;
						rooms[i].ups[j].over=0;
						if(rooms[i].reboots[k].life==0){
							rooms[i].reboot--;
						}
						break;
					}
				}//
				rooms[i].ups[j].p.x--;
				if(rooms[i].ups[j].p.x==TOP)
					rooms[i].ups[j].over =0;
			}//if up.over
		}//ups

		// down
		for(j=0;j<rooms[i].down%L;j++){
			if(rooms[i].downs[j].over ==1){
				for(k=0;k<MAXFD;k++){
					if(rooms[i].fds[k]!= 0 &&rooms[i].players[k].life>0 && rooms[i].players[k].p.y==rooms[i].downs[j].p.y && rooms[i].players[k].p.x==rooms[i].downs[j].p.x){
						rooms[i].players[k].life--;
						rooms[i].downs[j].over=0;
						if(rooms[i].players[k].life==0){
							rooms[i].player--;
						}
						break;
					}
				}//
				rooms[i].downs[j].p.x++;
				if(rooms[i].downs[j].p.x==HH)
					rooms[i].downs[j].over =0;
			}//if up.over
		}//ups


		//reboot move
		for(j=0;j<rooms[i].totalReboot;j++){
			if(rooms[i].reboots[j].life>0){
				if(randint(10)>5){
					if(rooms[i].reboots[j].to==1){// to right
						if(rooms[i].reboots[j].p.y+1<WW){
							rooms[i].reboots[j].p.y++;
						}else{
							rooms[i].reboots[j].to=0;
						}
					}else{//to left
						if(rooms[i].reboots[j].p.y+1>0){
							rooms[i].reboots[j].p.y--;
						}else{
							rooms[i].reboots[j].to=1;
						}						

					}//left
				}// move
				if(randint(100)>95){//if(randint(100)>95){//fire
					rooms[i].downs[rooms[i].down%L].p.x=rooms[i].reboots[j].p.x+1;
					rooms[i].downs[rooms[i].down%L].p.y=rooms[i].reboots[j].p.y;
					rooms[i].downs[rooms[i].down%L].over=1;
					rooms[i].down++;
				} 
				if(randint(1000)>997){//down
					rooms[i].reboots[j].p.x++;
				}
			}//life>0
		}

		// gen reboot
		if(rooms[i].totalReboot <TOTALREBOOT &&rooms[i].reboot <ONECREBONCE){
			rooms[i].reboots[rooms[i].totalReboot].p.x =REBOOTX;//+randint(10);	
			rooms[i].reboots[rooms[i].totalReboot].p.y =REBOOTY;///randint(WW);
			rooms[i].reboots[rooms[i].totalReboot].life =REBOOTLIFE;	
			rooms[i].reboots[rooms[i].totalReboot].to = REBOOTO;
			rooms[i].reboot++;
			rooms[i].totalReboot++;
		}
		pthread_mutex_unlock(&rooms[i].lock);
	}//rooms
        sendms(0);
}

/*
sending message to all players
play;
x y l;
up+down;
x y;

*/
void sendms(int fd){
	printf("com int to send message\n\n");
	fflush(NULL);
	int i ,j,k=0;
	char buf[100000] ,t[100000] ,me[100000];
	for(i=0;i<MAXROOM;i++){/// consider room living
		if(rooms[i].using==0)
			continue;
		sprintf(buf,"0; %d ;",rooms[i].player);
		pthread_mutex_lock(&(rooms[i].lock));
		for( j =0 ;j<MAXFD ;j++){
			if(rooms[i].fds[j]!= 0 && rooms[i].players[j].life > 0){
				sprintf(buf,"%s %d %d;",buf,rooms[i].players[j].p.x,rooms[i].players[j].p.y);
			}//if
		}//for num

		sprintf(buf,"%s %d;",buf,rooms[i].reboot);
		for(j= 0 ;j<rooms[i].totalReboot;j++){
			if(rooms[i].reboots[j].life >0)
				sprintf(buf ,"%s %d %d;",buf,rooms[i].reboots[j].p.x,rooms[i].reboots[j].p.y);
		}//for totalReboot
		k =0;
		strcpy(t,"");
		for(j=0;j<rooms[i].up%L;j++){
			if(rooms[i].ups[j].over ==1){
				k++;
				sprintf(t,"%s %d %d;",t,rooms[i].ups[j].p.x,rooms[i].ups[j].p.y);
			}//if
		}//for up
		for(j=0;j<rooms[i].down%L;j++){
			if(rooms[i].downs[j].over==1){
				k++;
				sprintf(t,"%s %d %d;",t,rooms[i].downs[j].p.x,rooms[i].downs[j].p.y);
			}
		}
		sprintf(buf,"%s %d ;%s",buf,k,t);

		for (j =0 ;j<MAXFD;j++){
			strcpy(me,"");
			if(rooms[i].fds[j]!=0){
				if(rooms[i].players[j].life >0){
					if(rooms[i].totalReboot == TOTALREBOOT &&rooms[i].reboot == 0)
						sprintf(me,"fd:%d,life:%d,win,room:%d,again input r;",rooms[i].fds[j],rooms[i].players[j].life,rooms[i].id);
					else{
						sprintf(me,"fd:%d,life:%d,total %d,reboot %d,room:%d;",rooms[i].fds[j],rooms[i].players[j].life,rooms[i].totalReboot,rooms[i].reboot,rooms[i].id);
					}
				}else{
					sprintf(me,"fd :%d ,game over,room:%d,again input r;",rooms[i].fds[j],rooms[i].id);
				}
				sprintf(t,"%s %s",buf,me);
				write(rooms[i].fds[j],t,strlen(t));
				printf("room:%d ,sned:%s\n\n",room,t);
			}//if fds		
		}//rooms
		pthread_mutex_unlock(&rooms[i].lock);
		//printf("room:%d ,sned:%s\n\n",room,buf);
	fflush(NULL);
	}//room
	
}
/*
reading message sent from player
*/
void * change(void * fd){
	int n,i ;
	char buf[100000] ,t[100];
	n = read(*(int *)fd,&buf,1024);
	proccess(buf,*(int *)fd);
}

// change the attributes by players op
void proccess(char * cmd,int fd){
	printf("come int to revice messgae %s\n\n",cmd);
	fflush(NULL);
	printf(" fd :%d   cmd: %s \n",fd,cmd);
 	char  *ptr,*t;
	int sit[2],r,index,flag=0,exist,i,k;
	exist=fdtoindex(fd,sit);//get return value to confire exsist
	if(exist){// no this player;   r,index can not use
		if(strncmp(cmd,"#",1)==0){
			ptr=strtok_r(cmd,"#",&t);
			flag=0;
			for(i=0;i<MAXROOM;i++){
				if(rooms[i].id==atoi(ptr)&& rooms[i].using==1 && rooms[i].player<MAXFD){// room using
					for(k=0;rooms[i].fds[k]!=0 ;k++)
						;
					index =k;
					printf("room using %d \n",index);
					flag=1;
					rooms[i].fds[index]=fd;
					rooms[i].players[index].p.x=PLAYERX;
					rooms[i].players[index].p.y=PLAYERY;	
					rooms[i].players[index].life=PLAYERLIFE;
					if(rooms[i].num <MAXFD)
						rooms[i].num++;
					rooms[i].player++;
				}else if(rooms[i].id==atoi(ptr)&& rooms[i].using==1 && rooms[i].player>=MAXFD){
					write(fd,"1;the room is full\0;\0",20);
					flag=1;
					fds[fdtoindex1(fd)]=0;
				}
			}
			if(!flag){// no this room
				if(room<MAXROOM){
					for(k=0;rooms[k].using==1;k++);
						r=k;
					printf("creatr room\n");
					rooms[r].id=atoi(ptr);
					rooms[r].using =1;
					rooms[r].reboot=0;
					rooms[r].totalReboot =0;
					rooms[r].up=0;
					rooms[r].down=0;
					rooms[r].player=0;
					rooms[r].num=0;
					rooms[r].fds[0]=fd;
					rooms[r].players[0].p.x=PLAYERX;
					rooms[r].players[0].p.y=PLAYERY;
					rooms[r].players[0].life=PLAYERLIFE;
					showroom(rooms[r]);
					rooms[r].num++;
					rooms[r].player++;
					room++;

				}else{//room is full
					//return wanning rewritewritr
					write(fd,"1;can not create new room;\0",30);
					
					fds[fdtoindex1(fd)]=0;
					
				}
			}
		}//if #

	}else{//
		r = sit[0];
		index=sit[1];
		pthread_mutex_lock(&rooms[r].lock);
		if(strncmp(cmd,"w",1)==0){
			if(rooms[r].players[index].p.x-1>TOP){
				rooms[r].players[index].p.x-=1;
			}
		}else if(strncmp(cmd,"s",1)==0){
			if(rooms[r].players[index].p.x+1<HH){
				rooms[r].players[index].p.x+=1;
				}
		}else if(strncmp(cmd,"a",1)==0){
			if(rooms[r].players[index].p.y-1>0){
				rooms[r].players[index].p.y-=1;
			}
		}else if(strncmp(cmd,"d",1)==0){
			if(rooms[r].players[index].p.y+1<WW){
				rooms[r].players[index].p.y+=1;
			}
		}else if(strncmp(cmd,"j",1)==0){
			if(rooms[r].players[index].p.x-2 >TOP){
				rooms[r].ups[rooms[r].up%L].p.x=rooms[r].players[index].p.x-1;
				rooms[r].ups[rooms[r].up%L].p.y=rooms[r].players[index].p.y;
				rooms[r].ups[rooms[r].up%L].over =1;
				rooms[r].up++;
			}
		}else if(strncmp(cmd,"0",1)==0){
			close(fd);
			rooms[r].fds[index]=0;
			fds[fdtoindex1(fd)]=0;
			rooms[r].player--;
			realnum--;
			if(rooms[r].player==0){
				rooms[r].using=0;
				room--;
				rooms[r].id=0;
			}
		}else if(strncmp(cmd,"r",1)==0){
			rooms[r].up=0;
			rooms[r].down=0;
			rooms[r].reboot =0;
			rooms[r].totalReboot =0;
			for(i = 0;i<MAXFD;i++){
				if(rooms[r].fds[i]!=0){
					rooms[r].players[index].p.x=PLAYERX;
					rooms[r].players[index].p.y=PLAYERY;	
					rooms[r].players[index].life=PLAYERLIFE;			
				}
			}	
		
		} 
		pthread_mutex_unlock(&rooms[r].lock);

	}//else	
	for(i=0;i<MAXROOM;i++){
		showroom(rooms[i]);
	}
}


int mk_service_socket(int port){
	struct sockaddr_in saddr ;
	struct hostent *hp;
	char hostname[HOSTLEN] ;
	int sock ;
	sock = socket(PF_INET,SOCK_STREAM,0);
	if( sock ==-1){
		perror("sock is error");
		return -1;
	}
	bzero((void*)&saddr,sizeof(saddr));
	gethostname(hostname,HOSTLEN);
	hp = gethostbyname(hostname);

	bcopy((void *)hp->h_addr_list[0],(void*)&saddr.sin_addr,hp->h_length);
	saddr.sin_port = htons(port);
	saddr.sin_family = AF_INET;
	if(bind(sock ,(struct sockaddr *)&saddr ,sizeof(saddr)) !=0){
		perror("bind error");
		return -1;
	}
	if(listen(sock,BACKLOG) != 0){
		perror(" listen error");
		return -1;
	}
	return sock;
}

int set_ticker(){
	int  n_msecs =100;
        struct itimerval new_timeset;
        long    n_sec, n_usecs;
        n_sec = n_msecs / 1000 ;
        n_usecs = ( n_msecs % 1000 ) * 1000L ;
        new_timeset.it_interval.tv_sec  = n_sec;        /* set reload  */
        new_timeset.it_interval.tv_usec = n_usecs;      /* new ticker value */
        new_timeset.it_value.tv_sec     = n_sec  ;      /* store this   */
        new_timeset.it_value.tv_usec    = n_usecs ;     /* and this     */
	return setitimer(ITIMER_REAL, &new_timeset, NULL);
}

int randint(int seed){
	return rand()%seed;
}

void showroom(struct ROOM r){
	char fff[100];
	int i;
	strcpy(fff,"");
	printf("room id:%d,num:%d,player:%d,using:%d\n",r.id,r.num,r.player,r.using);
	for(i=0;i<num;i++){
		sprintf(fff,"%s\t%d,",fff,fds[i]);
	}
	printf("totla fds :%s\n",fff);
	strcpy(fff,"");
	for(i=0;i<MAXFD;i++){
		sprintf(fff,"%s\t%d,",fff,r.fds[i]);
	}
	printf("room fds :%s\n",fff);
}
