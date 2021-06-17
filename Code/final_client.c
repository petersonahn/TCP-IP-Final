#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<pthread.h>
#include<time.h>
 
#define BUF_SIZE 100 //�޽��� ���۱���
#define NAME_SIZE 10  //Ŭ���̾�Ʈ �г����� ����
 
void* send_msg(void* arg);
void* recv_msg(void* arg);
void error_handling(char* msg);
 
void menu();
void changeName();
void menuOptions(); 
 
char name[NAME_SIZE]="[DEFALT]";     // �г���
char server_time[NAME_SIZE];        // ���� �ð�
char msg[BUF_SIZE];              // �޼���
char server_port[NAME_SIZE];        // ���� ��Ʈ �ѹ�
char clnt_ip[NAME_SIZE];            // client IP �ּ�
 
int main(int argc, char *argv[])
{

    int c_sock;
    struct sockaddr_in serv_addr;
    pthread_t snd_thread, rcv_thread;
    void* thread_return;
 
    if (argc!=4)           //����ڰ� ���α׷��� �߸� ������ ���
    {
        printf(" Usage : %s <ip> <port> <name>\n", argv[0]); //���� ���
        exit(1);
    }
 
    //  ���� �ð� 
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
 
    // �޴� ȣ�� 
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
 
    // ä�� ���� �� 
    printf(" >> ä�ù濡 �����ϼ̽��ϴ�!! \n");
    sprintf(myInfo, "%s���� �����ϼ̽��ϴ�. IP:%s\n",name , clnt_ip);
    write(c_sock, myInfo, strlen(myInfo));
 
    while(1)
    {
        fgets(msg, BUF_SIZE, stdin);
 
        // �޴� ȣ�� ��ɾ� -> !m
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
    printf("\n\t----------- �޴� -----------\n");
    printf("\t1. �г��� ����\n");
    printf("\t2. ä��â clear/update\n\n");
    printf("\t �ٸ� Ű�� ������ ��ҵ˴ϴ�.");
    printf("\n\t-----------------------------\n");
    printf("\n\t----> ");
    scanf("%c", &select);
    getchar();
    switch(select)
    {
        // ���� �г��� ����
        case '1':
        changeName();
        break;
 
        // console update(time, clear chatting log)
        case '2':
        menu();
        break;
 
		// �޴�â ����
        
        default:
        printf("\t�������� �ʴ� ����Դϴ�.");
		break;
        
    }
}
 
 
// �г��� ���� 
void changeName()
{
    char Temp[100];
    printf("\n\t ������ �г����� �Է��ϼ���. -> ");
    scanf("%s", Temp);
    sprintf(name, "%s>>", Temp);
    printf("\n\t������ �Ϸ�Ǿ����ϴ�.\n\n");
}

// ä��â ������Ʈ 
void menu()
{
    system("clear");
    printf(" ---------  chat client ---------\n");
    printf(" server port : %s \n", server_port);
    printf(" client IP   : %s \n", clnt_ip);
    printf(" chat name   : %s \n", name);
    printf(" server time : %s \n", server_time);
    printf(" ------------- �޴� -------------\n");
    printf(" �޴�â�� ���÷��� -> !m�� �Է��ϼ���\n");
    printf(" 1. �г��� ����\n");
    printf(" 2. ä��â clear/update\n");
    printf(" --------------------------------\n");
    printf(" �����Ϸ��� Q �Ǵ� q�� �����ּ���.\n\n");
}    
 
void error_handling(char* msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
