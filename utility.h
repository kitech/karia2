#ifndef UTILITY_H
#define UTILITY_H

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

class Utility
{
public:
	Utility(void);

	static bool isNumericIPv4Addr(const char * addr );

	static const char * getSaveFileNameByUrl( const char * url );

	
public:
	~Utility(void);
};


/////////////////////////////////////
//从PHP源代码中分离出来的URL解析函数
// from url.h
/////////////////////////////////////
#ifdef __cplusplus
extern "C"{
#endif

	typedef struct php_url {
		char *scheme;
		char *user;
		char *pass;
		char *host;
		unsigned short port;
		char *path;
		char *query;
		char *fragment;
	} php_url;




	void php_url_free(php_url *theurl);
	php_url *php_url_parse(char const *str);

	void test_func_php_url_pares(const char * url = 0 );

	void dump_php_url(php_url * url);

#ifdef __cplusplus
};
#endif

class URL
{
public:
	URL(const char * pUrl = 0 );
	~URL( );
	bool Parse( const char * pUrl = 0 ) ;
	char * Encode( const char * pUrl = 0 );
	char * Decode( const char * pUrl = 0 );
	char * RawEncode( const char * pUrl = 0 );
	char * RawDecode( const char * pUrl = 0 );	
	const char * GetScheme();
	const char * GetUser();
	const char * GetPass();
	const char * GetHost();
	unsigned int GetPort();
	const char * GetPath();
	const char * GetFullPath();
	const char * GetQuery();
	const char * GetFragment();

	const char * GetBaseFileName(char * pFileName);

	bool SetScheme(const char *pScheme);
	bool SetUser(const char *pUser);
	bool SetPass(const char *pPass);
	bool SetHost(const char *pHost);
	bool SetPort(unsigned int pPort);
	bool SetPath(const char *pPath);
	bool SetQuery(const char *pQuery);
	bool SetFragment(const char * pFragment);

	void Dump();
private:
	void Free();

	/* Used by main.c: detect URLs written using the "shorthand" URL forms
	popularized by Netscape and NcFTP.  HTTP shorthands look like this:

	www.foo.com[:port]/dir/file   -> http://www.foo.com[:port]/dir/file
	www.foo.com[:port]            -> http://www.foo.com[:port]

	FTP shorthands look like this:

	foo.bar.com:dir/file          -> ftp://foo.bar.com/dir/file
	foo.bar.com:/absdir/file      -> ftp://foo.bar.com//absdir/file

	If the URL needs not or cannot be rewritten, return NULL.  */	
	bool RewriteShortHandUrl();

	int FullPathLength();
	int FullPathRewrite( char * where );


private:

	char *scheme;
	char *user;
	char *pass;
	char *host;
	unsigned short port;
	char *path;
	char *query;
	char *fragment;

	char * raw_url ;

	char * full_path ;

};

#ifdef __cplusplus
extern "C"{
#endif


/////////////////////////////////////
//从mwget源代码中分离出来的一些工具函数
// from utils.h
/////////////////////////////////////

//Add thousand separators to a number already in string form
char * add_thousand_seps (const char *repr);

char *suffix (const char *s);
int match_tail (const char *, const char *, int);
int has_wildcards_p (const char *);

int has_html_suffix_p (const char *);

/* How many bytes it will take to store LEN bytes in base64.  */
#define BASE64_LENGTH(len) (4 * (((len) + 2) / 3))

int base64_encode (const char *, int, char *);
int base64_decode (const char *, char *);

//the same as std::stable_sort
//void stable_sort (void *, size_t, size_t,
//                          int (*) (const void *, const void *));

#ifdef __cplusplus
};
#endif


#endif


