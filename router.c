#include <dos.h>
#include <stdio.h>
#include <bios.h>

#define INTA 0x60
#define INTB 0x70
#define MAX 6

unsigned char class, type, number, f[2];
unsigned handle;
unsigned char BUFF[600], BUFF2[600], DATA[100];
char word1[256], word[256];
unsigned int length = 0, length2 = 0;
unsigned bp, di, si, ds, es, dx, cx, bx, ax, ip, cs, flags;
unsigned char src1[6];
unsigned char src2[6];
unsigned char brd_add[] = "\xff\xff\xff\xff\xff\xff";
unsigned char d_type[] = "\xab\xcd";
int de = -1;
int PACKET_DROP = 0, PACKET_SENT = 0, PACKET_RCV = 0;
typedef struct table
{
	unsigned char iip[2];
	unsigned char mac[6];
}table;

table IP[MAX];
void send_pkt(unsigned char *buffer, unsigned length, unsigned char INT);
void flush(char word[]);

void create_iptable()
{
	/* 0,4,5 - network1; 1,2,3 - network2 */
	IP[0].iip[0] = 1; IP[0].iip[1] = 2;

	memcpy(IP[0].mac, "\x08\x00\x27\xC3\xDB\xD9", 6);
	memcpy(IP[1].mac, "\x08\x00\x27\xA3\x89\xE4", 6);
	memcpy(IP[4].mac, "\x08\x00\x27\x39\xD9\x50", 6);
	memcpy(IP[5].mac, "\x08\x00\x27\x6A\x16\xF1", 6);
	IP[1].iip[0] = 2; IP[1].iip[1] = 3;
	/*IP[1].mac[3] = 0x6A; IP[1].mac[4] = 0x16; IP[1].mac[5] = 0xF1; */
	strcpy(IP[2].iip, "\x01\x01");
	strcpy(IP[3].iip, "\x02\x01");
	memcpy(IP[4].iip, "\x01\x03", 2);
	memcpy(IP[5].iip, "\x02\x02", 2);
	memcpy(IP[2].mac, src1, 6);
	memcpy(IP[3].mac, src2, 6);


}

void get_mac_address(unsigned char * src, unsigned char INT)
{
	int i;
	union REGS inregs, outregs;
	struct SREGS segregs;
	char far *buf;
	segread(&segregs);
	buf = (char far *)src;
	segregs.es = FP_SEG(buf);
	inregs.x.di = FP_OFF(src);
	inregs.x.cx = 6;
	inregs.h.ah = 6;
	int86x(INT, &inregs, &outregs, &segregs);
	printf("MAC Address: ");
	for(i = 0; i < 6; i++)
		printf("%02x:",src[i]);
	printf("\n");
}

void free(unsigned int l)
{
	int i;
	for(i = 0; i < l; i++){
		BUFF[i] = 0x00;
		BUFF2[i] = 0x00;}
}

void route(unsigned int l, int t, unsigned char * buff)
{
	int i, flag = 0, j;
	de = -1;
      /*
	for(i = 0; i < 6; i++)
		if(BUFF[i] == 0xff)
			return;
			*/
	/*if(BUFF[12] == 0xAB && BUFF[13] == 0xCD)
	if(memcmp(buff, src1, 6) == 0 || memcmp(buff, src2, 6) == 0)*/
	{
		printf("%x %x",buff[14], buff[15]);
		printf("Packet received...\n");
		PACKET_RCV++;
		for(i = 0; i < MAX; i++)
		{
			if(IP[i].iip[0] == buff[14] && IP[i].iip[1] == buff[15])
			{
				flag = 1;
				de = i;
				printf("Destination MAC :");
				break;
			}
		}

		if(flag == 0)
		{
			printf("Destination IIP not in table...Packet dropped\n");
			PACKET_DROP++;
		}
		else
		{
			/*printf("%d", de);  */
			for(j = 0; j < 6; j++)
				printf("%02x:", IP[de].mac[j]);
			printf("\n");
		}
		if(de == 2 || de == 3)
		{
			printf("Router's MAC provided..not forwarded\n");
			return;
		}
		fflush(stdout);
		printf("Recieved MSG : ");
		for(i = 12; buff[i] != 0x00; i++)
		{
			word[i-12] = buff[i];
			putchar(buff[i+6]);
		}
		word[i]='\0';
		printf("\n");
		memcpy(DATA, IP[de].mac, 6);
		memcpy(DATA+12, word,i-12);
		if(IP[de].iip[0] == 0x01)
		{
			memcpy(DATA+6, src1, 6);
			send_pkt(&DATA, 100, INTA);
		}
		else if(IP[de].iip[0] == 0x02)
		{
			memcpy(DATA+6, src2, 6);
			send_pkt(&DATA, 100, INTB);
		}
		flush(word);
		free(l);
	}

}

void interrupt receiver(bp, di, si, ds, es, dx, cx, bx, ax, ip, cs, flags)
{
	/*printf("Reciever ax = %d Packet Size %d\n",ax, cx);
	*/
	if(ax == 0)
	{
		es = FP_SEG(BUFF);
		di = FP_OFF(BUFF);
		length = cx;
	}
	if(ax == 1)
	{
		if(memcmp(BUFF, src1, 6) == 0){
		route(length, 1, BUFF);
		length = 0;                      }
	}
}

void interrupt receiver2(bp, di, si, ds, es, dx, cx, bx, ax, ip, cs, flags)
{

	if(ax == 0)
	{
		es = FP_SEG(BUFF2);
		di = FP_OFF(BUFF2);
		length2 = cx;
	}
	if(ax == 1)
	{
		if(memcmp(BUFF2, src2, 6) == 0){
		route(length2, 2, BUFF2);
		length2 = 0;                   }
	}
}

void driver_info(unsigned char INT)
{
	union REGS a, b;
	struct SREGS s;
	char far *p;
	a.h.ah = 1;
	a.h.al = 255;
	int86x(INT, &a, &b, &s);
	class = b.h.ch;
	type = b.x.dx;
	number = b.h.cl;
	if(INT == '\x60')
		printf("Packet Driver 1 : \n");
	else
		printf("Packet Driver 2 : \n");
	printf("Version : %x\n", b.x.bx);
	printf("Class : %x\n", b.h.ch);
	printf("No : %x\n", b.h.cl);
	printf("Type : %x\n", b.x.dx);
	p = MK_FP(s.ds, b.x.si);
	printf("Name : %s\n", p);
}

void access_type(unsigned char INT)
{
	union REGS a, b;
	struct SREGS s;
	printf("%x %x %x\n", class, type, number);
	a.h.al = class;
	a.x.bx = type;
	a.h.dl = number;
	a.x.cx = 0;
	a.h.ah = 2;
	if(INT == 0x60){
	s.es = FP_SEG(receiver);
	a.x.di = FP_OFF(receiver);
	}
	else
	{
		s.es = FP_SEG(receiver2);
		a.x.di = FP_OFF(receiver2);
	}
	f[0] = 0x01;
	f[1] = 0x01;
	s.ds = FP_SEG(f);
	a.x.si = FP_SEG(f);
	int86x(INT, &a, &b, &s);
	handle = b.x.ax;
	printf("CARRY FLAG : %x\n", b.x.cflag);
	printf("handle access type: %x\n", handle);
}

void send_pkt(unsigned char * buffer, unsigned length, unsigned char INT)
{
	int i;
	union REGS a, b;
	struct SREGS s;
	a.h.ah = 4;
	s.ds = FP_SEG(buffer);
	a.x.si = FP_OFF(buffer);
	a.x.cx = length;
	int86x(INT, &a, &b, &s);
	/*printf("Sending packet...\n"); */
	if(b.x.cflag)
	{
		printf("Can not send...Packet dropped\n");
		PACKET_DROP++;
		exit(1);
	}
	printf("Sending packet using interrupt ");
	printf("%x\n", INT);
	for(i=0; i<40; i++)
		printf("%02x:", buffer[i]);
	printf("\n");
	printf("SUCCESS..Packet send\n");
	PACKET_SENT++;
}

void create_packet(unsigned char *dest, unsigned char *src)
{

	memcpy(DATA, dest, 6);
	memcpy(DATA, src, 6);
	memcpy(DATA, d_type, 2);

}

void flush(char word[])
{
	int i;
	for(i = 14; i < 100; i++)
		DATA[i] = 0x00;
	for(i = 0; i < 256; i++)
		word[i]='\0';

}

void get_rcv_mode(unsigned char INT)
{
	union REGS a, b;
	struct SREGS s;
	a.h.ah = 21;
	a.x.bx = handle;
	int86x(INT, &a, &b, &s);

	printf("Rcv mode : %x\n", b.x.ax);
}

void set_rcv_mode(unsigned char INT)
{
	union REGS a, b;
	struct SREGS s;
	a.h.ah = 20;
	a.x.bx = handle;
	a.x.cx = 6;
	int86x(INT, &a, &b, &s);

}

void release_type(unsigned char INT)
{
	union REGS a, b;
	struct SREGS s;
	a.h.ah = 3;
	a.x.bx = handle;
	int86x(INT, &a, &b, &s);
	if(b.x.cflag)
	{
		printf("Error release type : \n");
		exit(1);
	}
	printf("Released handle =%x\n", handle);

}

void main()
{
	unsigned char *my_ad = (unsigned char *)malloc(6);
	char c;
	int length, k, j, flag = 0;
	driver_info(INTA);
	driver_info(INTB);
	get_mac_address(src1, INTA);
	get_mac_address(src2, INTB);
	create_iptable();
	access_type(INTA);
	access_type(INTB);
	printf("Handle in main : %x\n",handle);
	get_rcv_mode(INTA);
	set_rcv_mode(INTA);
	get_rcv_mode(INTB);
	set_rcv_mode(INTB);
	while(1)
	{
		for(j = 0; j < 99; j++)
		{
			/*printf("Press Enter to forward the packet\n");
			*/c=getch();
			if((int)c == 13)
				break;
			if((int)c == 27)
			{
				flag = 1;
				break;
			}

			word[j] = c;
			putch(word[j]);
		}

		printf("\nMSG : %s\n", word);
		length = strlen(word);
		if(flag == 1)
			break;
		memcpy(DATA, IP[de].mac, 6);
		if(de == 1)
			memcpy(DATA+6, src2, 6);
		else
			memcpy(DATA+6, src1, 6);
		memcpy(DATA+12, d_type, 2);
		memcpy(DATA+14, IP[de].iip, 2);
		if(de == 1)
			memcpy(DATA+16, IP[3].iip, 2);
		else
			memcpy(DATA+16, IP[2].iip, 2);

		k = 18;
		for(j = 0; j < length; j++)
		{
			DATA[k++] = word[j];
		}

		printf("%s\n", DATA);
		if(de == 0)
		send_pkt(&DATA, 100, INTA);
		else
		send_pkt(&DATA, 100, INTB);
		flush(word);
	}
	release_type(INTA);
	release_type(INTB);
	printf("Packet sent: %d\n", PACKET_SENT);
	printf("Packet dropped: %d\n", PACKET_DROP);
	printf("Packet received: %d\n", PACKET_RCV);
}