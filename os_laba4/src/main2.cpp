#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdint.h>
#include <fstream>
#include <iostream>
#include <semaphore.h>

sem_t semaphore1;
sem_t semaphore2;

char *getstr(char *stream){
	unsigned int maxlen = 64, size = 64;
	char *buffer = (char *)malloc(maxlen);
	if (buffer != NULL){
		char c = EOF;
		int position = 0;
		while((c =*(stream + position)) != '\n' && c != EOF){
			buffer[position] = c;
			position++;
			if (position == size){
				size = position + maxlen;
				buffer = (char *)realloc(buffer, size);
			}
		}
		buffer[position] = '\0'; //терминальный нуль -для обозначения конца строки
	}
	return buffer;
}
int main(){
	std::fstream fs1, fs2;
	int fd;
	std::string path_child1, path_child2;
	struct stat statbuf;
	std::cout<<"Congrats, you are in parent process. Please enter name of first child: "<<std::endl;
	std::cin >> path_child1;
	std::cout<<"For second child: "<<std::endl;
	std::cin >> path_child2;
	std::string string;
	if(sem_init(&semaphore1,1,1) < 0){
		std::cout<<"SEM1 ERROR"<<std::endl;
		exit(-1);
	}
	if(sem_init(&semaphore2,1,1) < 0){
		std::cout<<"SEM2 ERROR"<<std::endl;
		exit(-1);
	}
	if((fd = open("test.txt", O_RDWR | O_CREAT, 0777))==-1){
		std::cout<<"FILE OPEN ERROR"<<std::endl;
		exit(-1);
	}
	char *src;
	if (fstat(fd, &statbuf) < 0){
		std::cout<<"FSTAT ERROR"<<std::endl;
		exit(-1);
	}
	src = (char*)mmap(nullptr, statbuf.st_size,PROT_READ| PROT_WRITE,MAP_SHARED,fd,0);
	if(src == MAP_FAILED){
		std::cout<<"MAPPING ERROR"<<std::endl;
		exit(-1);
	}
	int child1, child2;
	if((child1 = fork())==-1){
		std::cout<<"FORK1 ERROR"<<std::endl;
		exit(-1);
	}
	else if(child1 == 0){
		sem_wait(&semaphore1);
		fs1.open(path_child1,std::fstream::in |std::fstream::out | std::fstream::app);
		int pos = 0;
		char* str;
		std::string k;
		while((str = getstr(src+pos)) && str[0] != '\0'){
			int lenght = strlen(str);
			if(lenght <=10){
				std::string k;
				for(int i = 0; i <lenght; i++){
					k[i] = str[i];
				}
				for(int i = 0; i<lenght/2;i++){
					char tmp = k[i];
					k[i] = k[lenght-i-1];
					k[lenght-i-1] = tmp;
				}
				for(int i = 0; i<lenght; i++){
					fs1<<k[i];
				}
				fs1<<'\n';
			}
			pos+=lenght;
			if (src[pos] =='\n') pos++;
			free(str);
		}
		free(str);
		sem_post(&semaphore1);
	}
	else{
		if ((child2 = fork())==-1){
			std::cout<<"FORK2 ERROR"<<std::endl;
			exit(-1);
		}
		else if(child2 == 0){
			sem_wait(&semaphore2);
			fs2.open(path_child2, std::fstream::in | std::fstream::out | std::fstream::app);
			int pos = 0;
			char* str;
			std::string k;
			while((str = getstr(src+pos))&&str[0] != '\0'){
				int lenght = strlen(str);
				if(lenght > 10){
					for(int i = 0; i<lenght;i++){
						k[i] = str[i];
					}
				        for(int i = 0; i<lenght/2;i++){
						char tmp = k[i];
						k[i] = k[lenght-i-1];
						k[lenght-i-1] = tmp;
					}
					for (int i = 0; i<lenght; i++){
						fs2 << k[i];
					}
					fs2 <<'\n';
				}
				pos+=strlen(str);
				if(src[pos] == '\n') pos++;
				free(str);
			}
			free(str);
			sem_post(&semaphore2);
		}
		else{
			sem_wait(&semaphore1);
			sem_wait(&semaphore2);
			close(fd);
			if(munmap(src, statbuf.st_size) < 0){
				std::cout<<"UNMAPPING ERROR"<<std::endl;
				exit(-1);
			}
			if(sem_destroy(&semaphore1) < 0){
				std::cout<<"DELSEM1 ERROR"<<std::endl;
				exit(-1);
			}
			if(sem_destroy(&semaphore2) < 0){
				std::cout<<"DELSEM2 ERROR"<<std::endl;
				exit(-1);
			}
		}
	}
}


