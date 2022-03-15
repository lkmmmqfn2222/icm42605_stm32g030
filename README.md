#  ICM42605 六轴传感器测试驱动

## 介绍

基于 STM32G030 的 hal 库写的 ICM42605 的测试工程，单片机通过硬件 SPI1 接口读取 加速度传感器的数据，测试工程支持 IAR For ARM 9.1及以上版本以及 keil5 MDK，详细描述可以查看 [《ICM-42605 6轴MEMS加速度计陀螺仪运动传感器数据的读取》](https://blog.csdn.net/u012121390/article/details/117227520)

## 软件架构

使用 hal 库编写，每隔 1s 采集一次加速度传感器的数据，并通过单片机的串口1打印

## 使用方法

1. 首先将传感器和 STM32G030 的开发板通过杜邦线连接

| ICM42605接线  |      |
| :------------ | :--- |
| ICM42605-SCK  | PA1  |
| ICM42605-MISO | PA6  |
| ICM42605-MOSI | PA7  |
| ICM42605-CS   | PA4  |
| 串口接线      |      |
| RXD           | PB7  |
| TXD           | PB6  |
|               |      |

2. 打开 IAR/MDK 的工程，下载程序到开发板中，观察现象即可



