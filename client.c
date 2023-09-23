#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define CA_FILE "certs/chain-ca.crt"
#define SIGN_CERT "certs/client_sign.crt"
#define SIGN_KEY "certs/client_sign.key"
#define ENC_CERT "certs/client_enc.crt"
#define ENC_KEY "certs/client_enc.key"




/*
  Blocking socket connect
*/
int build_connection( const char *ip, int port)
{
	int fd = 0;
	int ret = 0;
	struct sockaddr_in addr;
	//socklen_t len = 0;
	fd = socket(AF_INET,SOCK_STREAM,0);
	if(fd <= 0 ) goto END;
    
    memset(&addr,0,sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
	
	ret = connect(fd,(struct sockaddr *)&addr, sizeof(struct sockaddr));
	if(ret != 0 )
		goto END;

    END:
	  if(ret != 0 )
	  {
		  close(fd);
		  fd = -1;
	  }
	
	return fd;
} 


// openssl 1.1.1
int main()
{
	SSL *ssl = NULL;
	SSL_CTX *ssl_ctx = NULL;
	int fd = 0;
	const char *ip = "127.0.0.1";
	int port = 9443;
	int ret = 0;
	char *message = "GET / HTTP/1.1\r\nUser-Agent: curl/7.29.0\r\nHost: localhost\r\nAccept: */*\r\n\r\n";
	char rcvbuf[10240] = {0};
	int code = 0;
	//init openssl
   
   fd = build_connection(ip,port);
   if(fd <= 0 )
   {
	   printf("build_connection\n");
	   ret = -1;
	   goto END;
   }
	   
	   
   
   #if OPENSSL_VERSION_NUMBER >= 0x10100003L
    if(OPENSSL_init_ssl(OPENSSL_INIT_LOAD_CONFIG,NULL) == 0 ) return 0;
	ERR_clear_error();
   #else
	   OPENSSL_config(NULL);
       SSL_library_init();
	   SSL_load_error_strings();
	   OpenSSL_add_all_algorithms();
   #endif
   
	
	ssl_ctx = SSL_CTX_new(NTLS_client_method());
	if(NULL == ssl_ctx ) 
	{
		ret = -1;
		printf("SSL_CTX_new\n");
		ERR_print_errors_fp(stderr);
		goto END;
	}
	
	
	SSL_CTX_enable_ntls(ssl_ctx);
	
	ret = SSL_CTX_set_cipher_list(ssl_ctx,"ECC-SM2-WITH-SM4-SM3");
	if(ret != 1 )
	{
		ret = -1;
		printf("SSL_CTX_set_cipher_list\n");
		ERR_print_errors_fp(stderr);
		goto END;
	}
	
	ret = SSL_CTX_load_verify_locations(ssl_ctx,CA_FILE,NULL);
	if(ret != 1 )
	{
		ret = -1;
		printf("SSL_CTX_load_verify_locations\n");
		ERR_print_errors_fp(stderr);
		goto END;
	}
	ret = SSL_CTX_use_certificate_file(ssl_ctx,SIGN_CERT,SSL_FILETYPE_PEM);
	if(ret != 1 )
	{
		ret = -1;
		printf("SSL_CTX_use_certificate_file\n");
		ERR_print_errors_fp(stderr);
		goto END;
	}
	ret = SSL_CTX_use_PrivateKey_file(ssl_ctx,SIGN_KEY,SSL_FILETYPE_PEM);
	if(ret != 1 )
	{
		ret = -1;
		printf("SSL_CTX_check_private_key\n");
		ERR_print_errors_fp(stderr);
		goto END;
	}
	ret = SSL_CTX_use_certificate_file(ssl_ctx,ENC_CERT,SSL_FILETYPE_PEM);
	if(ret != 1 )
	{
		ret = -1;
		printf("SSL_CTX_use_certificate_file\n");
		ERR_print_errors_fp(stderr);
		goto END;
	}
	ret = SSL_CTX_use_PrivateKey_file(ssl_ctx,ENC_KEY,SSL_FILETYPE_PEM);
	if(ret != 1)
	{
		ret = -1;
		printf("SSL_CTX_use_PrivateKey_file\n");
		ERR_print_errors_fp(stderr);
		goto END;
	}
	ret = SSL_CTX_check_private_key(ssl_ctx);
	if(ret != 1)
	{
		ret = -1;
		printf("SSL_CTX_check_private_key\n");
		ERR_print_errors_fp(stderr);
		goto END;
	}
	
	ssl = SSL_new(ssl_ctx);
	if(NULL == ssl )
	{
		printf("SSL_new\n");
		ERR_print_errors_fp(stderr);
		ret = -1;
		goto END;
	}
	
	
	SSL_set_fd(ssl,fd);
	
	while(1)
	{
		ret = SSL_connect(ssl);
	    if(ret == 0 )
		{
			code = SSL_get_error(ssl,ret);
			printf("ret is 0,code --> %d\n",code);
		}
		else if(ret < 0 )
		{
			code = SSL_get_error(ssl,ret);
			printf("ret is < 0, code -->%d\n",code);
		}
		else if(ret == 1)
			break;
	}
	
	ret = SSL_write(ssl,message,strlen(message));
	if(ret <= 0 )
	{
		ret = -1;
		printf("SSL_write\n");
		ERR_print_errors_fp(stderr);
		goto END;
	}
	ret = SSL_read(ssl,rcvbuf,10240);
	if(ret <= 0 )
	{
		ret = -1;
		printf("SSL_read\n");
		ERR_print_errors_fp(stderr);
		goto END;
	}
    printf("rcvbuf --> %s\n",rcvbuf);
	
	
	
	END:
	  if(ssl) SSL_free(ssl);
	  if(ssl_ctx) SSL_CTX_free(ssl_ctx);
	  if(fd) close(fd);
	  
	return 0;
}
