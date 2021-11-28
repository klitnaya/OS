#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <fstream>
#include <string>
#include <sstream>
#include <signal.h>
#include <pthread.h>
#include <stdint.h>
#include <semaphore.h>
#define ll long long

sem_t semaphore1;
sem_t semaphore2;
std::fstream fs1, fs2;
	
void pr1( char* mapped_file1, int s){
        sem_wait(&semaphore1);
        fs1.open("f1.txt", std::fstream::in |std::fstream::out | std::fstream::app);
		if(!fs1.is_open()){
			exit(EXIT_FAILURE);
		}
		std::string st;
		for(int i = 0; i<s;i++){
		    st += mapped_file1[i];
		}
        std::cout<< st <<std::endl;
   		int first_identificator = fork();
   		std::cout<<"Congrats, you are in child1 process"<<std::endl; fflush;
		if(first_identificator == -1){
			std::cout<<"Fork error!" << std::endl;
			exit(EXIT_FAILURE);
		}
		else if(first_identificator == 0){
			std::cout<<"Congrats, you are in child1 process"<<std::endl;
			fs1 << mapped_file1 << std::endl;
			sem_post(&semaphore1);
	}
}

void pr2( char* mapped_file2, int s){
    	sem_wait(&semaphore2);
    	fs2.open("f2.txt", std::fstream::in |std::fstream::out | std::fstream::app);
		if(!fs2.is_open()){
			exit(EXIT_FAILURE);
		}
		std::string st;
		for(int i = 0; i<s;i++){
		    st += mapped_file2[i];
		}
		int second_identificator = fork();
		if(second_identificator == -1){
			std::cout<<"Fork error!" << std::endl;
			exit(EXIT_FAILURE);
		}
		else if(second_identificator == 0){
			//std::cout<<"Congrats, you are in child2 process"<<std::endl;
			int i = 0;
			fs2 << mapped_file2 << std::endl;
			sem_post(&semaphore2);
	
}
}
int main(){
	std::cout <<"Congrats, you are in parent process.  "<< std::endl;
	int fd1;
	int fd2;
    int first_pos = 0;
    int second_pos = 0;
  
	//std::fstream fs1, fs2;
	std::string path_child1, path_child2;
/*	std::cout << "Enter name of file for first child: "<<std::endl;
	std::cin >> path_child1;
	std::cout << "For second child: "<<std::endl;
	std::cin >> path_child2;*/
	std::string string;
        if (sem_init(&semaphore1,1,1) < 0){
		std::cout<<"semafore1 error"<<std::endl;
		exit(-1);
	}
	if (sem_init(&semaphore2,1,1) < 0){
		std::cout<<"semaphore2 error"<<std::endl;
		exit(-1);
	}
	if ((fd1 = open("f1.txt", O_RDWR| O_CREAT, 0777))==-1){
		std::cout <<"Error:can not open the f1.txt. Try again later."<<std::endl;
		exit(EXIT_FAILURE);
	}
	if ((fd2 = open("f2.txt", O_RDWR|O_CREAT,0777))==-1){
		std::cout <<"Error: can not open the f2.txt. Try again later."<<std::endl;
		exit(EXIT_FAILURE);
	}

        
        

    char *mapped_file1 = (char *)mmap(nullptr, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED, fd1, 0); // при помощи мемори маппа отображаем mapped file на оперативную память
    char *mapped_file2 = (char *)mmap(nullptr, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED, fd2, 0);
	
	if (mapped_file1 == MAP_FAILED){
		std::cout<<"An error with mmap function one has been detected"<<std::endl;
		exit(EXIT_FAILURE);
	}
	else {
	    std::cout<<"mapped_file is allright\n";
	}
	if (mapped_file2 == MAP_FAILED){
		std::cout<<"An error with mmap function two has been detected"<<std::endl;
		exit(EXIT_FAILURE);
	}
	std::cout<<"Please, enter your number: "<<std::endl;
	int number = 0;
	std::cin >> number;
	while(number != -1){
		
	    // std::cout<<"number="<<number<<std::endl;
		//string = string + "\n";
		if (number % 2 == 0){
			std::cout<<"we in process 1"<<std::endl;
			string = std::to_string(number);
			std::cout<<"string  "<<string<<std::endl;
			int l = string.size();
			std::cout<<"size = "<<l<<std::endl;
			for (int i = 0; i<l; ++i){
			    fflush;std::cout<<"i = "<<i<<": ";fflush;
			    mapped_file1[first_pos++] = string[i];
			    //std::cout<<"mp = "<<mapped_file1<<std::endl;fflush;
			}
			pr1(mapped_file1, first_pos);
			std::cout<<"the end of process 1"<<std::endl;
		}
		else{
			std::cout<<"we in process 2"<<std::endl;
			string = std::to_string(number);
			int l = string.size();
			for (int i = 0; i<l; i++){
			   mapped_file2[second_pos++] = string[i];
			}
			//std::cout<<"mp = "<< mapped_file2[second_pos]<<std::endl;
			//pr2(number);
			pr2(mapped_file2, second_pos);
		}
		std::cin >> number;
	}
/*	int first_identificator = fork();
	if (first_identificator == -1){
		std::cout<<"Fork error!"<<std::endl;
		exit(EXIT_FAILURE);
	}
	else if(first_identificator == 0){
		sem_wait(&semaphore1);
		int i = 0;
		fs1 << mapped_file1[i-1] << std::endl;
		sem_post(&semaphore1);
	}
	else {
		sem_wait(&semaphore2);
		int second_identificator = fork();
		if(second_identificator == -1){
			std::cout<<"Fork error!" << std::endl;
			exit(EXIT_FAILURE);
		}
		else if(second_identificator == 0){
			//std::cout<<"Congrats, you are in child2 process"<<std::endl;
			int i = 0;
			fs2 << mapped_file2[i-1] << std::endl;
			sem_post(&semaphore2);
			
		}
		else{*/
			if (munmap(mapped_file1, getpagesize()) == -1){
				std::cout<<"Munmap1 error has been dected!"<<std::endl;
				exit(EXIT_FAILURE);
			}
			if (munmap(mapped_file2, getpagesize()) == -1){
				std::cout<<"Munmap2 error has been dected!"<<std::endl;
				exit(EXIT_FAILURE);
			}
			close(fd1);
			close(fd2);
			if(sem_destroy(&semaphore1)<0){
				std::cout<<"DELSEM1 error"<<std::endl;
				exit(-1);
			}
			if(sem_destroy(&semaphore2)<0){
				std::cout<<"DELSEM2 error"<<std::endl;
				exit(-1);
			}
			remove("f1.txt");
			remove("f2.txt");
			return 0;
		}
/*	}
}*/


