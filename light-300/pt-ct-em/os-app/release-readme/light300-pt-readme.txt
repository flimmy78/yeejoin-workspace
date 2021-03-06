    请先阅读“00README.txt”、“readme-emc-emm.txt”。
    ptc分为ptc-m（无线主设备）、ptc-s（无线从设备）,无线主设备同时也是485的从设备。
    0.5.0是新light300系统的的第一个版本。
-------------------------------------------------------------------------------------------------------------------





====================================================================
light300-ptc-m_512k_0.6.0.20150427_Alpha_0 / light300-ptc-s_512k_0.6.0.20150427_Alpha_0
====================================================================
新增：
1：电压及电流有效值加入求平均值

修复：
1：修复内存泄漏bug

优化：
1：相关调试上位机对应的函数


====================================================================
light300-ptc-m_0.5.13.20141217_Alpha_0 / light300-ctc-s_0.5.12.20141217_Alpha_0
====================================================================
新增：
1:加入上位机调试相关接口函数


修复：

优化：

====================================================================
light300-ptc-m_0.5.12.20141204_Alpha_0 / light300-ptc-s_0.5.11.20141204_Alpha_0
====================================================================
新增：
1、加入整体调试精度写入和读出函数

修复：

优化：
1、pt/ct统一数据表格,所有的em统一数据表格, NOTE：原表格数据会损坏


====================================================================
light300-ptc-m_0.5.11.20141029_Alpha_0 / light300-ptc-s_0.5.10.20141029_Alpha_0
====================================================================
新增：
1、添加命令行控制打印的机制。
2、硬件未使用运放，启用7880内部PGA16倍放大。

修复：
1、修复7880没有处理动态内存申请失败的bug。

优化：
1、去除没有必要的打印。


====================================================================
light300-ptc-m_0.5.10.20141009_Alpha_0
====================================================================
1、修改485相关线程名


====================================================================
light300-ptc-s_0.5.9.20141009_Alpha_0
====================================================================
light300-ptc-s_0.5.9.20140828_Alpha_0 ----> light300-ptc-s_0.5.9.20141009_Alpha_0


====================================================================
light300-ptc-s_0.5.9.20140828_Alpha_0
====================================================================
1.修改无线上报数据间隔为1分钟


====================================================================
light300-ptc-m_0.5.9.20140820_Alpha_0
====================================================================
7880模块的新增、优化以及bug修复：
1.更改数据汇集部分中电压电流采样函数
2.解决B相采样波形数据没有上传BUG
3.加入波形采样数据获取函数
4.修改三相三相下视在功率数据越界问题
5.加入三相三接线下B相电压电流采样值为零
6.增加扫描通道接线方式功能
7.去除7880获取电能的相关代码
8.7880使用I2C时，添加超时打印
9.修改扫描通道接线方式函数，加入未接入电表识别
10.重写用于7880通信的i2c代码，以避免死等问题
11.优化相位自动调试部分代码。
12.修复谐波获取过程中计算中数据类型的问题
13.修复切换通道后相位获取问题
14.增加手动更改参数接口
15.B相频率增加接线方式判断。
16.修改em从设备调试参数写入错误问题
17.加入调试单通道互感器相移功能

RS485模块的新增、优化以及bug修复：
1.增加RS485读数据时的超时时间。
2.给用于接收无线数据的485接收数据，添加超时判断
3.修改释放信号量的位置，在数据处理完后再释放信号量
4.增加RS485接收超时时长，解决因系统繁忙导致RS485接收数据超时问题
5.由于RS485发送波特率太低，导致看门口复位，所以在发送函数中加入喂狗操作

SI4432模块的新增、优化以及bug修复：
1.修改设置无线主节点ID函数。
2.当4432处于TX_ERROR_STATE时,重新初始化4432
3.无线出错时复位SI4432模块
4.将无线接收信号量的值改为1
5.增加rfslave线程的栈空间
6.屏蔽掉ECHO相关的函数
7.去除死循环
8.增加获取信号量失败时的打印
9.复位时如果SI4432的SPI初始化不成功就重复初始化
10.初始化时操作si443x的sdn引脚

其他模块的新增、优化以及bug修复：
1.修复b相电压电流采用应答命令未设置bug;使能实时无功电能测试代码
2.用printf_syn代替rt_kprintf,以避免打印交错
3.添加编译ct作伪无线主节点的配置代码；添加统计线程使用cpu的功能。
4.增加将组网信息写入flash的命令。
5.增加组网失败时手动配置组网信息的命令。
6.修改无线主节点相关条件编译；添加打印当前systicks值
7.添加app_cfg_ct_m.h文件
8.将应用线程的初始化放到初始化线程中
9.修改代码，使其在未配置数据之前不获取数据
10.修改清空无线白名单函数
11.调整7880使用的引脚
12.开启独立看门狗
13.新硬件的em与pt、ct板上的7880使用的引脚不同
14.独立看门狗超时时间为1220ms,只在led线程中喂狗
15.流模式时，也需要喂狗去掉宏定义，将UART5初始化编译进入所有工程，防止shell线程工作不正常



====================================================================
light300-ptc-s_0.5.9.20140820_Alpha_0
====================================================================
7880模块的新增、优化以及bug修复：
1.更改数据汇集部分中电压电流采样函数
2.解决B相采样波形数据没有上传BUG
3.加入波形采样数据获取函数
4.修改三相三相下视在功率数据越界问题
5.加入三相三接线下B相电压电流采样值为零
6.增加扫描通道接线方式功能
7.去除7880获取电能的相关代码
8.7880使用I2C时，添加超时打印
9.修改扫描通道接线方式函数，加入未接入电表识别
10.重写用于7880通信的i2c代码，以避免死等问题
11.优化相位自动调试部分代码。
12.修复谐波获取过程中计算中数据类型的问题
13.修复切换通道后相位获取问题
14.增加手动更改参数接口
15.B相频率增加接线方式判断。
16.修改em从设备调试参数写入错误问题
17.加入调试单通道互感器相移功能

SI4432模块的新增、优化以及bug修复：
1.修改设置无线主节点ID函数。
2.当4432处于TX_ERROR_STATE时,重新初始化4432
3.无线出错时复位SI4432模块
4.将无线接收信号量的值改为1
5.增加rfslave线程的栈空间
6.屏蔽掉ECHO相关的函数
7.去除死循环
8.增加获取信号量失败时的打印
9.复位时如果SI4432的SPI初始化不成功就重复初始化
10.初始化时操作si443x的sdn引脚

其他模块的新增、优化以及bug修复：
1.修复b相电压电流采用应答命令未设置bug;使能实时无功电能测试代码
2.用printf_syn代替rt_kprintf,以避免打印交错
3.添加编译ct作伪无线主节点的配置代码；添加统计线程使用cpu的功能。
4.增加将组网信息写入flash的命令。
5.增加组网失败时手动配置组网信息的命令。
6.修改无线主节点相关条件编译；添加打印当前systicks值
7.添加app_cfg_ct_m.h文件
8.将应用线程的初始化放到初始化线程中
9.修改代码，使其在未配置数据之前不获取数据
10.修改清空无线白名单函数
11.调整7880使用的引脚
12.开启独立看门狗
13.新硬件的em与pt、ct板上的7880使用的引脚不同
14.独立看门狗超时时间为1220ms,只在led线程中喂狗
15.流模式时，也需要喂狗去掉宏定义，将UART5初始化编译进入所有工程，防止shell线程工作不正常


====================================================================
light300-ptc_0.5.9.20140820_Alpha_0
====================================================================
主要功能概述如下：
1.无线芯片采用Silicon Labs公司的SI4432-B1版本。
3.用ade7880采集CT测的CT负荷和CT电流。
4.通过SI4432无线芯片将ade7880采集的数据发送给PT主设备或CT主设备。


====================================================================
light300-ptc-m_0.5.3.20140604_Alpha_0
====================================================================
新增：
1.加入波形采样数据获取函数
2.增加采集7880的有功功率、电压、电流、视在功率的采集并传输
3.增加扫描通道接线方式功能
4.增加将组网信息写入flash的命令
5.增加组网失败时手动配置组网信息的命令
优化：
1.修改打印信息
2.用printf_syn代替rt_kprintf,以避免打印交错
3.重写用于7880通信的i2c代码，以避免死等问题
4.增加RS485读数据时的超时时间
bug修复：
1.变量定义时初始化为0
2.修改三相三相下视在功率数据越界问题
3.加入三相三接线下B相电压电流采样值为零
4.增加SI4432接收时检查SI4432的状态与状态机的状态是否一致

====================================================================
light300-ptc-s_0.5.3.20140604_Alpha_0
====================================================================
新增：
1.加入波形采样数据获取函数
2.增加采集7880的有功功率、电压、电流、视在功率的采集并传输
3.增加扫描通道接线方式功能
4.增加将组网信息写入flash的命令
5.增加组网失败时手动配置组网信息的命令
优化：
1.修改打印信息
2.用printf_syn代替rt_kprintf,以避免打印交错
3.重写用于7880通信的i2c代码，以避免死等问题
bug修复：
1.变量定义时初始化为0
2.修改三相三相下视在功率数据越界问题
3.加入三相三接线下B相电压电流采样值为零
4.增加SI4432接收时检查SI4432的状态与状态机的状态是否一致

====================================================================
light300-ptc-m_0.5.2.20140515_Alpha_0
====================================================================
新增：

优化：

bug修复：
1. 修复rt_free(NULL)的bug。
2. 动态申请内存失败后，将相关的动态申请的内存释放掉。

====================================================================
light300-ptc-s_0.5.2.20140515_Alpha_0
====================================================================
新增：

优化：

bug修复：
1. 修复rt_free(NULL)的bug。
2. 动态申请内存失败后，将相关的动态申请的内存释放掉。

====================================================================
light300-ptc-m_0.5.1.20140513_Alpha_0
====================================================================
新增：
1. RS485长包数据传输。
2. 加入1~63次谐波分析原始数据的获取
3. 加入切换检测电表时自动获取电表常数用以配置7880脉冲输出功能

优化：
1. 将消息邮箱si4432_recv_mb的大小给位10
bug修复：
====================================================================
light300-ptc-s_0.5.1.20140513_Alpha_0
====================================================================
新增：
1. 加入1~63次谐波分析原始数据的获取
2. 加入切换检测电表时自动获取电表常数用以配置7880脉冲输出功能

优化：
1. 将消息邮箱si4432_recv_mb的大小给位10
bug修复：


====================================================================
light300-ptc-m_0.5.0.20140430_Alpha_0
====================================================================
该版本是新light300系统的ptc-m的第一个版本。


该版本主要功能如下：
1、通过485总线与emm-m设备交互，完成半自动无线组网，并记录自己的星型无线网络中的从节点sn。
2、收集自己无线网络从节点的数据，在emm-m设备通过485总线查询时，将这些采集数据返回。


该版本的缺陷：
1、不能完成大于256字节的485长包传输。
2、没有完成仔细的自测，软件中可能存在低级问题。
3、代码需要重新整理。



====================================================================
light300-ptc-s_0.5.0.20140430_Alpha_0
====================================================================
该版本是新light300系统的ptc-s的第一个版本。


该版本主要功能如下：
1、配合无线主节点完成半自动组网。
2、定时上报采集数据给无线主节点。


该版本的缺陷：
1、没有完成仔细的自测，软件中可能存在低级问题。
2、代码需要重新整理。





==============================================================================================================
==============================================================================================================
==============================================================================================================
==============================================================================================================





====================================================================
light300-pt_0.1.6.20131125_Alpha_0
====================================================================
新增：


优化：


bug修复：
1、增加7880读取电压、电流值时的超时处理，以免7880故障导致系统假死。


====================================================================
light300-pt_0.1.5.20131121_Alpha_0
====================================================================
新增：
1、串口读取7880各项参数的ABC三相的值

优化：


bug修复：
1、在配置完a相电流增益后，没有使能7880的写保护


====================================================================
light300-pt_0.1.4.20131120_Alpha_0
====================================================================
新增：


优化：


bug修复：
1、对SPI3添加保护，硬件上，SPI3由ade7880和si4432共用，在读取ade7880数据时，有可能被si4432中断抢断SPI3的使用权，导致ade7880读取数据错误。
2、初始化7880后将7880的片选拉高。
3、对SPI3时钟进行切换时，将SPI3的极性也进行切换。因为4432与7880手册上的极性不同。


====================================================================
light300-pt_0.1.3.20131103_Alpha_0
====================================================================
新增：
1.增加指示灯。
2.增加读取和设置7880寄存器的命令。



优化：


bug修复：




====================================================================
light300-pt_0.1.2.20131101_Alpha_0
====================================================================
新增：
1.在请求关联时带上设备类型。



优化：
1.优化串口读取和配置参数功能。
2.增加接收成功和失败超时。
3.清理无效代码。


bug修复：
1.超时时间加长。





====================================================================
light300-pt_0.1.1.20131031_Alpha_0
====================================================================
新增：
1.在发送请求关联命令时带上设备的SN号。
2.增加通过串口读取和设置SN号、读取和设置同步字、读取系统运行时间。
3.增加flash读写功能。
4.增加SI4432出错超时判断。



优化：
1.代码整理。
2.修改程序中死等的情况，因为要检测串口的数据。
3.更新sinkinfo_common.h文件中的内容。


bug修复：




====================================================================
light300-pt_0.1.0.20131025_Alpha_0
====================================================================
主要功能概述如下：
1.无线芯片采用Silicon Labs公司的SI4432-B1版本。
2.PT设备与电能表采集器实现自组星型网络。
3.用ade7880采集PT测的PT负荷、PT二次压降。
4.通过SI4432无线芯片将ade7880采集的数据发送给电表采集器。


