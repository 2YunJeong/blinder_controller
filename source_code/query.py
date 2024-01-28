from influxdb import InfluxDBClient
from socket import *
import pickle,time

def main():
    #데이터베이스에 대한 ip port 계정을 입력
    host = 'localhost'
    port = 8086
    user = 'admin'
    password = 'admin'
    dbname = 'test3'

    #데이터베이스의 어떤부분에 접근할지 설정
    query = 'select * from light order by time desc limit 20'

    #jetson nano와의 소켓통신을 위한 설정
    serverSock = socket(AF_INET, SOCK_STREAM)
    serverSock.bind(('', 9051))
    serverSock.listen(1)
    connectionSock, addr = serverSock.accept()

    while True:
        aver=0
        total=0
        client = InfluxDBClient(host, port, user, password, dbname) #Local ip에 있는 influxdb의 데이터베이스에 접근
        print("Querying data: " + query)
        result = client.query(query) #데이터베이스의 접근한부분에서 가져온데이터를 result에 저장

        #딕셔너리 형태의 result에서 키가 data인 값을 가져옴(조도센서로부터 받은 데이터)
        for point in result.get_points():
            print(point)
            total += point['data']

        print("total : ", (total))
        aver = total/20
        print("average : ", (aver))

        data = pickle.dumps(aver)
        connectionSock.sendall(data) #jetson nano로 데이터 전송

        print('메시지를 보냈습니다.')
        time.sleep(2)

if __name__ == '__main__':
    main()
