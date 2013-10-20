#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ncurses.h>
#include <pthread.h>
#include <locale.h>

typedef struct
{
	unsigned long up_bytes;
	unsigned long dn_bytes;
	unsigned long up_packs;
	unsigned long dn_packs;
}SP;

int get_traffic(SP *sp,char *interface)
{
	char *buf=NULL;
	size_t len;
	FILE *fp;

	if((fp=fopen("/proc/net/dev","rb")) == NULL)
	{
		perror("Read /proc/net/dev Error!");
		return -1;
	}

	while(!feof(fp))
	{
		getline(&buf,&len,fp);
		if(buf == NULL)
			return -1;

		if(strstr(buf,interface))
			break;
		
		free(buf);
		buf=NULL;
	}

	if(feof(fp))
	{
		printf("Can Not Find %s Interface!\n",interface);
		fclose(fp);

		return -1;
	}

	sscanf(buf,"%*[^:]: %ld %ld %*d %*d %*d %*d %*d %*d %ld %ld",
			&sp->dn_bytes,&sp->dn_packs,
			&sp->up_bytes,&sp->up_packs);

	free(buf);
	fclose(fp);
	return 0;
}

void _quit(WINDOW *win)
{
	while(getchar() != 'q')
		continue;

	delwin(win);
	echo();
	endwin();

	exit(0);
}

void help(void)
{
	printf("Use:\n");
	printf("netsp [interface]\n");

	exit(-1);
}

int main(int argc,char **argv)
{
	unsigned long up_times=0;
	unsigned long dn_times=0;
	SP start,end,temp;
	WINDOW *win;
	int x,y;
	pthread_t thread;

	if(argc != 2)
		help();

	setlocale(LC_ALL,"");
	if(get_traffic(&start,argv[1]) == -1)
		return -1;

	initscr();
	curs_set(0);
	noecho();

	getmaxyx(stdscr,y,x);
	win=newwin(12,30,y/2-10/2,x/2-30/2);
	refresh();
	box(win,0,0);
	wrefresh(win);
	pthread_create(&thread,NULL,(void *)_quit,win);

	while(1)
	{
		get_traffic(&temp,argv[1]);
		sleep(1);
		++up_times;
		++dn_times;
		get_traffic(&end,argv[1]);

		if(up_times % 10 == 0)
		{
			up_times=1;
			start.up_bytes=temp.up_bytes;
		}

		if(dn_times % 10 == 0)
		{
			dn_times=1;
			start.dn_bytes=temp.dn_bytes;
		}

		if(end.up_bytes-temp.up_bytes == 0)
		{
			up_times=1;
			temp.dn_bytes=start.dn_bytes;
			get_traffic(&start,argv[1]);
			start.dn_bytes=temp.dn_bytes;
		}

		if(end.dn_bytes-temp.dn_bytes == 0)
		{
			dn_times=1;
			temp.up_bytes=start.up_bytes;
			get_traffic(&start,argv[1]);
			start.up_bytes=temp.up_bytes;
		}

		mvwprintw(win,2,6,"已上传 %.2f MB",
				(double)end.up_bytes/1024/1024);
		mvwprintw(win,3,6,"已下载 %.2f MB",
				(double)end.dn_bytes/1024/1024);
		mvwprintw(win,5,6,"已上传包数量 %ld",end.up_packs);
		mvwprintw(win,6,6,"已下载包数量 %ld",end.dn_packs);
		mvwprintw(win,8,6,"上传速度 %.2f kb/s",
				(double)(end.up_bytes-start.up_bytes)/up_times/1024);
		mvwprintw(win,9,6,"下载速度 %.2f kb/s",
				(double)(end.dn_bytes-start.dn_bytes)/dn_times/1024);
		wrefresh(win);

	}

	return 0;
}
