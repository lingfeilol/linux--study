#ifndef __CONFIG_H__
#define __CONFIG_H__  

//加载，在程序启动时将所有的配置设置加载到内存(可以自己写到链表上)
void load(const char* conf_name);

//配置信息都是kv模型的
//读取字符串类型的配置项
const char* get_string(const char* key);
//读取整数类型的配置项
//端口号啥的，且一般的可以带默认值
int get_int_default(const char* key,const int def);
#endif  //__CONFIG_H__ 
