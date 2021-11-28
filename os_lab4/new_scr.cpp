#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
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
#include <semaphore.h>
#define ll long long

sem_t semaphore1;
sem_t semaphore2;


int main() {
    int n = 0;
    int m = 0;
    int first_pos = 0;
    int second_pos = 0;
    int first = 0;
    int second = 0;
    int fd1;
    int fd2;
    int number;
    std::fstream fs1, fs2;
    std::string path_child1, path_child2;
    std::cout << "Enter name of file for first child: " << std::endl;
    std::cin >> path_child1;
    std::cout << "For second child: " << std::endl;
    std::cin >> path_child2;
    std::string string;
    if (sem_init(&semaphore1,1,1) < 0){
		std::cout<<"semafore1 error"<<std::endl;
		exit(-1);
	}
	if (sem_init(&semaphore2,1,1) < 0){
		std::cout<<"semaphore2 error"<<std::endl;
		exit(-1);
	}
    if ((fd1 = open("f1.txt", O_RDWR| O_CREAT, 0777)) == -1)
    {
        std::cout << "Error: can not open the f1.txt. Try again later." << std::endl;
        exit(EXIT_FAILURE);
    }
    if ((fd2 = open("f2.txt", O_RDWR| O_CREAT, 0777)) == -1)
    {
        std::cout << "Error: can not open the f2.txt. Try again later." << std::endl;
        exit(EXIT_FAILURE);
    }
    char *mapped_file1 = (char *)mmap(nullptr, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED, fd1, 0); // при помощи мемори маппа отображаем mapped file на оперативную память
    char *mapped_file2 = (char *)mmap(nullptr, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED, fd2, 0); // при помощи мемори маппа отображаем mapped file на оперативную память
    if (mapped_file1 == MAP_FAILED)
    {
        std::cout << "An error with mmap function one has been detected" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (mapped_file2 == MAP_FAILED)
    {
        std::cout << "An error with mmap function two has been detected" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << "Good. Please enter your strings: " << std::endl;
    std::cin >> number;
    while (number != -1)
    {
        
        string = std::to_string(number);
        string = string + "\n";
        if (number % 2 == 0)
        {
             
            n++;
            first += string.size();
            if (ftruncate(fd1, first))
            {
                std::cout << "Error during ftrancate with mf1 has been detected" << std::endl;
                exit(EXIT_FAILURE);
            }
            for (int i = 0; i < string.size(); ++i)
            {
                mapped_file1[first_pos++] = string[i];
            }
            
        }
        else
        {
            
            m++;
            second += string.size();
            if (ftruncate(fd2, second))
            {
                std::cout << "Error during ftrancate with mf2 has been detected" << std::endl;
                exit(EXIT_FAILURE);
            }
            for (int i = 0; i < string.size(); ++i)
            {
                mapped_file2[second_pos++] = string[i];
            }
        }
        std::cin >> number;
    }
    int first_id = fork();
    if (first_id == -1)
    {
        std::cout << "Fork error!" << std::endl;
        exit(EXIT_FAILURE);
    }
    else if (first_id == 0)
    {
        sem_wait(&semaphore1);
        fs1.open(path_child1, std::fstream::in | std::fstream::out | std::fstream::app);
        if (!fs1.is_open())
        {
            exit(EXIT_FAILURE);
        }
        std::cout << "Congrats, you are in child #1 process" << std::endl;
        int i = 0;
        while (n > 0)
        {
            std::string string;
            while (mapped_file1[i] != '\n')
            {
                string += mapped_file1[i];
                i++;
            }
            if (mapped_file1[i] == '\n')
                i++;
            fs1 << string << std::endl;
            n--;
        }
        sem_post(&semaphore1);
    }
    else
    {
        int second_id = fork();
        if (second_id == -1)
        {
            std::cout << "Fork error!" << std::endl;
            return 4;
        }
        else if (second_id == 0)
        {
             sem_wait(&semaphore2);
            fs2.open(path_child2, std::fstream::in | std::fstream::out | std::fstream::app);
            if (!fs2.is_open())
            {
                exit(EXIT_FAILURE);
            }
            std::cout << "Congrats, you are in child #2 process" << std::endl;
            int i = 0;
            while (m > 0)
            {
                std::string string;
                while (mapped_file2[i] != '\n')
                {
                    string += mapped_file2[i];
                    i++;
                }
                if (mapped_file2[i] == '\n')
                    i++;
                fs2 << string << std::endl;
                m--;
            }
            sem_post(&semaphore2);
        }
        else  //PARENT
        {
            if (munmap(mapped_file1, getpagesize()) == -1)
            {
                std::cout << "Munmap1 error has been dected!" << std::endl;
                exit(EXIT_FAILURE);
            }
            if (munmap(mapped_file2, getpagesize()) == -1)
            {
                std::cout << "Munmap2 error has been dected!" << std::endl;
                exit(EXIT_FAILURE);
            }
            close(fd1);
            close(fd2);
            remove("f1.txt");
            remove("f2.txt");
            return 0;
        }
    }
}
