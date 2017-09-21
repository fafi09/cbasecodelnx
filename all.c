#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <pthread.h>
#include <semaphore.h>

extern char **environ;

void readEnv(char** arge)
{
	while(*arge != NULL)
	{
		printf("%s\n",*arge);
		*arge++;
	}
}

void sysMem()
{
	void* p = sbrk(0);
	int* p1 = p;
	//p1[0] = 11; //�δ���
	brk(p1+4);
	p1[0] = 10;
	p1[1] = 20;
	p1[2] = 30;
	p1[3] = 40;
	p1[4] = 50;
	int* p3 = sbrk(0);
	int* p2 = sbrk(4);
	printf("p2addr = %p\n",p2);
	printf("p3addr = %p\n",p3);
	printf("p2=%d\n",*p2);
	p2[0] = 60;
	printf("p2=%d\n",*p2);
	
	p1[1023] = 70;
	printf("p1[1023]=%d\n",p1[1023]);
	//p1[1024] = 80; //�δ��� ֻӳ��4k�ֽ�,4*1024
	//printf("p1[1024]=%d\n",p1[1024]);
	brk(p1); //�ͷ��ڴ�
}

void mmapMem()
{
	printf("mmapMem\n");
	printf("getpagesize=%d\n",getpagesize());
	//ӳ���ڴ�
	int a = 9;
	int* digit = mmap(NULL, getpagesize(), PROT_READ | PROT_WRITE, MAP_ANONYMOUS,
                  0, 0);
                  
	digit = &a;
	printf("digit=%d\n",digit[0]);
  munmap(digit, getpagesize());
}
int openFile(const char* filename)
{
	int fd;
	fd = open(filename, O_RDWR | O_CREAT | O_EXCL, 0666);
	if(fd == -1)
	{
		fd = open(filename, O_RDWR | O_APPEND);
		if(fd==-1) printf("::%m\n"),exit(-1);
	}
	return fd;
}

void mmapFile()
{
	printf("mmapFile\n");
	printf("getpagesize=%d\n",getpagesize());
	int fd;
	int value[4];
	fd = openFile("allmmapfile.dat");
	//ӳ���ļ�
	int a = 9;
	int b[] = {4,5,6,7};
	int* digit = mmap(NULL,16, PROT_READ | PROT_WRITE, MAP_SHARED,
                  fd, 0);
	//digit= &a;
	//memcpy(digit,&a,4);
	memcpy(digit,b,16);
	ftruncate(fd,16);
	printf("digit=%d\n",digit[1]);
  munmap(digit, 16);
  close(fd);
  
  fd = openFile("allmmapfile.dat");
  read(fd, value, 16);
  printf("value[0]=%d\n",value[0]);
  printf("value[1]=%d\n",value[1]);
  printf("value[2]=%d\n",value[2]);
  printf("value[3]=%d\n",value[3]);
  
  lseek(fd, 8, SEEK_SET);
  read(fd, value, 8);
  printf("value[0]=%d\n",value[0]);
  close(fd);
}

void wrIO()
{
	//write(0,"Hello\n",6);
	//write(1,"world\n",6);
	//write(2,"louis\n",6);
	int r;
	char buf[32];
	bzero(buf, 32);
	
	//r = read(0, buf, 30);
	printf("��ȡfd1\n");
	r = read(1, buf, 30);
	if(r > 0)
	{
		buf[r] = 0;
		printf("buf=%s\n",buf);
	}
}

void Getfstat()
{
	int fd;
	struct stat buf;
	fd = openFile("allmmapfile.dat");
	fstat(fd, &buf);
	printf("size=%d\n",buf.st_size);
	close(fd);
}

void testdup()
{
	int fd;
	int newfd;
	int oldfd;
	printf("pid=%d\n",getpid());
	fd = openFile("testdup.dat");
	oldfd = dup(STDOUT_FILENO);
	if(oldfd==-1) printf("::%m\n"),exit(-1);
	newfd = dup2(fd,STDOUT_FILENO);
	if(newfd==-1) printf("::%m\n"),exit(-1);
	printf("adfd\n");
	dup2(oldfd,newfd);
	printf("adfd1\n");
	close(fd);
	//while(1);
}

void testfcntl_dup()
{
	//1.����һ�����е�������
	//cmd=F_DUPFD
	int fd;
	int newfd;
	int oldfd;
	char buf[128];
	printf("pid=%d\n",getpid());
	fd = openFile("testdup.dat");
	newfd = fcntl(fd, F_DUPFD);
	strncpy(buf, "helloworld\n", 12);
	write(newfd, buf, 12);
	close(fd);
}

void testfcntl_fd()
{
	//2.����close-on-exec��־��
	//�ڴ˺����д����ӽ��̣�����execl
	int pid;
	int fd;
	int ret;
	fd = openFile("testfcntl_fd.dat");
	ret = fcntl(fd, F_GETFD, NULL);
	printf("ret=%d\n",ret);
	printf("FD_CLOEXEC=%d\n",FD_CLOEXEC);
	printf("FD_CLOEXEC & ret=%d\n",(FD_CLOEXEC & ret));
	fcntl(fd, F_SETFD, 1);
	ret = fcntl(fd, F_GETFD, NULL);
	printf("ret=%d\n",ret);
	printf("FD_CLOEXEC=%d\n",FD_CLOEXEC);
	printf("FD_CLOEXEC & ret=%d\n",(FD_CLOEXEC & ret));
	
	char *s = "oooooooooooo";
	printf("*s=%s\n",s);
	printf("*strlen(s)=%d\n",strlen(s));
	printf("man pid = %d\n",getpid());
	printf("execl �������µĽ��̣��滻��ǰ���̴���\n");
	pid = fork();
	if(pid == 0)
	{
		ret = execl("fcntl_getfd_test","./fcntl_getfd_test", &fd, NULL);
	}
	wait(NULL);
	write(fd, s, strlen(s));
	printf("end:%d\n",ret);
	close(fd);
}

void testfcntl_fl()
{
	int fd;
	int ret;
	unsigned int flg;
	fd = openFile("testfcntl_fd.dat");
	ret = fcntl(fd, F_GETFL, NULL);
	printf("*ret=%d\n",ret);
	printf("*O_ACCMODE=%d\n",O_ACCMODE);
	printf("*O_RDONLY=%d\n",O_RDONLY);
	printf("*O_WRONLY=%d\n",O_WRONLY);
	printf("*O_RDWR=%d\n",O_RDWR);
	printf("*O_APPEND=%d\n",O_APPEND);
	printf("*O_ASYNC=%d\n",O_ASYNC);
	//printf("*O_DIRECT=%d\n",O_DIRECT);
	//printf("*O_NOATIME=%d\n",O_NOATIME);
	printf("*O_NONBLOCK=%d\n",O_NONBLOCK);
	switch(ret & O_ACCMODE) {
		case O_RDONLY:
			printf("read only\n");
			break;
		case O_WRONLY:
			printf("write only\n");
			break;
		case O_RDWR:
			printf("read write\n");
			break;
		default:
			printf("unknown access mode\n");
			break;
	}
	if(ret & O_APPEND)
	{
		printf("append\n");
	}
	if(ret & O_NONBLOCK)
	{
		printf("nonblock\n");
	}
	//���Ը��ĵļ�����־O_APPEND, O_ASYNC, O_DIRECT, O_NOATIME, and O_NONBLOCK 
	fcntl(fd, F_SETFL, O_NONBLOCK);
	ret = fcntl(fd, F_GETFL, NULL);
	printf("*ret=%d\n",ret);
	if(ret & O_APPEND)
	{
		printf("append\n");
	}
	if(ret & O_NONBLOCK)
	{
		printf("nonblock\n");
	}
	fcntl(fd, F_SETFL, O_APPEND);
	ret = fcntl(fd, F_GETFL, NULL);
	printf("*ret=%d\n",ret);
	if(ret & O_APPEND)
	{
		printf("append\n");
	}
	if(ret & O_NONBLOCK)
	{
		printf("nonblock\n");
	}
	//���ӱ�־Ϊ
	printf("======add mode=======\n");
	flg |= O_NONBLOCK;
	fcntl(fd, F_SETFL, flg);
	ret = fcntl(fd, F_GETFL, NULL);
	printf("*ret=%d\n",ret);
	if(ret & O_APPEND)
	{
		printf("append\n");
	}
	if(ret & O_NONBLOCK)
	{
		printf("nonblock\n");
	}
	//�����־λ
	printf("======clear mode=======\n");
	ret = fcntl(fd, F_GETFL, NULL);
	flg &= ~O_NONBLOCK;
	fcntl(fd, F_SETFL, flg);
	ret = fcntl(fd, F_GETFL, NULL);
	printf("*ret=%d\n",ret);
	if(ret & O_APPEND)
	{
		printf("append\n");
	}
	if(ret & O_NONBLOCK)
	{
		printf("nonblock\n");
	}
	close(fd);
}

void diroperate()
{
	DIR *d;
	struct dirent *de;
	off_t offset = 0;
	int fd;
	struct stat buf;
	d = opendir("/home");
	if(d == NULL)
	{
		printf("%m\n");
		exit(-1);
	}
	while(de = readdir(d))
	{
		printf("name=%s\n",de->d_name);
		if(strcmp(de->d_name,"oracle") == 0)
		{
			offset = telldir(d);
			printf("binggo\n");
		}
	}
	
	closedir(d);
	printf("offset=%d\n",offset);
	d = opendir("/home");
	seekdir(d, offset);//��λ��oracleĿ¼������֮���Ŀ¼ȡ��
	while(de = readdir(d))
	{
		printf("name=%s\n",de->d_name);
	}
	
	//dirfdȡ��Ŀ¼���ļ�������
	fd = dirfd(d);
	fstat(fd, &buf);
	printf("size=%d\n",buf.st_size);
	closedir(d);
	
}
int filter(const struct dirent *d)
{
	if(strcmp(d->d_name,".") == 0 || strcmp(d->d_name,"..") == 0)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}
int compar(const void *a, const void *b)
{
	return alphasort(a,b);
}
void scandirtest()
{
	int ret;
	struct dirent **namelist;
	ret = scandir("/home", &namelist,
             	filter,// NULL,
              compar //NULL
              );
  while(*namelist != NULL)
  {
  	printf("name=%s\n",(*namelist)->d_name);
  	//free(*namelist);
  	namelist++;
  }
  //free(namelist);
}

void diroprate2()
{
	char buf[128];
	getcwd(buf, 128);
	printf("pwd=%s\n",buf);
	mkdir("mt",0666);
	bzero(buf, 128);
	getcwd(buf, 128);
	printf("pwd=%s\n",buf);
	chdir("./mt");
	bzero(buf, 128);
	getcwd(buf, 128);
	printf("pwd=%s\n",buf);
}

void processSystem()
{
	int r;
	int code;
	r = system("ls -l");
	code = WEXITSTATUS(r);
	printf("code=%d\n",code);
	r = system("s2d");
	code = WEXITSTATUS(r);
	printf("code=%d\n",code);
}

void h(int s)
{
	printf("deal int signal\n");
}

void h1(int s)
{
	printf("deal %d signal\n",s);
}

void signalMask()
{
	sigset_t sigs,sigp,sigq;
	signal(10,h1);
	signal(SIGINT,h1);
	sigemptyset(&sigs);
	sigemptyset(&sigp);
	sigemptyset(&sigq);
	sigaddset(&sigs,10);
	sigprocmask(SIG_BLOCK,&sigs,0);
	sleep(100);
	printf("first sleep end\n");
	sigprocmask(SIG_UNBLOCK,&sigs,0);
	sleep(100);
	printf("second sleep end\n");
}

void htimer(int s)
{
	printf("wake up:%d \n",s);
}

void settimer()
{
	signal(SIGALRM,htimer);
	struct itimerval value = {0};
	value.it_value.tv_sec = 5;
	value.it_interval.tv_sec = 1;
	setitimer(ITIMER_REAL, &value,
                     NULL);
  while(1);
}

void hmasksuspend(int s)
{
	printf("idle time deal int signal\n");
}
void masksuspend()
{
	int sum=0;
	int i;
	//1.
	signal(SIGINT,hmasksuspend);
	sigset_t sigs,sigp,sigq;
	//2.
	sigemptyset(&sigs);
	sigemptyset(&sigp);
	sigemptyset(&sigq);
	
	sigaddset(&sigs,SIGINT);
	//3.
	sigprocmask(SIG_BLOCK,&sigs,0);
	for(i=1;i<=10;i++)
	{
		sum+=i;
		sigpending(&sigp);
		if(sigismember(&sigp,SIGINT))
		{
			printf("SIGINT in queue!\n");
			sigsuspend(&sigq);
			//ʹԭ�������ź���Ч������ԭ���ź�
			//ʹ�µ��ź�����,
			//��ĳ���źŴ������������
			//sigsuspend�ָ�ԭ�������źţ����� 
		}
		sleep(1);
	}
	printf("sum=%d\n",sum);
	sigprocmask(SIG_UNBLOCK,&sigs,0);
	printf("Over!\n");
}

void h_sa_sigaction(int signo, siginfo_t *info, void *d)
{
	printf("signal:%d\n",signo);
	printf("signal value:%d\n",info->si_value);
}

void t_sigaction() 
{
	struct sigaction act = {0};
	act.sa_sigaction = h_sa_sigaction;
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask,SIGINT);
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGUSR1, &act, NULL);
	
	while(1);
}

//#include <stdio.h>
//#include <signal.h>
//#include <unistd.h>
//int main(int argc , char** argv)
//{
//	union sigval val;
//	val.sival_int=8888;
//	printf("argc=%d\n",argc);
//	printf("argv=%s\n",argv[0]);
	
//	if(argc == 2)
//	{
//		printf("argvatoi=%d\n",atoi(argv[1]));
//		sigqueue(atoi(argv[1]),SIGUSR1,val);
////		sigqueue(atoi(argv[1]),SIGINT,val);
//	}
//}

int fd;
int i;
void h_end(int s)
{
	printf("signal:%d\n",s);
	//�رչܵ�
	close(fd);
	//ɾ���ܵ�
	unlink("my.pipe");
	exit(-1);
}
//�����ܵ�
void t_pipe()
{
	signal(SIGINT,h_end);
	//�����ܵ�
	mkfifo("my.pipe",0666);
	//�򿪹ܵ�
	fd = open("my.pipe", O_RDWR);
	//shutdown(fd,SHUT_RD); ��ֹԶ�̶�
	i = 0;
	while(1)
	{
		//ÿ��һ��д����
		sleep(1);
		write(fd, &i, 4);
		i++;
	}
}

//int fd;
void t_read_end(int s)
{
	//�رչܵ�
	close(fd);
	exit(-1);
}
void t_read_pipe()
{
	int i;	
	//�򿪹ܵ�
	signal(SIGINT,t_read_end);
	fd=open("my.pipe",O_RDWR);
	//shutdown(fd,SHUT_WR);
	while(1)
	{
		read(fd,&i,4);
		printf("%d\n",i);
	}	
}

//�����ܵ�ֻ�ڸ��ӽ�����ʹ��
//int pipe(int fd[2]);//�����ܵ�.�򿪹ܵ�.�����ܵ�.�رն�д		
//		fd[0]:ֻ��(����д)
//		fd[1]:ֻд(���ܶ�)

void anonymous_pipe()
{
	int fd2[2];
	int r;
	char buf[64];
	r = pipe(fd2);
	write(fd2[1],"hello", 5);
	write(fd2[1],"world", 5);
	r = read(fd2[0], buf, 5);
	buf[r] = 0;
	printf("%s\n",buf);
	r = read(fd2[0], buf, 5);
	buf[r] = 0;
	printf("%s\n",buf);
	
}
int* p_shm;
int shmid;
void h_shm(int s)
{
	//ж�ع����ڴ�
	int r = shmdt(p_shm);
	if(r == -1) printf("shmdt error:%m"),exit(-1);
	r = shmctl(shmid, IPC_RMID, NULL);
	if(r == -1) printf("shmctl error:%m"),exit(-1);
	exit(0);
}
//�����ڴ�
void t_shm()
{
	signal(SIGINT, h_shm);
	key_t key = ftok(".", 255);
	if(key==-1) printf("ftok error:%m\n"),exit(-1);
	//ȡ�ù����ڴ�
	shmid = shmget(key, 4, IPC_CREAT | IPC_EXCL | 0666);
	if(shmid == -1) printf("shmget error:%m"),exit(-1);
	//���ع����ڴ�
	p_shm = shmat(shmid, NULL, 0);
	if(p_shm == (void *) -1) printf("shmat error:%m"),exit(-1);
	int i = 0;
	while(1)
	{
		*p_shm = i;
		sleep(1);
		i++;
	}
}

void deal(int s)
{
	if(s==2)
	{
		//4.ж�ع����ڴ�shmdt
		shmdt(p_shm);
		exit(0);
	}
}
void read_shm()
{
	signal(SIGINT,deal);	
	//1.���������ڴ�shmget
	key_t key=ftok(".",255);
	if(key==-1) printf("ftok error:%m\n"),exit(-1);
	
	shmid=shmget(key,4,0);
	if(shmid==-1) printf("get error:%m\n"),exit(-1);
	//2.���ع����ڴ�shmat
	p_shm=shmat(shmid,0,0);
	if(p_shm==(int*)-1) printf("at error:%m\n"),exit(-1);
	//3.���ʹ����ڴ�
	while(1)
	{		
		sleep(1);
		printf("%d\n",*p_shm);
	}
	
}

int msgid;
struct msgbuf {
                 long mtype;     /* message type, must be > 0 */
                 char mtext[200];  /* message data */ //����Ҫ�������ͬ�ĳ���
            };
void h_send_msg(int s)
{
	printf("del msg:%d",s);
	msgctl(msgid, IPC_RMID, 0);
}
//��Ϣ����
void t_send_msg()
{
	int i;
	struct msgbuf msg;
	signal(SIGINT,h_send_msg);	
	key_t key=ftok(".",255);
	//�õ���Ϣid
	msgid = msgget(key, IPC_CREAT|IPC_EXCL|0666);
	if(msgid==-1) printf("msgget error:%m\n"),exit(-1);
	//������Ϣ��������Ϣ
	for(i = 0; i < 10; i++)
	{
		bzero(msg.mtext,sizeof(msg.mtext));
		msg.mtype = 1;
		sprintf(msg.mtext,"send 1 msg:%d",i);
		msgsnd(msgid, &msg, sizeof(msg.mtext), 0);
	}
	//msgctl(msgid, IPC_RMID, 0);
}

void t_receive_msg()
{
	key_t key;
	int msgidd;
	int i;
	struct msgbuf msg;
	//1�õ���Ϣ����
	key=ftok(".",200);
	if(key==-1) printf("ftok err:%m\n"),exit(-1);
	
	msgidd=msgget(key,0);
	if(msgidd==-1)printf("get err:%m\n"),exit(-1);
	//2������Ϣ
		
	//3������Ϣ
	while(1)
	{
		bzero(&msg,sizeof(msg));
		msg.mtype=1;
		msgrcv(msgidd,&msg,sizeof(msg.mtext),1,0);
		printf("%s\n",msg.mtext);
	}
	
}

union semun {
               int              val;    /* Value for SETVAL */
               struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
               unsigned short  *array;  /* Array for GETALL, SETALL */
               struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                           (Linux specific) */
           };
//�ź���
void t_sem_a()
{
	union semun v;
	struct sembuf op[1];
	key_t key=ftok(".",99);
	if(key==-1) printf("ftok err:%m\n"),exit(-1);
	int semid = semget(key, 1, IPC_CREAT|IPC_EXCL|0666);
	if(semid==-1) printf("semget err:%m\n"),exit(-1);
	v.val = 2;
	int r = semctl(semid, 0, SETVAL, v);
	if(r==-1) printf("semctl err:%m\n"),exit(-1);
	op[0].sem_num = 0;
	op[0].sem_op = -1;
	op[0].sem_flg = 0;
	while(1)
	{
		r = semop(semid, op, 1);
		printf("free blocked\n");
	}
	//4.ɾ��(���Բ�ɾ��)
	//semctl(semid,0,IPC_RMID);
}

void t_sem_b()
{
	key_t key;
	int semid;	//�ź���ID
	union  semun v;//2.2.�����ʼ��ֵ
	int r;
	struct sembuf op[2];
	//1.�����ź���
	key=ftok(".",99);
	if(key==-1) printf("ftok err:%m\n"),exit(-1);
	
			
	semid=semget(key,1,0);//�õ��ź���
	if(semid==-1) printf("get err:%m\n"),exit(-1);
	
	printf("id:%d\n",semid);		
	//3.���ź���������������
	//3.1.�������
	op[0].sem_num=0;//�ź����±�
	op[0].sem_op=1;//�ź���������λ������
	op[0].sem_flg=0;
	op[1].sem_num=0;//�ź����±�
	op[1].sem_op=1;//�ź���������λ������
	op[1].sem_flg=0;
	while(1)
	{
		r=semop(semid,op,2);
		sleep(1);
	}
	
	//4.ɾ��(���Բ�ɾ��)
	//semctl(semid,0,IPC_RMID);
}

//�߳�
void * start_routine(void* data)
{
	printf("�����߳�:%s\n",data);
	pthread_exit("world");
}
void creatTh()
{
	pthread_t tid;
	char* value_ptr;
	pthread_create(&tid,
              0,
              start_routine, "jack");
  pthread_join(tid, (void **)&value_ptr);
  printf("return:%s\n",value_ptr);
}

//�̻߳�����
pthread_mutex_t mutex;
void routine(void* d)
{
	printf("������\n");
	pthread_mutex_unlock(&mutex);
}
void * runodd(void* data)
{
	int i;
	for(i = 1;i< 10; i+=2)
	{
		pthread_cleanup_push(routine, 0);
		/*1.����pthead_exitʱ,
			2.��Ӧȡ������ʱ,
			3.�÷�0��execute��������pthread_cleanup_pop*/
		pthread_mutex_lock(&mutex);
		printf("%d\n",i);
		sleep(1);
		//pthread_mutex_unlock(&mutex);
		pthread_cleanup_pop(1);
	}
}
void * runeven(void* data)
{
	int i;
	for(i = 0;i<10; i+=2)
	{
		pthread_cleanup_push(routine, 0);
		pthread_mutex_lock(&mutex);
		printf("%d\n",i);
		sleep(1);
		//pthread_mutex_unlock(&mutex);
		pthread_cleanup_pop(1);
	}
}

void t_mutex_th()
{
	pthread_t todd,teven;
	pthread_mutex_init(&mutex,0);
	pthread_create(&todd,
              0,
              runodd, NULL);
  pthread_create(&teven,
              0,
              runeven, NULL);
 	sleep(2);
 	pthread_cancel(todd);
  pthread_join(todd, 0);
  pthread_join(teven, 0);
  pthread_mutex_destroy(&mutex);
}

//�߳��ź�
pthread_t t_th_signal1,t_th_signal2;
sigset_t t_signal_sigs;
void t_th_handle(int s)
{
	printf("signal:%d\n",s);
}

void* t_th_signal1_run(void* d)
{
	int s;
	while(1)
	{
		
		printf("before th1\n");
		//pause();
		sigwait(&t_signal_sigs, &s);
		printf("after th1:%d\n",s);
	}
}
void* t_th_signal2_run(void* d)
{
	while(1)
	{
		printf("before th2 pthread_kill\n");
		sleep(1);
		pthread_kill(t_th_signal1, SIGUSR1);
		/*
		���û��sigwait�ɽ��̵��źŴ���������
		������̵��źŴ�������sigwaitͬʱ������sigwait����
		������͵��ź���sigwait��ƥ������̵��źŴ���������
		*/
		printf("after th2 pthread_kill\n");
	}
}
void t_th_signal()
{
	sigemptyset(&t_signal_sigs);
	sigaddset(&t_signal_sigs, SIGUSR1);
	//sigaddset(&t_signal_sigs, SIGUSR2);
	
	signal(SIGUSR1,t_th_handle);
	pthread_create(&t_th_signal1,NULL,
              t_th_signal1_run, 0);
  pthread_create(&t_th_signal2,NULL,
              t_th_signal2_run, 0);
  pthread_join(t_th_signal1, NULL);
  pthread_join(t_th_signal2, NULL);
}

//������ waitʱ���ͷŻ�����
//����Ϊth1��th2�����������
pthread_mutex_t t_cond_mutex;
pthread_t t_th_cond1,t_th_cond2;
pthread_cond_t t_cond_cond;
int cond = 0;
int sum = 0;
void* t_th_cond1_run(void* d)
{
	int s;
	int i =0;
	//for(i = 0; i < 10; i++)
	//{
		pthread_mutex_lock(&t_cond_mutex);
		while(!cond)
		{
			
			//printf("th1 wait before\n");
			pthread_cond_wait(&t_cond_cond,
	              &t_cond_mutex);
			//printf("th1 wait after\n");
		}
		printf("th1 is working\n");
		cond = 0;
		//printf("th1 signal before\n");
		pthread_cond_broadcast(&t_cond_cond);//��broadcast������signal��ֹֻ���Ѵ��߳�
		//printf("th1 signal after\n");
		pthread_mutex_unlock(&t_cond_mutex);
	//}
}
void* t_th_cond2_run(void* d)
{
	int s;
	int i =0;
	//for(i = 0; i < 10; i++)
	//{
		pthread_mutex_lock(&t_cond_mutex);
		while(cond)
		{
			
			//printf("th2 wait before\n");
			pthread_cond_wait(&t_cond_cond,
	              &t_cond_mutex);
			//pthread_cond_signal(&t_cond_cond);
			////pthread_cond_signal(&t_cond_cond);
			////pthread_cond_signal(&t_cond_cond);
			//sleep(1);
			//printf("th2 wait after\n");
		}
		printf("th2 is working\n");
		cond = 1;
		//printf("th2 signal before\n");
		pthread_cond_broadcast(&t_cond_cond);
		//printf("th2 signal after\n");
		pthread_mutex_unlock(&t_cond_mutex);
	//}
}
void t_th_cond()
{
	pthread_mutex_init(&t_cond_mutex,0);
	pthread_cond_init(&t_cond_cond,NULL);
	pthread_create(&t_th_cond1,
              0,
              t_th_cond1_run, NULL);
  pthread_create(&t_th_cond2,
              0,
              t_th_cond2_run, NULL);
  pthread_join(t_th_cond1, 0);
  pthread_join(t_th_cond2, 0);
  pthread_cond_destroy(&t_cond_cond);
  pthread_mutex_destroy(&t_cond_mutex);
}


//�߳��ź���
pthread_t t_th_sem1;
sem_t t_th_sem_sem1;
void* t_th_sem1_run(void* d)
{
	while(1)
	{
		sem_wait(&t_th_sem_sem1);//ÿ��-1��0����
		printf("wait free\n");
	}
}

void t_th_sem()
{
	sem_init(&t_th_sem_sem1, 
					0, //�ڶ�������0:
						 //�ź����ڴ˽��̵��̼߳乲��
						 //�����ڽ��̼乲��,
					5); //��ʼ���ź���Ϊ5
	pthread_create(&t_th_sem1,
              0,
              t_th_sem1_run, NULL);
  while(1)
  {
  	sleep(1);
  	sem_post(&t_th_sem_sem1);//ÿ��+1
  	printf("sem_post\n");
  }
  pthread_join(t_th_sem1, 0);
  sem_destroy(&t_th_sem_sem1);
}

//��д��
int global_sum = 10;
pthread_rwlock_t rwlock;
pthread_t t_th_r1,t_th_r2,t_th_w1,t_th_w2;
void* t_th_r(void* d)
{
	while(1)
	{
		pthread_rwlock_rdlock(&rwlock);
		printf("%s entry rdlock area, global=%d\n", (char*)d, global_sum);
		sleep(1);
		printf("%s leave rdlock area\n", (char*)d);
		pthread_rwlock_unlock(&rwlock);
		sleep(1);
	}
}
void* t_th_w(void* d)
{
	while(1)
	{
		pthread_rwlock_wrlock(&rwlock);
		global_sum++;
		printf("%s entry wrlock area, global=%d\n", (char*)d, global_sum);
		sleep(1);
		printf("%s leave wrlock area\n", (char*)d);
		pthread_rwlock_unlock(&rwlock);
		sleep(2);
	}
}
void t_th_rw()
{
	pthread_rwlock_init(&rwlock,NULL);
	pthread_create(&t_th_r1,
              0,
              t_th_r, "r1");
  pthread_create(&t_th_r2,
              0,
              t_th_r, "r2");
  pthread_create(&t_th_w1,
              0,
              t_th_w, "w1");
  pthread_create(&t_th_w2,
              0,
              t_th_w, "w2");  
  pthread_join(t_th_r1, 0);
  pthread_join(t_th_r2, 0);
  pthread_join(t_th_w1, 0);
  pthread_join(t_th_w2, 0);
	pthread_rwlock_destroy(&rwlock);
}


//�߳�˽�б���
pthread_t t_th_local1,t_th_local2;
pthread_key_t key;
/*
�߳�ִ������ڴ�����
*/
void destructor(void* d)
{
	printf("des:%d\n",*(int*)d);
}
void* t_th_local1_run(void* d)
{
	int i = 0; 
	char value[] = "hello";
	for(i =0; i < 10 ; i++)
	{
		pthread_setspecific(key, value);
		printf("\tThread -1:%s\n",pthread_getspecific(key));
		sleep(1);
	}
}
void* t_th_local2_run(void* d)
{
	int i = 0; 
	char value[] = "world";
	for(i =0; i < 10 ; i++)
	{
		pthread_setspecific(key, value);
		printf("\tThread -2:%s\n",pthread_getspecific(key));
		sleep(1);
	}
}
void t_th_local()
{
	pthread_key_create(&key, destructor);
	pthread_create(&t_th_local1,
              0,
              t_th_local1_run, "t_th_local1");
  pthread_create(&t_th_local2,
              0,
              t_th_local2_run, "t_th_local2");
  pthread_join(t_th_local1, 0);
  pthread_join(t_th_local2, 0);
  pthread_key_delete(key);
  
   
}

//������
/*�������뻥����������ʵ����Ч����
mutex����sleep-waiting���͵�����
������һ��˫�˵Ļ������������̣߳�A��B��
���Ƿֱ�������core0��core1�������߳�A��Ҫͨ��
pthread_mutex_lock���һ���ٽ����������������
��B���У���ô�߳�A�ᱻ������core0��ʱ����������
�л������߳�A���ڵȴ������У���ʱcore0�Ϳ�������
��������������һ���߳�C�������ؽ���æ�ȴ���
��spin lock��Ȼ��������busy-waiting���͵�����
����߳�A��ʹ��spinlockȥ����������ô�߳�A�ͻ�һֱ
��core0�Ͻ���æ�ȴ�����ͣ�Ľ���������ֱ����������
pthread_mutex_init(&t_cond_mutex,0);
pthread_mutex_destroy(&t_cond_mutex);
pthread_mutex_lock(&t_cond_mutex);
pthread_mutex_unlock(&t_cond_mutex);
*/
pthread_spinlock_t spin_lock;
pthread_t t_th_spin1,t_th_spin2;
pthread_mutex_t t_th_spin_mutex;
void* t_th_spin1_run(void* d)
{
	
	pthread_spin_lock(&spin_lock);
	//pthread_mutex_lock(&t_th_spin_mutex);
	printf("%s entry spin lock\n", (char*)d);
	sleep(5);
	pthread_spin_unlock(&spin_lock);
	//pthread_mutex_unlock(&t_th_spin_mutex);
	printf("%s leave spin lock\n", (char*)d);
	
}
void* t_th_spin2_run(void* d)
{
	printf("%s entry spin lock before\n", (char*)d);
	pthread_spin_lock(&spin_lock);
	//pthread_mutex_lock(&t_th_spin_mutex);
	printf("%s entry spin lock\n", (char*)d);
	sleep(1);
	pthread_spin_unlock(&spin_lock);
	//pthread_mutex_unlock(&t_th_spin_mutex);
	printf("%s leave spin lock\n", (char*)d);
}

void t_th_spin()
{
	pthread_spin_init(&spin_lock, 0);
	//pthread_mutex_init(&t_th_spin_mutex,0);
	pthread_create(&t_th_spin1,
              0,
              t_th_spin1_run, "t_th_spin1");
  pthread_create(&t_th_spin2,
              0,
              t_th_spin2_run, "t_th_spin2");
  sleep(3);
  printf("main thread is running\n");
  pthread_join(t_th_spin1, 0);
  pthread_join(t_th_spin2, 0);
	pthread_spin_destroy(&spin_lock);
	//pthread_mutex_destroy(&t_th_spin_mutex);
}

//�߳�����ģ��
/*
int pthread_barrier_destroy(pthread_barrier_t *barrier);
int pthread_barrier_init(pthread_barrier_t *restrict barrier,
              const pthread_barrierattr_t *restrict attr, unsigned count);
int pthread_barrier_wait(pthread_barrier_t *barrier);
�ȴ�ֻ�������֮ǰ�Ķ�����Խ�����Ϻ�
���������̻߳������̶߳��Ტ��ִ��
�����Ҫ�����߳�����ȫȫִ���꣬
�����ټӸ����ϵ��߳�ĩβ��
��Ӧ���߳�ҲҪ�ӡ�
*/
pthread_t t_th_barrier1,t_th_barrier2;
pthread_barrier_t  barrier;
void* t_th_barrier_run(void* d)
{
	int result;
	printf("%d is running\n",pthread_self());
	//sleep(1);
	result = pthread_barrier_wait(&barrier);
	if(result == PTHREAD_BARRIER_SERIAL_THREAD)
		printf("%d is first return:%d\n",pthread_self(),result);
	else
		printf("%d is return:%d\n",pthread_self(),result);
}
void t_th_barrier()
{
	int i = 0;
	char t_name[32];
	pthread_barrier_init(&barrier,NULL, 5);
	for(i = 0; i < 4; i++)
	{
		bzero(t_name,32);
		sprintf(t_name,"t_th_barrier%d",i);
		pthread_create(&t_th_barrier1,
              0,
              t_th_barrier_run, t_name);
	}
	
	pthread_barrier_wait(&barrier);
	printf("all work is completed\n");
	////pthread_join(t_th_barrier1[0], 0);
	////pthread_join(t_th_barrier1[1], 0);
	////pthread_join(t_th_barrier1[2], 0);
	////pthread_join(t_th_barrier1[3], 0);
	//pthread_join(t_th_barrier1, 0);
	sleep(1);
	pthread_barrier_destroy(&barrier);
}
int main(int argc, char* argv[], char** agre)
{
	//1.�鿴��������
	//readEnv(agre);
	
	//2.brk�����ڴ�
	//sysMem();
	
	//3.mmap�����ڴ�ӳ��
	//mmapMem();
	
	//4.mmap�ļ�ӳ��
	//mmapFile();
	
	//5.write, read�������
	//wrIO();
	
	//6.��ȡ�ļ�״̬
	//Getfstat();
	
	//7.dup,dup2
	//testdup();
	
	//8.fcntl
	//testfcntl_dup();
	//testfcntl_fd();
	//testfcntl_fl();
	
	//Ŀ¼����
	//diroperate();
	//scandirtest();
	//diroprate2();
	
	//����
	//system
	//processSystem();
	
	//�ź�
	//1.0�ź�����
	//signalMask();
	
	//2.0��ʱ��
	//settimer();
	
	//3.0masksuspend
	//masksuspend();
	
	//4.sigaction
	//t_sigaction();
	
	//���̼�ͨ��ipc
	//1.�����ܵ�(�����ļ�)
	//t_pipe();
	
	//2.�����ܵ�(�����ļ�)
	//anonymous_pipe();
	
	//3.�����ڴ�(�����ڴ�)
	//t_shm();
	
	//4.��Ϣ����(�����ڴ�)
	//t_send_msg();
	
	//5.�ź���
	//t_sem_a();
	
	//�߳�
	//creatTh();
	
	//�̻߳�����
	//t_mutex_th();
	
	//�߳��ź�
	//t_th_signal();
	
	//������
	//t_th_cond();
	
	//�߳��ź���
	//t_th_sem();
	
	//��д��
	//t_th_rw();
	
	//�̵߳�˽�б���
	//t_th_local();
	
	//������
	//t_th_spin();
	
	//�߳�����
	t_th_barrier();
	//while(1) 
	//{
	//	printf("pid=%d\n",getpid());
	//}
}

