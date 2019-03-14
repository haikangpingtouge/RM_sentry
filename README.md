# sentry（哨兵代码）v2.0

## 文件层次

* Drivers(HAL库驱动层)
* Middlewares(freertos层)
* MDK-ARM （keil工程文件和编译文件）
* User（用户层）
  + bsp (用户底层配置层)
    + drivers（用户模块设备层）
    - counters(控制器等算法)
    - data_struture(用户自定义数据结构)
    - modules（模块和外设）
  + control（控制层）
  + inc（应用层头文件）
  + src（应用层源文件）
* Inc（主函数和中断层头文件）
* Src（主函数和中断层源文件）

## 库说明

STM32CubeF4 Firmware Package V1.21.0 / 23-February-2018

## 系统支持

FreeRTOS

## 开发板支持

RM新板,RM旧板,彬哥第一代板（f427IIHx）

## 代码命名规则说明

类型|命名规则|示例
-|-|-
 函数名|大驼峰|MyName
 普通变量|全小写,连接加下划线|my_name
 结构体声明|小驼峰加后缀Struct|myNameStruct
 结构体定义|小驼峰加后缀_t|myName_t
 枚举声明|小驼峰加后缀Enum|myNameEnum
 枚举定义|小驼峰加后缀_e|myName_e
 指针类型|相应类型加前缀p|普通变量指针类型pmy_name，结构体指针类型pmyName_t

## 底层外设支持（bsp）

外设|说明
-|-
串口|串口空闲中断+DMA+消息队列不定长度接收
can|can中断+消息队列接收

## 模块设备支持（modules）

模块设备|模块名称
-|-
电机|6623  3508  2006 maxion
遥控|大疆遥控dbus
陀螺仪|GY555
上位机|匿名v2.6 平衡小车之家
编码器|增量AB相欧姆龙 E6A2-CW3C

## 数据结构和算法（data_struture，counters）

数据结构和算法|算法名称
-|-
pid控制器|普通pid，模糊pid, 专家pid
数据结构|循环队列
滤波器|加权滑动平均滤波


## 控制流程手稿
  1. 所有任务初始化，然后挂起，进入模块自检，判断初始化完毕，进入数据解析，数据接收正常判断，开启数据校验任务
    1. 如果全部成功，进入数据校验任务
    2. 如果次要模块离线，则关闭相应功能
    3. 如果重要模块离线，则蜂鸣器报警和led灯提示，然后进入oled显示错误任务，打印错误信息
  2. 数据校验，检查哪个模块正常并且选择
    1. 手动对位校准 
      1. 遥控碰边，轻触开关时间持续t ms，设置编码器零点
      2. 以恒定速度回跑半米，检验激光测距和超声波测距，和编码器的值相比较
         1. 距离从小到大，判断其变化率是否稳定，
         2. 然后再回跑半米，从大到小
      3. 开启所有任务，删除手动自检任务
    2. 自动对位校准
      1. 激光测距和超声波比较，值是否在正常范围内，即半米，tms内数据的波动范围
        1. 两传感器各采取50个数据，数据间隔为t，求出平均值，再求方差，取方差小的，即波动性小
        2. 设置编码器零点，然后20cm附近抖动跑，记录30个数据，求据间隔为t，求出平均值，再求方差，取方差小的，即波动性小
        3. 已选出的传感器往近方向跑，(路径做位置闭环)，碰到轻触开关并且持续tms，设置编码器零点
        4. 如果跑飞，数据不对，马上切换遥控自检
      2. 开启所有任务，删除自动自检任务
      3. 如果没有剩余时间，开启紧急模式，生成随机数，在过道弯道中间随机速度跑
  2. 自动控制
  3. 跑飞检测和控制


## 模块功能要求
  1. 编码器，可变方向增加记数
