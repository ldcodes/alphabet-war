#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
void process_rq(char * rq,int fd);
void header(FILE *fp,char *content_type);
void cannot_do(int fd);
void do_404(char *item, int fd);
int isdir(char *f);
int not_exist(char *f);
void do_ls(char *dir, int fd);
char * file_type(char *f);
int ends_in_cgi(char *f);
void do_exec( char *prog, int fd );
void  do_cat(char *f, int fd);
void read_tile_cml(FILE *fp);

int service_sock(int port){
	int HOSTLEN =256,BACKLOG=1;
	struct sockaddr_in saddr ;
	struct hostent *hp;
	char hostname[256] ;
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
        //printf("ip addr :%s\n", hp->h_addr_list[0]);
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
int main(int ac ,char * av[]){
	int sock ,fd,t;
	FILE *fpin;
	char request[1024];
	if(ac==1){
		fprintf(stderr,"need prot\n");
		exit(1);

	}
	t= atoi(av[1]);
	sock = service_sock(t);
	if(sock == -1)
		exit(2);

	while(1){
		fd = accept(sock ,NULL,NULL);
		fpin = fdopen(fd,"r");

		fgets(request,1024,fpin);
		printf("got a call request = %s",request);
		read_tile_cml(fpin);
		process_rq(request, fd);
		fclose(fpin);

	}

}



void read_tile_cml(FILE *fp){
	char buf[1024];
	while(fgets(buf,1024,fp)!= NULL && strcmp(buf,"\r\n")!=0);
}

void process_rq(char * rq,int fd){
	char cmd[1024] ,arg[1024];
//printf("com in");
	if(fork()!= 0)
		return ;
	strcpy(arg,"./");
	if(sscanf(rq,"%s %s",cmd,arg+2) != 2)
		return ;
	if(strcmp(cmd,"GET")!=0)
		cannot_do(fd);
	else if(not_exist(arg))
		do_404(arg,fd);
	else if(isdir(arg))
		do_ls(arg,fd);
	else if(strstr(arg+1,"."))
		do_cat(arg,fd);
	else
		do_exec(arg,fd);
}
void header(FILE *fp,char *content_type){
	fprintf(fp,"HTTP/1.0 200 OK \r\n");
	if(content_type)
		fprintf(fp,"Content-type: %s\r\n", content_type);
}
void cannot_do(int fd){
	FILE * fp = fdopen(fd,"w");
	fprintf(fp,"HTTP/1.0 501 Not Implemented\r\n");
	fprintf(fp, "Content-type: text/plain\r\n");
	fprintf(fp, "\r\n");
	fprintf(fp, "That command is not yet implemented\r\n");
	fclose(fp);
}
void do_404(char *item, int fd){
	FILE *fp = fdopen(fd,"w");
	fprintf(fp, "HTTP/1.0 404 Not Found\r\n");
	fprintf(fp, "Content-type: text/plain\r\n");
	fprintf(fp, "\r\n");
	fprintf(fp, "The item you requested: %s\r\nis not found\r\n",
			item);
	fclose(fp);
}
int isdir(char *f){
	struct stat info;
	return ( stat(f, &info) != -1 && S_ISDIR(info.st_mode) );
}
int not_exist(char *f){
	struct stat info;
	return( stat(f,&info) == -1 );
}
void do_ls(char *dir, int fd){
	FILE *fp ;
	fp = fdopen(fd,"w");
	header(fp, "text/plain");
	fprintf(fp,"\r\n");
	fflush(fp);
	dup2(fd,1);
	dup2(fd,2);
	close(fd);
	execlp("ls","ls","-l",dir,NULL);
	perror(dir);
	exit(1);
}
char * file_type(char *f){
	char *cp;
	if ( (cp = strrchr(f, '.' )) != NULL )
		return cp+1;
	return "";
}


void do_exec( char *prog, int fd ){
	FILE *fp ;
	fp = fdopen(fd,"w");header(fp, NULL);
	fflush(fp);
	dup2(fd, 1);
	dup2(fd, 2);
	close(fd);
	execl(prog,prog,NULL);
	//perror(prog);
        exit(0);
}
void do_cat(char *f, int fd){
	char *extension = file_type(f);
	char *content = "text/plain";
	FILE *fpsock, *fpfile;
	int c;
	if ( strcmp(extension,"html") == 0 )
		content = "text/html";
	else if ( strcmp(extension, "gif") == 0 )
		content = "image/gif";
	else if ( strcmp(extension, "jpg") == 0 )
		content = "image/jpeg";
	else if ( strcmp(extension, "jpeg") == 0 )
		content = "image/jpeg";
	fpsock = fdopen(fd, "w");
	fpfile = fopen( f , "r");   
        //perror("open:");
	if ( fpsock != NULL && fpfile != NULL )
	{      

		header( fpsock, content );               
		fprintf(fpsock, "\r\n");
		while( (c = getc(fpfile) ) != EOF )
			putc(c, fpsock);
		fclose(fpfile);
		fclose(fpsock);
	}
	exit(0);
}
