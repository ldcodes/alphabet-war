# include<stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
# include<signal.h>
# include<unistd.h>
int s1 , s2,f;

void d(){
	kill(s1,10);
	kill(s2,10);
	printf("begin\n");
}

void s(){
	kill(f,12);
}
void ss(){

	printf("%d saller recive the sign from driver\n",getpid());
	kill(f,12);    
}
void dd(){
	printf("driver recive the sign from saller\n");
}
int main(){
	int exitcode ;
	f = getpid();
	while((	s1 = fork())==-1);
	if(s1 == 0){// saller 1
		signal(2,SIG_IGN);
		signal(10,ss);
		pause();
		printf("s1 over%d\n",getpid());
		exit(0);
	}else{
		while((	s2 = fork())==-1);
		if(s2 == 0){
			signal(2,SIG_IGN);
			signal(10,ss);
			pause();
			printf("s2 over%d\n",getpid());
			exit(0);
		}else{
			//sleep(0);//import !!!
			printf("p %d ,s1 %d ,S2 %d\n",f,s1,s2);
			signal(2,d);
			signal(12,dd);
			d();
			pause();
		//	sleep(1);
			printf("main over\n");

			exit(0);//import !!!
		}
	}
return 0;
}

