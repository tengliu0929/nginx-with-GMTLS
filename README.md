# nginx-with-GMTLS
基于开源项目铜锁的国密nginx改造


一、项目基于开源项目铜锁，相关信息如下：  
铜锁项目地址： https://github.com/Tongsuo-Project/Tongsuo  
铜锁项目相关文档地址： https://www.yuque.com/tsdoc  
二、国密功能新增功能说明
   
   2.1  http模块
        
		Syntax: gmssl on | off;
		Default: off
		Context: main, http 
		
	    Syntax: ssl_sign_certificate file;
		Default: --
		Context: main, http 
		
	    Syntax: ssl_sign_certificate_key file;
		Default: --
		Context: main, http 
		
		
	    Syntax: ssl_enc_certificate  file;
		Default: --
		Context: main, http 
		
		
	    Syntax: ssl_enc_certificate_key file;
		Default:--
		Context: main, http 
   2.2 http proxy 模块
       Syntax: proxy_gmssl_enable on | off;
	   Default: -- 
	   Context: main, http, location
	   
	   Syntax: proxy_gmssl_sign_certificate file;
	   Default: --
	   Context: main, http, location
	   
	   Syntax: proxy_gmssl_sign_key file;
	   Default: --
	   Context: main, http, location
	   
	   Syntax: proxy_gmssl_enc_certificate file;
	   Default: --
	   Context: main, http, location
	   
	   Syntax: proxy_gmssl_enc_key file;
	   Default: --
	   Context: main, http, location
	   
	2.3  stream模块
	   Syntax: gmssl_enable on | off;
	   Default: off
	   Context: stream, server
	   
	   
	   Syntax: gmssl_sign_certificate file;
	   Default: --
	   Context: stream, server
	   
	   Syntax: gmssl_sign_key file;
	   Default: --
	   Context: stream, server
	   
	   Syntax: gmssl_enc_certificate file;
	   Default: --
	   Context: stream, server
	   
	   Syntax: gmssl_enc_key file;
	   Default: --
	   Context: stream, server
	   
	2.4 stream proxy模块
       Syntax: proxy_gmssl_enable on | off;
       Default: off
       Context:	stream, server 

       Syntax: proxy_sign_certificate file;
       Default: --
       Context: stream, server 	   
	   
	   Syntax: proxy_sign_key file;
       Default: --
       Context: stream, server 
	   
	   Syntax: proxy_enc_certificate file;
       Default: --
       Context: stream, server 
	   
	   Syntax: proxy_enc_key file;
       Default: --
       Context: stream, server 
	   
三、相关国密标准  
《GMT_0024-2014》 《GB∕T 38636-2020 信息安全技术 传输层密码协议（TLCP）》  
文档相关地址： https://github.com/guanzhi/GM-Standards  
四、编译  
需要提前下载铜锁开源项目源码，然后进入nginx的目录,然后根据所需来配置编译选项，以下只是一个示例  
./configure --with-debug --prefix=/opt/nginx --with-stream --with-stream_ssl_module --with-http_ssl_module --with-openssl=${OPENSSL_SRC} --with-openssl-opt="enable-ntls"  
五、测试  
主代码中的client.c和server.c可以用来测试铜锁开源项目的国密功能，至于证书，请参考铜锁说明文档，地址如下：  
https://www.yuque.com/tsdoc/ts/sulazb  
六、鸣谢  
多谢https://github.com/wa5i 的答疑解惑  
七、如果您有疑问哪一块有问题，请及时联系我，联系方式如下  
liuteng09029@gmail.com 或 liuteng0929@163.com
