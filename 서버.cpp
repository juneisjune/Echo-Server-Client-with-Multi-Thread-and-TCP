//멀티 스레드 서버
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS // 최신 VC++ 컴파일 시 경고 방지
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include<tchar.h>
#include<iostream>
#include<string.h>
#include<ctime>
using namespace std;

#define SERVERPORT 9000
#define BUFSIZE    512

// 소켓 함수 오류 출력 후 종료
void err_quit(const char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCWSTR)msg, MB_ICONERROR);
    LocalFree(lpMsgBuf);
    exit(1);
}

// 소켓 함수 오류 출력
void err_display(const char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    printf("[%s] %s", msg, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}
string a = "close";
// 클라이언트와 데이터 통신
DWORD WINAPI ProcessClient(LPVOID arg)
{
    SOCKET client_sock = (SOCKET)arg;
    int retval = 0;
    SOCKADDR_IN clientaddr;
    int addrlen;
    char buf[BUFSIZE + 1];

    // 클라이언트 정보 얻기
    addrlen = sizeof(clientaddr);
    getpeername(client_sock, (SOCKADDR*)&clientaddr, &addrlen);

    while (1) {

        // 데이터 받기
        retval = recv(client_sock, buf, BUFSIZE, 0);
        if (retval == SOCKET_ERROR) 
         {
            err_display("recv()");
            break;
        }
        else if (retval == 0)
        {
            break;
        }
           
        if (strcmp(buf,"close")==0)
        {
            cout << "클라이언트가 연결 종료를 희망합니다." << endl;
            cout << "동의 하시면 OK을 눌러주세요";
            cin >> buf; 
        }
        else
        {
            // 받은 데이터 출력
            buf[retval] = '\0';
            printf("받은 값은 %s\n", buf);
        }
        //보내기
        retval = send(client_sock, buf, retval, 0);
        if (retval == SOCKET_ERROR)
        {
            err_display("send()");
            break;
        }

       
    }

    // closesocket()  
    closesocket(client_sock);
    printf("\n해제된 클라이언트의 IP 주소=%s, 포트 번호=%d\n",
        inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    cout << "연결 해제 시간은: ";
    cout << tm.tm_year + 1900 << "년도 " << tm.tm_mon + 1 << "월 " << tm.tm_mday << "일 " << tm.tm_hour << "시" << tm.tm_min << "분" << tm.tm_sec << "초";
    cout << endl; 

    return 0;
}

int main(int argc, char* argv[])
{

    int retval;

    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // socket()
    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) err_quit("socket()");

    // bind()
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("bind()");

    // listen()
    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR) err_quit("listen()");

    // 데이터 통신에 사용할 변수
    SOCKET client_sock;
    SOCKADDR_IN clientaddr;
    int addrlen;
    HANDLE hThread;

    while (1) {
        // accept()
        addrlen = sizeof(clientaddr);
        client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
        if (client_sock == INVALID_SOCKET) {
            err_display("accept()");
            break;
        }

        // 접속한 클라이언트 정보 출력
        printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
            inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
        
        time_t t= time(NULL);
        struct tm tm= *localtime(&t);
        cout << "연결된 시간: ";
        cout << tm.tm_year + 1900 << "년도 " << tm.tm_mon + 1 << "월 " << tm.tm_mday << "일 " << tm.tm_hour<<"시" << tm.tm_min<<"분" << tm.tm_sec<<"초";
        cout << endl;

        //char a;
        //cout << "접속을 허용하시겠습니까?";
        //cin >> a;
    
        
            hThread = CreateThread(NULL, 0, ProcessClient,
                (LPVOID)client_sock, 0, NULL);
        
     
        // 스레드 생성
      
        if (hThread == NULL)
        {
            closesocket(client_sock);
        }
        else
        {
            CloseHandle(hThread);
        }
    }
  
    // closesocket()
    closesocket(listen_sock);

    // 윈속 종료
    WSACleanup();

    return 0;
}