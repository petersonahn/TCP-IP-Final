#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<pthread.h>
#include<time.h>
 
#define BUF_SIZE 100 //메시지 최대 글자
#define MAX_LNT 10 //최대 접속 인원 10명 제한
#define MAX_IP 30  //최대 IP
 
void * handle_clnt(void *arg);
void send_msg(char *msg, int len); //메시지 전달 함수
void error_handling(char *msg);    //예외 처리 함수
char* serverState(int count);      //서버 상태 확인 함수
void menu(char port[]);            //메뉴 출력 함수
 
 
 
int clnt_cnt=0;  //현재 접속자 = 0명
int clnt_socks[MAX_LNT];
pthread_mutex_t mutex; //서버의 어느 한 쓰레드가 서버에 있는 전역변수에 접근할때 다른 쓰레드의 접근을 막기위해 Mutex 이용
 
int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    int clnt_adr_sz;
    pthread_t t_id;
 
    // time log 
    struct tm *t;
    time_t timer = time(NULL);
    t=localtime(&timer);
    //포트를 입력하지 않은 경우
    if (argc != 2)
    {
        printf(" Usage : %s <port>\n", argv[0]);  //사용법 출력 
        exit(1);
    }
 
    menu(argv[1]);
 
    pthread_mutex_init(&mutex, NULL);
    serv_sock=socket(PF_INET, SOCK_STREAM, 0);
    
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;                     
    serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_adr.sin_port=htons(atoi(argv[1]));
    //서버 소켓에 주소 할당
    if (bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
        error_handling("bind() error");
	//서버 소켓을 서버로 사용함.
    if (listen(serv_sock, 5)==-1)
        error_handling("listen() error");
 
    while(1)
    {
        t=localtime(&timer); 
        clnt_adr_sz=sizeof(clnt_adr);
        clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
 
        pthread_mutex_lock(&mutex);
        clnt_socks[clnt_cnt++]=clnt_sock;
        pthread_mutex_unlock(&mutex);
 
        pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
        pthread_detach(t_id);
        printf(" 참가자의 IP : %s ", inet_ntoa(clnt_adr.sin_addr));
        printf("(%d-%d-%d %d:%d)\n", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min);
        printf(" 채팅 참가 인원 (%d/10)\n", clnt_cnt);
    }
    close(serv_sock);
    return 0;
}
 
void *handle_clnt(void *arg)
{
    int clnt_sock=*((int*)arg);
    int str_len=0, i;
    char msg[BUF_SIZE];
 
    while((str_len=read(clnt_sock, msg, sizeof(msg)))!=0)
        send_msg(msg, str_len);
 
    
	pthread_mutex_lock(&mutex);
	for (i = 0; i<clnt_cnt; i++)
	{
		if (clnt_sock == clnt_socks[i])
		{
			while (i<clnt_cnt - 1) {
				clnt_socks[i] = clnt_socks[i + 1];
				i++;
			}
			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutex);
	close(clnt_sock);
	return NULL;
}
 
void send_msg(char* msg, int len)
{
    int i;
    pthread_mutex_lock(&mutex);
    for (i=0; i<clnt_cnt; i++)
        write(clnt_socks[i], msg, len);
    pthread_mutex_unlock(&mutex);
}
 
void error_handling(char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
 
char* serverState(int count)  //서버 상태 표시
{
    char* stateMsg = malloc(sizeof(char) * 20);
    strcpy(stateMsg ,"....");
    
    if (count < 5)
        strcpy(stateMsg, "좋음");
    else
        strcpy(stateMsg, "나쁨");
    
    return stateMsg;
}        
 
void menu(char port[])
{
	system("clear");
	printf(" --------   chat server   --------\n");
	printf(" 서버 포트    : %s\n", port);
	printf(" 서버 상태   : %s\n", serverState(clnt_cnt));
	printf(" 최대 접속 인원 : %d\n", MAX_LNT);
	printf(" -----   Connection Log    ------\n\n");
}

