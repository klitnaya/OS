#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <vector>
#include <fcntl.h>
#include "funcs.hpp"
#include <map>
#include <vector>

int in(std::vector<std::string> logins, std::string str)
{
    for (int i = 0; i < logins.size(); ++i)
    {
        if (logins[i] == str)
            return  i;
    }
    return -1;
}

int main()
{
//    std::vector<std::vector<std::string>> draft;

    std::vector<std::string> logins;
    std::string command;
    std::string login;

    //ввод логинов
    std::cout << "Enter all user's logins. insert 'end' to stop:\n";
    while (login != "end")
    {
        std::cin >> login; 
        if (in(logins, login) == -1){
            logins.push_back(login);
	}
        else
            std::cout << "already exists!";
    }

    //создание входного FIFO
    if (mkfifo("input", 0666) == -1){
	if (errno != EEXIST){
		perror("input");
          exit(1);
	}
    }


    //создание выходных FIFO для всех логинов
    for (int i = 0; i < logins.size(); ++i)
    {
        if (mkfifo(logins[i].c_str(), 0777) == -1) {
                perror(logins[i].c_str());
		//std::cout<<errno<<std::endl;
                //std::cout<<"errno EEXIST "<< EEXIST<<std::endl;
            if (errno != EEXIST)
            {
                //std::cout << "FIFO WAS NOT CREATED";
                exit(1);
            }
        }
    }
    int fd_recv = open("input", O_RDWR);
    if (fd_recv == -1)
    {
        std::cout << "inPUT FIFO WAS NOT OPENED";
        exit(1);
    }

    //открытие всех FIFO на запись
    int fd[logins.size()];
    for (int i = 0; i < logins.size(); ++i)
    {
        fd[i] = open(logins[i].c_str(), O_RDWR);
    }

    while (1)
    {

        std::string message;
        message = recieve_message_server(fd_recv);
        std::cout << message;
        std::string my_log = extract_login(message);          //отправитель
        std::string rec_log = extract_addressee(message); //получатель
        std::string rcvd_message = extract_message(message);    //сообщение
        int repl_fd = in(logins, rec_log);                //id получателя
        int fd_usr = in(logins, my_log);   	//id отправителя
	//std::cout<<"log= "<<rec_log<<std::endl;
            if (in(logins, rec_log) == -1)
            {
                send_message(fd[fd_usr], "Login does not exists!\n");
            }
            else
            {
                send_message(fd[repl_fd], rcvd_message);
            }
        }
}
