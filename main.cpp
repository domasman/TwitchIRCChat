//Programos testavimas
//Prisijungiame prie twitch accounto su username: softwaretesting, password: thisismysoftware12
//Atsisiunciame Twitch studio programa
//Sukompiliuojame main.cpp, nepamirsti prie linkerio nustatymu prideti libws2_32.a ir libwinmm.a biblioteku
//Rasant i Twitch chat'a viskas atsiras main.cpp programos konsoleje.
//Irasius i twitch chat'a komanda !labas, pasigirs signalas
#define _WIN32_WINNT 0x501
#include <ws2tcpip.h>
#include <iostream>
#include <fstream>
#include <Winsock2.h>
#include <string>
#include <Winsock.h>
#include <Windows.h>
#include <Mmsystem.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <regex>
//Nustatome bufferio dydi(pakaktu ir zymiai mazesnio)
constexpr int BufferLength = 512;

//Klase Twitcho socketui, turetu buti atskirame .cpp faile su savo .h failu,
//taciau del daugelio linking'o ir biblioteku klaidu viska sudejau i viena faila.

class TwitchSocket {
private:
    std::vector<char> buffer = std::vector<char>(BufferLength);
    SOCKET Connection;

public:
    int send(const std::string& information) {
        return ::send(Connection, information.c_str(), information.length(), 0);
    }

    std::string receive() {
    int bytesReceived = recv(Connection, buffer.data(), BufferLength, 0);
    return std::string(buffer.begin(), buffer.begin() + bytesReceived);
    }

    TwitchSocket(const TwitchSocket&) = delete;
    TwitchSocket& operator = (const TwitchSocket&) = delete;

    TwitchSocket(SOCKET Connection) {
        constexpr int BufferLength = 512;
        this->Connection = Connection;
    }
};

int main()
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    addrinfo hints {};
    addrinfo* result;
    //Atsidarome faila token.txt, perskaitome Twitcho OAuthToken'a prisijungimui, patalpiname token'a i string ir uzdarome faila
    std::ifstream file("token.txt");
    std::string OAuthToken;

    if (file.is_open()) {
        getline(file, OAuthToken, '\n');
        file.close();
    }

   if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cout<< "Startup failed"<<std::endl;
        return 1;
    }
    //AF_UNSPEC pranesa getaddrinfo(), kad norima grazinti socketo adresa
    hints.ai_family = AF_UNSPEC;
    //Socketas padaromas STREAM tipo
    hints.ai_socktype = SOCK_STREAM;
    //Protokolas yra TCP
    hints.ai_protocol = IPPROTO_TCP;
    //Tikriname twitcho adresa, jei nepavyko, terminuojame programa
    if(getaddrinfo("irc.chat.twitch.tv", "6667", &hints, &result)!=0){
        std::cout<<"getaddrinfo failed!"<<std::endl;
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }
    //Kuriame socketa, ir patikriname ar yra korektiskas
    ConnectSocket = socket(result ->ai_family, result -> ai_socktype, result ->ai_protocol);
    if(ConnectSocket == INVALID_SOCKET) {
        std::cout<<"Error at socket()" <<WSAGetLastError() << std::endl;
    }

    if(connect(ConnectSocket, result ->ai_addr, result->ai_addrlen)==SOCKET_ERROR) {
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
    }

    freeaddrinfo(result);

    if(ConnectSocket == INVALID_SOCKET) {
        std::cout << "Cannot connect to server" << std::endl;
        WSACleanup();
        return 1;
    }
    //Prisijungiame prie Twitch API ir siunciame prisijungimo duomenis(OAuthToken)
    //ir savo vartotojo varda(vartotojo vardas privalo buti mazosiomis raidemis)
    TwitchSocket twitch(ConnectSocket);
    twitch.send("PASS " + OAuthToken + "\r\n");
    twitch.send("NICK softwaretesting\r\n");

   (void)twitch.receive();
    //prisijungiame kaip softwaretesting vartotojas
    twitch.send("JOIN #softwaretesting\r\n");

    while(1) {

    std::string reply = twitch.receive();
    //Zinuciu apdorojimas naudojant regex, kadangi gautos zinutes i konsole yra <name>:<name> formoje (labai netvarkingos)
    std::regex re("!(.+)@.+ PRIVMSG #([^\\s]+) :(.*)");
    std::smatch match;
    std::regex_search(reply, match, re);
    std::cout<< "User: "<< match[1] <<std::endl;
    std::cout<< "Channel: "<< match[2]<<" message: "<< match[3] <<std::endl;
    //Irasius komanda !labas i Twitch Chat, bus gaunamas garsinis atsakas
    if(match[3] == "!labas") {
            //atkreipti demesi kur yra garso takelis
            PlaySound("C:\\labas.wav", NULL, SND_FILENAME);
    }
    }
}

