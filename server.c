#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <openssl/evp.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*
#define CA_FILE  "/opt/certs/CA.cert.pem"
#define SIGN_CERT  "/opt/certs/SS.cert.pem"
#define SIGN_FILE "/opt/certs/SS.key.pem"
#define ENC_CERT  "/opt/certs/SE.cert.pem"
#define ENC_FILE  "/opt/certs/SE.key.pem"
*/


#define CA_FILE  "/mnt/hgfs/code/test/src/tongsuo/certs/chain-ca.crt"
#define SIGN_CERT  "/mnt/hgfs/code/test/src/tongsuo/certs/server_sign.crt"
#define SIGN_FILE "/mnt/hgfs/code/test/src/tongsuo/certs/server_sign.key"
#define ENC_CERT  "/mnt/hgfs/code/test/src/tongsuo/certs/server_enc.crt"
#define ENC_FILE  "/mnt/hgfs/code/test/src/tongsuo/certs/server_enc.key"



#define FILE_PATH "/opt/nginx/html/index.html"

int build_server_sock( const char *ip , int port)
{
	 int fd = 0;
	 int ret = 0;
	 struct sockaddr_in addr;
	 
	 memset(&addr,0,sizeof(struct sockaddr_in));
	 
	 addr.sin_family = AF_INET;
	 addr.sin_addr.s_addr = inet_addr(ip);
	 addr.sin_port = htons(port);
	 
	 fd = socket(AF_INET,SOCK_STREAM,0);
	 if(fd <= 0 )
	 {
		 ret = -1;
		 goto END;
	 }
	 
	 ret = bind(fd,(const struct sockaddr *)&addr,(socklen_t)sizeof(struct sockaddr_in));
	 if(ret != 0 )
	 {
		 
		 goto END;
	 }
	 
	 ret = listen(fd,1024);
	 if(ret != 0 )
	 {
		 //ret = -1;
		 goto END;
	 }
	 
	 END:
	  if(ret != 0 )
	  {
		  if(fd) 
		  {
			  close(fd);
			  fd = -1;
		  }
		  
	  }
	  
	  
	  
	 return fd;
}

int main()
{
	
	const char *ip = "127.0.0.1";
	int port = 443;
	int fd = 0;
	int ret = 0;
	int newfd = 0 ;
	SSL_CTX *ssl_ctx = NULL;
	SSL *ssl = NULL;
	struct sockaddr_in accept_addr;
	char *message = NULL;
	char buf[1024] = {0};
	int code = 0;
	FILE *fp = NULL;
	int len = 0;
	
	socklen_t addr_len;
	memset(&accept_addr,0,sizeof(accept_addr));
	
	
	
	fp = fopen(FILE_PATH,"r");
	if(NULL == fp )
	{
		return -1;
	}
	
	
	fseek(fp,0,SEEK_END);
	len = ftell(fp);
	rewind(fp);
	
	message = (char *)malloc(len+1);
	if(message == NULL )
	{
		fclose(fp);
		return -1;
	}
	memset(message,0,len+1);
	
	fread(message,1,len,fp);
    fclose(fp);	
	
	fd = build_server_sock(ip,port);
	if(fd <= 0 )
	{
		ret = -1;
		goto END;
	}
	
	//openssl init
	
	#if OPENSSL_VERSION_NUMBER >= 0x10100003L
	 if(OPENSSL_init_ssl(OPENSSL_INIT_LOAD_CONFIG,NULL) == 0) return 0;
	 
	 ERR_clear_error();
	#else
	 OPENSSL_config(NULL);
	 SSL_library_init();
	 SSL_load_error_strings();
	 OpenSSL_add_all_algorithms();
	#endif
	
	ssl_ctx = SSL_CTX_new(NTLS_server_method());
	//ssl_ctx = SSL_CTX_new(SSLv23_method());
	if(NULL == ssl_ctx)
    {
		ret = -1;
		goto END;
	}		
	 SSL_CTX_enable_ntls(ssl_ctx);
	//SSL_CTX_set_cipher_list();
	SSL_CTX_set_verify(ssl_ctx,SSL_VERIFY_NONE,NULL);
	
	ret = SSL_CTX_load_verify_locations(ssl_ctx,CA_FILE,NULL);
	if(ret != 1 )
	{
		printf("SSL_CTX_load_verify_locations\n");
		ERR_print_errors_fp(stderr);
		ret = -1;
		goto END;
	}
	
	ret = SSL_CTX_use_sign_certificate_file(ssl_ctx, SIGN_CERT,SSL_FILETYPE_PEM);
	if(ret != 1 )
	{
		printf("SSL_CTX_use_certificate_file\n");
		ERR_print_errors_fp(stderr);
		ret = -1;
		goto END;
	}
	
	ret = SSL_CTX_use_sign_PrivateKey_file(ssl_ctx,SIGN_FILE,SSL_FILETYPE_PEM);
	if(ret != 1 )
	{
		printf("SSL_CTX_use_PrivateKey_file\n");
		ERR_print_errors_fp(stderr);
		ret = -1;
		goto END;
	}
	
	ret = SSL_CTX_use_enc_certificate_file(ssl_ctx, ENC_CERT,SSL_FILETYPE_PEM);
	if(ret != 1)
	{
		printf("SSL_CTX_use_certificate_file\n");
		ERR_print_errors_fp(stderr);
		ret = -1;
		goto END;
	}
	
	ret = SSL_CTX_use_enc_PrivateKey_file(ssl_ctx,ENC_FILE,SSL_FILETYPE_PEM);
	if(ret != 1)
	{
		printf("SSL_CTX_use_PrivateKey_file\n");
		ERR_print_errors_fp(stderr);
		ret = -1;
		goto END;
	}
	
	ret = SSL_CTX_check_private_key(ssl_ctx);
	if(ret != 1)
	{
		printf("SSL_CTX_check_private_key\n");
		ERR_print_errors_fp(stderr);
		ret = -1;
		goto END;
	}
		
	
	while(1)
	{
		newfd  = accept(fd,(struct sockaddr *)&accept_addr,(socklen_t *)&addr_len);
		if(newfd <= 0 )
		{
			perror("accept wrong\n");
			ret = -1;
			goto END;
		} 
	    ssl =  SSL_new(ssl_ctx);
		if(NULL == ssl )
		{
			ret = -1;
			goto END;
		}
	
		SSL_set_fd(ssl,newfd);
		
		//SSL_set_accept_state(ssl);
		while(1)
		{
			
			ret = SSL_accept(ssl);
		    if(ret == 0 )
			{
				code = SSL_get_error(ssl,ret);
				printf("ret is 0, the code is %d\n",code);
				 ERR_print_errors_fp(stderr);
				 perror("errno");
			}
			else if(ret < 0 )
			{
				code = SSL_get_error(ssl,ret);
				printf("ret is < 0 , the code is %d\n",code);
				 ERR_print_errors_fp(stderr);
				 perror("errno");
			}
			else if(ret == 1)
			break;
			/*
			ret = SSL_do_handshake(ssl);
			if(ret <= 0 )
			{
			   code = SSL_get_error(ssl,ret);
			   printf("The code is %d\n",code);
			   ERR_print_errors_fp(stderr);
			}
			else if(ret == 1 )
				break;
			*/
		}
		
		SSL_read(ssl,buf,1024);
		printf("buf --> %s\n",buf);
		ret = SSL_write(ssl,message,len);
		
		if(ssl) 
		{
			SSL_free(ssl);
			ssl = NULL;
		}
	}
	
	
	
    END:
	  if(message) free(message);
	  if(ssl_ctx) SSL_CTX_free(ssl_ctx);
	  if(ssl) SSL_free(ssl);
	  if(newfd) close(newfd);
	  if(fd) close(fd);
	
	return ret;
}
