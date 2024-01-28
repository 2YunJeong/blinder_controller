# blinder_controller
빛의 양에 따라 블라인더를 조절하는 자동 블라인더 조절기 프로젝트입니다.
---------------
## [개발 환경]
- language : C language, python
- Embedded : Raspberry Pi 4, NVIDIA Jetson Nano
- Cloud Service : AWS
---------------
## [개발 기간]
- 2022.11.18 ~ 2022.12.23
---------------
## [프로젝트 구성]
- Raspberry Pi 4
    + 조도센서를 사용하여 빛의 양 감지
    + C-Rest API를 사용하여 AWS에 데이터 전달
- AWS
    + Raspberry Pi 4를 통해 전달받은 데이터를 influxdb에 저장
    + influxdb에 저장된 데이터는 Grafana를 통해 시각화
- NVIDIA Jetson Nano
    + AWS에 저장된 데이터는 python Socket을 통해 받음
    + 서보모터를 사용하여 빛의 양에 따라 각도를 조절하고, 이를 활용하여 블라인드를 자동으로 조절하게 함
---------------
## [프로젝트 기능]
스스로 블라인드를 조정할 필요 없이 햇빛의 양에 따라 블라인드를 자동으로 조절하게 함
