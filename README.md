# detours拦截进程中的堆内存分配操作
## 原理

首先用detours工具拦截HeapAlloc和HeapFree的API，写入dll文件中。

然后用CreateRemoteThread()将dll文件远程注入到进程中，使其随进程运行，从而监控进程中的堆内存分配操作。

## 使用方法

解压release.zip，打开cmd进入解压目录，输入dlltest  进程id，即可将dll注入到相应进程。

关闭进程后，会将监控到的最后没有释放的内存地址和字节大小写入到C:/User/username目录下的workLog.txt文件中。