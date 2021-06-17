#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<pthread.h>
#include<time.h>
 
#define BUF_SIZE 100 //메시지 버퍼길이
#define NAME_SIZE 10  //클라이언트 닉네임의 길이
 
void* send_msg(void* arg);
void* recv_msg(void* arg);
void error_handling(char* msg);
 
void menu();
void changeName();
void menuOptions(); 
 
char name[NAME_SIZE]="[DEFALT]";     // 닉네임
char server_time[NAME_SIZE];        // 서버 시간
char msg[BUF_SIZE];              // 메세지
char server_port[NAME_SIZE];        // 서버 포트 넘버
char clnt_ip[NAME_SIZE];            // client IP 주소
 
int main(int argc, char *argv[])
{

    int c_sock;
    struct sockaddr_in serv_addr;
    pthread_t snd_thread, rcv_thread;
    void* thread_return;
 
    if (argc!=4)           //사용자가 프로그램을 잘못 실행한 경우
    {
        printf(" Usage : %s <ip> <port> <name>\n", argv[0]); //사용법 출력
        exit(1);
    }
 
    //  현재 시각 
    struct tm *t;
    time_t timer = time(NULL);
    t=localtime(&timer);
    sprintf(server_time, "%d-%d-%d %d:%d", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour,
    t->tm_min);
 
    sprintf(name, "%s>>", argv[3]);
    sprintf(clnt_ip, "%s", argv[1]);
    sprintf(server_port, "%s", argv[2]);
    c_sock=socket(PF_INET, SOCK_STREAM, 0);
 
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
    serv_addr.sin_port=htons(atoi(argv[2]));
 
    if (connect(c_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
        error_handling(" conncet() error");
 
    // 메뉴 호출 
    menu();
 
    pthread_create(&snd_thread, NULL, send_msg, (void*)&c_sock);
    pthread_create(&rcv_thread, NULL, recv_msg, (void*)&c_sock);
    pthread_join(snd_thread, &thread_return);
    pthread_join(rcv_thread, &thread_return);
    close(c_sock);
    return 0;
}
 
void* send_msg(void* arg)
{
    int c_sock=*((int*)arg);
    char name_msg[NAME_SIZE+BUF_SIZE];
    char myInfo[BUF_SIZE];
    char* who = NULL;
    char temp[BUF_SIZE];
 
    // 채팅 참가 시 
    printf(" >> 채팅방에 입장하셨습니다!! \n");
    sprintf(myInfo, "%s님이 입장하셨습니다. IP:%s\n",name , clnt_ip);
    write(c_sock, myInfo, strlen(myInfo));
 
    while(1)
    {
        fgets(msg, BUF_SIZE, stdin);
 
        // 메뉴 호출 명령어 -> !m
        if (!strcmp(msg, "!m\n"))
        {
            menuOptions();
            continue;
        }
 
        else if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n"))
        {
            close(c_sock);
            exit(0);
        }
 
        sprintf(name_msg, "%s %s", name,msg);
        write(c_sock, name_msg, strlen(name_msg));
    }
    return NULL;
}
 
void* recv_msg(void* arg)
{
    int c_sock=*((int*)arg);
    char name_msg[NAME_SIZE+BUF_SIZE];
    int str_len;
 
    while(1)
    {
        str_len=read(c_sock, name_msg, NAME_SIZE+BUF_SIZE-1);
        if (str_len==-1)
            return (void*)-1;
        name_msg[str_len]=0;
        fputs(name_msg, stdout);
    }
    return NULL;
}
 
 
void menuOptions() 
{
    int select;
    // print menu
    printf("\n\t----------- 메뉴 -----------\n");
    printf("\t1. 닉네임 변경\n");
    printf("\t2. 채팅창 clear/update\n\n");
    printf("\t 다른 키를 누르면 취소됩니다.");
    printf("\n\t-----------------------------\n");
    printf("\n\t----> ");
    scanf("%c", &select);
    getchar();
    switch(select)
    {
        // 유저 닉네임 변경
        case '1':
        changeName();
        break;
 
        // console update(time, clear chatting log)
        case '2':
        menu();
        break;
 
		// 메뉴창 에러
        
        default:
        printf("\t존재하지 않는 기능입니다.");
		break;
        
    }
}
 
 
// 닉네임 변경 
void changeName()
{
    char Temp[100];
    printf("\n\t 수정할 닉네임을 입력하세요. -> ");
    scanf("%s", Temp);
    sprintf(name, "%s>>", Temp);
    printf("\n\t수정이 완료되었습니다.\n\n");
}

// 채팅창 업데이트 
void menu()
{
    system("clear");
    printf(" ---------  chat client ---------\n");
    printf(" server port : %s \n", server_port);
    printf(" client IP   : %s \n", clnt_ip);
    printf(" chat name   : %s \n", name);
    printf(" server time : %s \n", server_time);
    printf(" ------------- 메뉴 -------------\n");
    printf(" 메뉴창을 띄우시려면 -> !m을 입력하세요\n");
    printf(" 1. 닉네임 변경\n");
    printf(" 2. 채팅창 clear/update\n");
    printf(" --------------------------------\n");
    printf(" 종료하려면 Q 또는 q를 눌러주세요.\n\n");
}    
 
void error_handling(char* msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
