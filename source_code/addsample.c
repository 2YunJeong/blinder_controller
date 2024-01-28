#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h> 
#include <wiringPi.h> 
#include <wiringPiSPI.h> 

/*자신의 influx db와 매칭*/
#define PORT PORT
#define IP_ADDRESS IP_ADDRESS    
#define DATABASE DATABASE
#define USERNAME USERNAME
#define PASSWORD PASSWORD
#define HOSTNAME "admin"

#define SECONDS 2
#define LOOPS   1000
#define BUFSIZE 8196
#define CS_MCP3208 8 //GPIO 8 
#define SPI_CHANNEL 0 
#define SPI_SPEED 1000000 //1Mhz
  
int read_mcp3208_adc(unsigned char adcChannel) 
{
    unsigned char buff[3];
    int adcValue = 0;
    
    //MCP3208과 SPI통신을 하기 위한 통신 패킷 설정
    //MCP3208에서 사용할 채널 번호를 buff[0]과 buff[1]에 저장
    buff[0] = 0x06 | ((adcChannel & 0x07) >> 2);
    buff[1] = ((adcChannel & 0x07) << 6);
    buff[2] = 0x00;
    
    digitalWrite(CS_MCP3208, 0); //GPIO 8번에 0(LOW) 출력
    wiringPiSPIDataRW(SPI_CHANNEL, buff, 3); //MCP3208의 센서 값을 read
    
    //데이터 송수신 후의 데이터 형식
    buff[1] = 0x0f & buff[1];
    adcValue = (buff[1] << 8 ) | buff[2];
    
    digitalWrite(CS_MCP3208, 1); //GPIO 8번에 1(HIGH) 출력
    
    return adcValue;
}

int pexit(char * msg)
{
    perror(msg);
    exit(1);
}

int main()
{
    int i;
    int sockfd;
    int loop;
    int ret;
    char header[BUFSIZE];
    char body[BUFSIZE];
    char result[BUFSIZE];
    unsigned char adcChannel_light = 0;
	int adcValue_light = 0;
	float cout_light;
	float vout_oftemp;
	float percentrh = 0;
	float supsiondo = 0;
    static struct sockaddr_in serv_addr;
    
    printf("start\n");
    if(wiringPiSetupGpio() == -1) //프로그램이 BCM의 GPIO 핀 번호를 사용하도록 설정
    {
       fprintf(stdout, "Unable to start wiringPi :%s\n", strerror(errno));
        return 1;
    }
    
    if(wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) == -1) //SPI통신 채널과 속도 설정(ch 0, 1MHz)
    {
        fprintf(stdout, "wiringPiSPISetup Failed :%s\n", strerror(errno));
        return 1;
    }
    
    pinMode(CS_MCP3208, OUTPUT); //GPIO 8번의 상태를 출력모드로 설정

    printf("Connecting socket to %s and port %d\n", IP_ADDRESS, PORT);
    if((sockfd = socket(AF_INET, SOCK_STREAM,0)) <0) 
        pexit("socket() failed");

    /*소켓 통신을 위한 설정*/
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    serv_addr.sin_port = htons(PORT);
    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <0) 
        pexit("connect() failed");

    for(loop=0;i<LOOPS; i++) /*loop의 횟수만큼 조도센서의 센서 값을 받아 서버의 influx db에 저장*/
        adcValue_light = read_mcp3208_adc(adcChannel_light);
        printf("==============================\n");
        printf("light sensor = %u\n", adcValue_light);
        
        /*헤더에 요청내용과 데이터 응용할 데이터 베이스에 대한 정보를 담아 전송, 보낼 데이터에대한 크기도 전송*/
        /*바디에는 전달할 실제 데이터들을 담아 전송*/
        sprintf(body, "light,host=%s data=%.3f   \n", HOSTNAME, (double)adcValue_light);
        sprintf(header, 
            "POST /write?db=%s&u=%  s&p=%s HTTP/1.1\r\nHost: influx:8086\r\nContent-Length: %ld\r\n\r\n", 
             DATABASE, USERNAME, PASSWORD, strlen(body));
        printf("Send to InfluxDB the POST request bytes=%d \n\n%s\n",strlen(header), header);
        write(sockfd, header, strlen(header));
        if (ret < 0)
            pexit("Write Header request to InfluxDB failed");

        printf("Send to InfluxDB the data bytes=%d \n\n%s\n",strlen(body), body);
        ret = write(sockfd, body, strlen(body));
        if (ret < 0)
            pexit("Write Data Body to InfluxDB failed");

        /*influx db에 데이터를 저장후 그에대한 응답을 받음*/ 
        ret = read(sockfd, result, sizeof(result));
        if (ret < 0)
            pexit("Reading the result from InfluxDB failed");
        result[ret] = 0;
        printf("Result returned from InfluxDB. Note:204 is Success\n\n%s\n",result);
        printf(" - - - sleeping for %d secs\n\n",SECONDS);
        sleep(SECONDS);
    }
    close(sockfd);
}

