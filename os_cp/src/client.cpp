#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <vector>
#include <fcntl.h>
#include "funcs.hpp"
#include <thread>

//функция приёма сообщений (для потока)
void func(int fd_recv, std::string login)
{
    while (1)
    {
        std::string reply = recieve_message_client(fd_recv);
        std::cout << reply << "\n";
        std::cout.flush();
        std::cout << login << ">";
        std::cout.flush();
    }
}
int main()
{
    //подключение к входному FIFO сервера
    int fd_send = open("input", O_RDWR);
    if (fd_send == -1)
    {
        std::cout << "ERROR: MAIN FIFO WAS NOT OPENED\n";
        exit(1);
    }

    //подготовка - инструкции, ввод логина
    std::cout << "Hellow. insert logins.\n If you stop the enter logins you can enter 'end'\n";
    std::cout << "Input: [login] [message]. for example like : one\n hey, how are you?\n";
    std::cout << "Insert your login: ";
    std::string login;
    std::vector<std::string> drafts;

    //подключение к персональному именованному пайпу
    int fd_recv = -1;
    while (fd_recv == -1)
    {
        std::cin >> login;
        fd_recv = open(login.c_str(), O_RDWR);
        if (fd_recv == -1){
            std::cout << "Wrong login!\nInsert your login: ";
        }
    };

    //вход успешен, запуск потока принятия сообщений от сервера
    std::string adressee, message;
    std::cout << "Congrats! You have signed. Now you can send messages!\n";
    std::thread thr_recieve(func, fd_recv, login);

    //запуск цикла отправки сообщений на сервер
    while (1){
        std::cout << login << "> ";
        std::cin >> adressee;
        if (adressee == "draft"){
            std::string mesage;
            std::getline(std::cin, mesage);
            drafts.push_back(mesage);
        } 
        else{
        if (adressee == "send_draft"){
            std::string recipient;
            std::getline(std::cin,recipient);
            for(int i = 0; i < drafts.size(); i++){
                send_message_to_server(fd_send, login, recipient, drafts[i]);
            }
        }
        else{
            if (adressee == "quit")
                break;
            std::getline(std::cin, message);
            send_message_to_server(fd_send, login, adressee, message);
        }
    }}
    thr_recieve.detach();
}
