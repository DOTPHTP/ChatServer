# ChatServer
by ShunQi Fan

***
&nbsp;  这个是个人用于学习多线程，reactor模式，以及C++在Linux中的网络编程所做的一个服务端小项目。

&nbsp;  目的功能为用户登录系统后能够在聊天室中发送消息，该消息会被广播至所有登录了系统的用户。用户信息初始存放在数据库中，密码会进行hash处理，不存放明文数据（因此无法找回密码）。暂时不处理客户端，有时间可能会写，使用时使用py这种语言模拟客户端即可。