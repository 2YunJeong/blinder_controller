from socket import*
import RPi.GPIO as GPIO
import time
import pickle

servo_pin = 33

#GPIO 세팅 및 PWM 핀을 OUT으로 설정
GPIO.setwarnings(False) #setwarning false 오류 방지를 위해 작성
GPIO.setmode(GPIO.BOARD) #프로그램이 GPIO 핀 번호를 라즈베리파이 보드 번호로 사용하도록 설정 
GPIO.setup(servo_pin, GPIO.OUT) #33번 핀(PWM 핀)을 출력 모드로 설정

#소켓 통신을 위한 설정
clientSock=socket(AF_INET,SOCK_STREAM)
clientSock.connect(('3.38.103.165',9051))

print("Success to connect")

pwm = GPIO.PWM(servo_pin, 50) #주파수 50Hz로 설정
pwm.start(1) #PWM 주기 20ms의 1%로 시작
print("start")

while True:
	data=clientSock.recv(1024) #데이터 read
	cmd = pickle.loads(data)
	print("Received Data :", cmd)

	#cmd(조도센서로부터 받은 값)의 범위에 따라 서보모터 각도 조절
	if cmd < 2200:
		pwm.ChangeDutyCycle(6)
	if cmd >= 2200:
		pwm.ChangeDutyCycle(1)

print("Connection Ending")
pwm.ChangeDutyCycle(1)
print("end")

pwm.stop() #PWM신호 종료
GPIO.cleanup() #GPIO 초기화