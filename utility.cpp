#include <cassert>

#include "utility.h"

Utility::Utility(void)
{
}

Utility::~Utility(void)
{
}

//000.000.000.000
//static 
bool Utility::isNumericIPv4Addr( const char *  addr)
{

	int field = 0 ;
	int len = strlen( addr ) ;
	const char * fieldStartPtr = addr ;

	if( len > 15 ) return false ;	// 
	for( int pos = 0 ; pos < len ; pos ++ )
	{
		char c = addr[pos] ;
		if( c != '.' && (! ( c >= '0' && c <= '9' ) ) )
		{
			return false ;
		}
		if( c == '.' )
		{
			if( ( ( addr + pos) - fieldStartPtr ) > 3 )
			{
				return false ;
			}
			if( pos + 1 >= len )	// . 后面是空的。
			{
				return false ;
			}
			fieldStartPtr = addr+pos + 1 ;
			field ++ ;
		}		
	}

	if( field != 3 )
	{
		return false ;
	}
	else
	{
		return true ;
	}
	return false ;
}

//static 
const char *  Utility::getSaveFileNameByUrl( const char *  url )
{
	char saveFileName[256] = {0}  ;

	URL * ou = new URL(url) ;
	bool bret = ou->Parse();

	const char * temp = ou->GetBaseFileName((char*)ou->GetQuery());

	strcpy(saveFileName,temp);

	delete ou ; ou = 0 ;

	return saveFileName ;

}


/////////////////////////////////////////////
//
////////////////////////////////////////////


#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <stdio.h>


//#include "url.h"


#define ecalloc calloc
#ifdef _WIN32
#define estrndup(x,y) _strdup(x); assert( y == strlen(x) ); 
#define strncasecmp( x , y , z ) _strnicmp ( x , y , z )
#define strcasecmp( x , y  ) _stricmp ( x , y )
#elif defined(__unix__)
#define estrndup(x,y) strdup(x); assert( y == strlen(x) ); 
#elif defined(__DARWIN__)
#define estrndup(x,y) strdup(x); assert(y == strlen(x));
#elif !defined(strndup)
#define estrndup(x,y) strdup(x); assert(y == strlen(x));
#else
#define estrndup strndup
#endif

#define efree free
#define STR_FREE(x) \
	free(x);	\
	x = 0 ;
#define safe_emalloc(nmemb, size, offset)		malloc((nmemb) * (size) + (offset))

static unsigned char hexchars[] = "0123456789ABCDEF";

//internal function by this model
php_url *php_url_parse_ex(char const *str, int length);
char *php_replace_controlchars_ex(char *str, int len);

static int php_htoi(char *s)
{
	int value;
	int c;

	c = ((unsigned char *)s)[0];
	if (isupper(c))
		c = tolower(c);
	value = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16;

	c = ((unsigned char *)s)[1];
	if (isupper(c))
		c = tolower(c);
	value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;

	return (value);
}


int php_raw_url_decode(char *str, int len)
{
	char *dest = str;
	char *data = str;

	while (len--) {
		if (*data == '%' && len >= 2 && isxdigit((int) *(data + 1)) 
			&& isxdigit((int) *(data + 2))) {
#ifndef CHARSET_EBCDIC
				*dest = (char) php_htoi(data + 1);
#else
				*dest = os_toebcdic[(char) php_htoi(data + 1)];
#endif
				data += 2;
				len -= 2;
		} else {
			*dest = *data;
		}
		data++;
		dest++;
	}
	*dest = '\0';
	return dest - str;
}

char *php_raw_url_encode(char const *s, int len, int *new_length)
{
	register int x, y;
	unsigned char *str;

	str = (unsigned char *) safe_emalloc(3, len, 1);
	for (x = 0, y = 0; len--; x++, y++) 
	{
		str[y] = (unsigned char) s[x];
#ifndef CHARSET_EBCDIC
		if ((str[y] < '0' && str[y] != '-' && str[y] != '.') ||
			(str[y] < 'A' && str[y] > '9') ||
			(str[y] > 'Z' && str[y] < 'a' && str[y] != '_') ||
			(str[y] > 'z')) {
				str[y++] = '%';
				str[y++] = hexchars[(unsigned char) s[x] >> 4];
				str[y] = hexchars[(unsigned char) s[x] & 15];
#else /*CHARSET_EBCDIC*/
		if (!isalnum(str[y]) && strchr("_-.", str[y]) != NULL) {
			str[y++] = '%';
			str[y++] = hexchars[os_toascii[(unsigned char) s[x]] >> 4];
			str[y] = hexchars[os_toascii[(unsigned char) s[x]] & 15];
#endif /*CHARSET_EBCDIC*/
		}
	}
	str[y] = '\0';
	if (new_length) {
		*new_length = y;
	}
	return ((char *) str);
}

char *php_url_encode(char const *s, int len, int *new_length)
{
	register unsigned char c;
	unsigned char *to, *start;
	unsigned char const *from, *end;

	from = (unsigned char *) s;
	end = (unsigned char *) (s + len);
	start = to = (unsigned char *) safe_emalloc(3, len, 1);

	while (from < end) {
		c = *from++;

		if (c == ' ') {
			*to++ = '+';
#ifndef CHARSET_EBCDIC
		} else if ((c < '0' && c != '-' && c != '.') ||
			(c < 'A' && c > '9') ||
			(c > 'Z' && c < 'a' && c != '_') ||
			(c > 'z')) {
				to[0] = '%';
				to[1] = hexchars[c >> 4];
				to[2] = hexchars[c & 15];
				to += 3;
#else /*CHARSET_EBCDIC*/
		} else if (!isalnum(c) && strchr("_-.", c) == NULL) {
			/* Allow only alphanumeric chars and '_', '-', '.'; escape the rest */
			to[0] = '%';
			to[1] = hexchars[os_toascii[c] >> 4];
			to[2] = hexchars[os_toascii[c] & 15];
			to += 3;
#endif /*CHARSET_EBCDIC*/
		} else {
			*to++ = c;
		}
	}
	*to = 0;
	if (new_length) {
		*new_length = to - start;
	}
	return (char *) start;
}

int php_url_decode(char *str, int len)
{
	char *dest = str;
	char *data = str;

	while (len--) {
		if (*data == '+') {
			*dest = ' ';
		}
		else if (*data == '%' && len >= 2 && isxdigit((int) *(data + 1)) 
			&& isxdigit((int) *(data + 2))) {
#ifndef CHARSET_EBCDIC
				*dest = (char) php_htoi(data + 1);
#else
				*dest = os_toebcdic[(char) php_htoi(data + 1)];
#endif
				data += 2;
				len -= 2;
		} else {
			*dest = *data;
		}
		data++;
		dest++;
	}
	*dest = '\0';
	return dest - str;
}

/////////////////
void php_url_free(php_url *theurl)
{
	if (theurl->scheme)
		efree(theurl->scheme);
	if (theurl->user)
		efree(theurl->user);
	if (theurl->pass)
		efree(theurl->pass);
	if (theurl->host)
		efree(theurl->host);
	if (theurl->path)
		efree(theurl->path);
	if (theurl->query)
		efree(theurl->query);
	if (theurl->fragment)
		efree(theurl->fragment);
	efree(theurl);
}
php_url *php_url_parse(char const *str)
{
	return php_url_parse_ex(str, strlen(str));
}
php_url *php_url_parse_ex(char const *str, int length)
{
	char port_buf[6];
	php_url *ret = (php_url *) ecalloc (1, sizeof(php_url));
	char const *s, *e, *p, *pp, *ue;

	s = str;
	ue = s + length;

	/* parse scheme */
	if ((e = (char*)memchr(s, ':', length)) && (e - s)) {
		/* 
		* certain schemas like mailto: and zlib: may not have any / after them
		* this check ensures we support those.
		*/
		if (*(e+1) != '/') {
			/* check if the data we get is a port this allows us to 
			* correctly parse things like a.com:80
			*/
			p = e + 1;
			while (isdigit(*p)) {
				p++;
			}

			if ((*p) == '\0' || *p == '/') {
				goto parse_port;
			}

			ret->scheme = (char*) estrndup(s, (e-s));
			php_replace_controlchars_ex(ret->scheme, (e - s));

			length -= ++e - s;
			s = e;
			goto just_path;
		} else {
			ret->scheme = estrndup(s, (e-s));
			php_replace_controlchars_ex(ret->scheme, (e - s));

			if (*(e+2) == '/') {
				s = e + 3;
				if (!strncasecmp("file", ret->scheme, sizeof("file"))) {
					if (*(e + 3) == '/') {
						/* support windows drive letters as in:
						file:///c:/somedir/file.txt
						*/
						if (*(e + 5) == ':') {
							s = e + 4;
						}
						goto nohost;
					}
				}
			} else {
				if (!strncasecmp("file", ret->scheme, sizeof("file"))) {
					s = e + 1;
					goto nohost;
				} else {
					length -= ++e - s;
					s = e;
					goto just_path;
				}	
			}
		}	
	} else if (e) { /* no scheme, look for port */
parse_port:
		p = e + 1;
		pp = p;

		while (pp-p < 6 && isdigit(*pp)) {
			pp++;
		}

		if (pp-p < 6 && (*pp == '/' || *pp == '\0')) {
			memcpy(port_buf, p, (pp-p));
			port_buf[pp-p] = '\0';
			ret->port = atoi(port_buf);
		} else {
			goto just_path;
		}
	} else {
just_path:
		ue = s + length;
		goto nohost;
	}

	e = ue;

	if (!(p = (char*)memchr(s, '/', (ue - s)))) {
		if ((p = (char*)memchr(s, '?', (ue - s)))) {
			e = p;
		} else if ((p = (char*)memchr(s, '#', (ue - s)))) {
			e = p;
		}
	} else {
		e = p;
	}	

	/* check for login and password */
	if ((p = (char*)memchr(s, '@', (e-s)))) {
		if ((pp = (char*)memchr(s, ':', (p-s)))) {
			if ((pp-s) > 0) {
				ret->user = estrndup(s, (pp-s));
				php_replace_controlchars_ex(ret->user, (pp - s));
			}	

			pp++;
			if (p-pp > 0) {
				ret->pass = estrndup(pp, (p-pp));
				php_replace_controlchars_ex(ret->pass, (p-pp));
			}	
		} else {
			ret->user = estrndup(s, (p-s));
			php_replace_controlchars_ex(ret->user, (p-s));
		}

		s = p + 1;
	}

	/* check for port */
	if (*s == '[' && *(e-1) == ']') {
		/* Short circuit portscan, 
		we're dealing with an 
		IPv6 embedded address */
		p = s;
	} else {
		/* memrchr is a GNU specific extension
		Emulate for wide compatability */
		for(p = e; *p != ':' && p >= s; p--);
	}

	if (p >= s && *p == ':') {
		if (!ret->port) {
			p++;
			if (e-p > 5) { /* port cannot be longer then 5 characters */
				STR_FREE(ret->scheme);
				STR_FREE(ret->user);
				STR_FREE(ret->pass);
				efree(ret);
				return NULL;
			} else if (e - p > 0) {
				memcpy(port_buf, p, (e-p));
				port_buf[e-p] = '\0';
				ret->port = atoi(port_buf);
			}
			p--;
		}	
	} else {
		p = e;
	}

	/* check if we have a valid host, if we don't reject the string as url */
	if ((p-s) < 1) {
		STR_FREE(ret->scheme);
		STR_FREE(ret->user);
		STR_FREE(ret->pass);
		efree(ret);
		return NULL;
	}

	ret->host = estrndup(s, (p-s));
	php_replace_controlchars_ex(ret->host, (p - s));

	if (e == ue) {
		return ret;
	}

	s = e;

nohost:

	if ((p = (char*)memchr(s, '?', (ue - s)))) {
		pp = strchr(s, '#');

		if (pp && pp < p) {
			p = pp;
			pp = strchr(pp+2, '#');
		}

		if (p - s) {
			ret->path = estrndup(s, (p-s));
			php_replace_controlchars_ex(ret->path, (p - s));
		}	

		if (pp) {
			if (pp - ++p) { 
				ret->query = estrndup(p, (pp-p));
				php_replace_controlchars_ex(ret->query, (pp - p));
			}
			p = pp;
			goto label_parse;
		} else if (++p - ue) {
			ret->query = estrndup(p, (ue-p));
			php_replace_controlchars_ex(ret->query, (ue - p));
		}
	} 
	else if ((p = (char*)memchr(s, '#', (ue - s)))) {
		if (p - s) {
			ret->path = estrndup(s, (p-s));
			php_replace_controlchars_ex(ret->path, (p - s));
		}	

label_parse:
		p++;

		if (ue - p) {
			ret->fragment = estrndup(p, (ue-p));
			php_replace_controlchars_ex(ret->fragment, (ue - p));
		}	
	} else {
		ret->path = estrndup(s, (ue-s));
		php_replace_controlchars_ex(ret->path, (ue - s));
	}

	return ret;
}


char *php_replace_controlchars_ex(char *str, int len)
{
	unsigned char *s = (unsigned char *)str;
	unsigned char *e = (unsigned char *)str + len;

	if (!str) {
		return (NULL);
	}

	while (s < e) {

		if (iscntrl(*s)) {
			*s='_';
		}	
		s++;
	}

	return (str);
} 


void test_func_php_url_pares(const char * url )
{
	php_url * surl ;
	char turl[255] = {0};
	if( url == 0 )
	{
		strcpy(turl,"http://me:124@localhost/hehe/sdfsdf.ph?sdfsdf=sdfiu#fra");
	}
	else
	{
		strcpy(turl,url);
	}
	surl = php_url_parse(turl);
	//dump_php_url(surl);

	URL u (turl);
	u.Parse();
	u.Dump();

}


void dump_php_url(php_url * theurl)
{
	if (theurl->scheme)
		fprintf(stderr,"scheme: %s\t",theurl->scheme);
	if (theurl->user)
		fprintf(stderr,"user: %s\t",theurl->user);
	if (theurl->pass)
		fprintf(stderr,"pass: %s\t",theurl->pass);
	if (theurl->host)
		fprintf(stderr,"host: %s\t",theurl->host);
	fprintf(stderr,"port: %d\t",theurl->port);
	if (theurl->path)
		fprintf(stderr,"path: %s\t",theurl->path);
	if (theurl->query)
		fprintf(stderr,"query: %s\t",theurl->query);
	if (theurl->fragment)
		fprintf(stderr,"fragment: %s\t",theurl->fragment);

	fprintf(stderr,"\n");
}

URL::URL(const char * pUrl  )
{
	if(pUrl != 0 )
	{
		raw_url = (char*)malloc(255);
		strncpy(raw_url,pUrl,254);
	}
	this->port = 0 ;

	this->scheme=0;
	this->user=0;
	this->pass=0;
	this->host=0;

	this->path=0;
	this->query=0;
	this->fragment=0;

	this->full_path = 0 ;

}
URL::~URL( )
{
	this->Free();
}

bool URL::Parse( const char * pUrl ) 
{
	if(pUrl != 0 )
	{
		raw_url = (char*)malloc(255);
		strncpy(raw_url,pUrl,254);
	}
	char port_buf[6];
	//php_url *ret = (php_url *) ecalloc (1, sizeof(php_url));
	//php_url *ret = (php_url *) this ;
	//#define ret this
	char const *s, *e, *p, *pp, *ue;
	char * str = 0  ;
	int length ; 

	this->RewriteShortHandUrl();
	//printf("RW : %s\n" , this->raw_url ) ;

	s = str = this->raw_url ;
	length = strlen(str) ;
	ue = s + length;

	/* parse scheme */
	if ((e = (char*)memchr(s, ':', length)) && (e - s)) {
		/* 
		* certain schemas like mailto: and zlib: may not have any / after them
		* this check ensures we support those.
		*/
		if (*(e+1) != '/') {
			/* check if the data we get is a port this allows us to 
			* correctly parse things like a.com:80
			*/
			p = e + 1;
			while (isdigit(*p)) {
				p++;
			}

			if ((*p) == '\0' || *p == '/') {
				goto parse_port;
			}

			this->scheme = (char*) estrndup(s, (e-s));
			php_replace_controlchars_ex(this->scheme, (e - s));

			length -= ++e - s;
			s = e;
			goto just_path;
		} else {
			this->scheme = estrndup(s, (e-s));
			php_replace_controlchars_ex(this->scheme, (e - s));

			if (*(e+2) == '/') {
				s = e + 3;
				if (!strncasecmp("file", this->scheme, sizeof("file"))) {
					if (*(e + 3) == '/') {
						/* support windows drive letters as in:
						file:///c:/somedir/file.txt
						*/
						if (*(e + 5) == ':') {
							s = e + 4;
						}
						goto nohost;
					}
				}
			} else {
				if (!strncasecmp("file", this->scheme, sizeof("file"))) {
					s = e + 1;
					goto nohost;
				} else {
					length -= ++e - s;
					s = e;
					goto just_path;
				}	
			}
		}	
	} else if (e) { /* no scheme, look for port */
parse_port:
		p = e + 1;
		pp = p;

		while (pp-p < 6 && isdigit(*pp)) {
			pp++;
		}

		if (pp-p < 6 && (*pp == '/' || *pp == '\0')) {
			memcpy(port_buf, p, (pp-p));
			port_buf[pp-p] = '\0';
			this->port = atoi(port_buf);
		} else {
			goto just_path;
		}
	} else {
just_path:
		ue = s + length;
		goto nohost;
	}

	e = ue;

	if (!(p = (char*)memchr(s, '/', (ue - s)))) {
		if ((p = (char*)memchr(s, '?', (ue - s)))) {
			e = p;
		} else if ((p = (char*)memchr(s, '#', (ue - s)))) {
			e = p;
		}
	} else {
		e = p;
	}	

	/* check for login and password */
	if ((p = (char*)memchr(s, '@', (e-s)))) {
		if ((pp = (char*)memchr(s, ':', (p-s)))) {
			if ((pp-s) > 0) {
				this->user = estrndup(s, (pp-s));
				php_replace_controlchars_ex(this->user, (pp - s));
			}	

			pp++;
			if (p-pp > 0) {
				this->pass = estrndup(pp, (p-pp));
				php_replace_controlchars_ex(this->pass, (p-pp));
			}	
		} else {
			this->user = estrndup(s, (p-s));
			php_replace_controlchars_ex(this->user, (p-s));
		}

		s = p + 1;
	}

	/* check for port */
	if (*s == '[' && *(e-1) == ']') {
		/* Short circuit portscan, 
		we're dealing with an 
		IPv6 embedded address */
		p = s;
	} else {
		/* memrchr is a GNU specific extension
		Emulate for wide compatability */
		for(p = e; *p != ':' && p >= s; p--);
	}

	if (p >= s && *p == ':') {
		if (!this->port) {
			p++;
			if (e-p > 5) { /* port cannot be longer then 5 characters */
				STR_FREE(this->scheme);
				STR_FREE(this->user);
				STR_FREE(this->pass);
				//efree(ret);
				return false ;
			} else if (e - p > 0) {
				memcpy(port_buf, p, (e-p));
				port_buf[e-p] = '\0';
				this->port = atoi(port_buf);
			}
			p--;
		}	
	} else {
		p = e;
	}

	/* check if we have a valid host, if we don't reject the string as url */
	if ((p-s) < 1) {
		STR_FREE(this->scheme);
		STR_FREE(this->user);
		STR_FREE(this->pass);
		//efree(ret);
		return false;
	}

	this->host = estrndup(s, (p-s));
	php_replace_controlchars_ex(this->host, (p - s));

	if (e == ue) {
		return true ;
	}

	s = e;

nohost:

	if ((p = (char*)memchr(s, '?', (ue - s)))) {
		pp = strchr(s, '#');

		if (pp && pp < p) {
			p = pp;
			pp = strchr(pp+2, '#');
		}

		if (p - s) {
			this->path = estrndup(s, (p-s));
			php_replace_controlchars_ex(this->path, (p - s));
		}	

		if (pp) {
			if (pp - ++p) { 
				this->query = estrndup(p, (pp-p));
				php_replace_controlchars_ex(this->query, (pp - p));
			}
			p = pp;
			goto label_parse;
		} else if (++p - ue) {
			this->query = estrndup(p, (ue-p));
			php_replace_controlchars_ex(this->query, (ue - p));
		}
	} 
	else if ((p = (char*)memchr(s, '#', (ue - s)))) {
		if (p - s) {
			this->path = estrndup(s, (p-s));
			php_replace_controlchars_ex(this->path, (p - s));
		}	

label_parse:
		p++;

		if (ue - p) {
			this->fragment = estrndup(p, (ue-p));
			php_replace_controlchars_ex(this->fragment, (ue - p));
		}	
	} else {
		this->path = estrndup(s, (ue-s));
		php_replace_controlchars_ex(this->path, (ue - s));
	}

	if( this->full_path == 0 )
	{
		this->full_path = (char*)malloc(255);	
	}
	memset(this->full_path , 0 , 255 );
	if( this->query == 0 )
	{
		strcpy(this->full_path , this->path );
	}
	else
	{
		sprintf(this->full_path , "%s?%s" , this->path , this->query );
	}

	return true ;

	return false;
}
char * URL::Encode( const char * pUrl )
{
	register int x, y;
	unsigned char *str;
	const char * s = pUrl;
	int len = strlen(s);
	int inew_length;
	int * new_length = & inew_length ;

	str = (unsigned char *) safe_emalloc(3, len, 1);
	for (x = 0, y = 0; len--; x++, y++) 
	{
		str[y] = (unsigned char) s[x];
#ifndef CHARSET_EBCDIC
		if ((str[y] < '0' && str[y] != '-' && str[y] != '.') ||
			(str[y] < 'A' && str[y] > '9') ||
			(str[y] > 'Z' && str[y] < 'a' && str[y] != '_') ||
			(str[y] > 'z')) {
				str[y++] = '%';
				str[y++] = hexchars[(unsigned char) s[x] >> 4];
				str[y] = hexchars[(unsigned char) s[x] & 15];
#else /*CHARSET_EBCDIC*/
		if (!isalnum(str[y]) && strchr("_-.", str[y]) != NULL) {
			str[y++] = '%';
			str[y++] = hexchars[os_toascii[(unsigned char) s[x]] >> 4];
			str[y] = hexchars[os_toascii[(unsigned char) s[x]] & 15];
#endif /*CHARSET_EBCDIC*/
		}
	}
	str[y] = '\0';
	if (new_length) {
		*new_length = y;
	}
	return ((char *) str);

	return 0;
}
char * URL::Decode( const char * pUrl  )
{
	//char *dest = str;
	char *dest = (char*)malloc(strlen(pUrl)+1);
	char *str = (char*) pUrl ;
	char *data = str;
	int len = strlen(data);

	while (len--) {
		if (*data == '+') {
			*dest = ' ';
		}
		else if (*data == '%' && len >= 2 && isxdigit((int) *(data + 1)) 
			&& isxdigit((int) *(data + 2))) {
#ifndef CHARSET_EBCDIC
				*dest = (char) php_htoi(data + 1);
#else
				*dest = os_toebcdic[(char) php_htoi(data + 1)];
#endif
				data += 2;
				len -= 2;
		} else {
			*dest = *data;
		}
		data++;
		dest++;
	}
	*dest = '\0';
	//return dest - str;
	return dest ;

	return 0;
}

char * URL::RawEncode( const char * pUrl  )
{
	register int x, y;
	int len = strlen(pUrl);

	unsigned char *str = (unsigned char *) safe_emalloc(3, len, 1); 
	char * s = (char*)malloc(strlen(pUrl)+1);

	int inew_length;
	int * new_length = &inew_length ;

	strcpy(s,pUrl);

	str = (unsigned char *) safe_emalloc(3, len, 1);
	for (x = 0, y = 0; len--; x++, y++) 
	{
		str[y] = (unsigned char) s[x];
#ifndef CHARSET_EBCDIC
		if ((str[y] < '0' && str[y] != '-' && str[y] != '.') ||
			(str[y] < 'A' && str[y] > '9') ||
			(str[y] > 'Z' && str[y] < 'a' && str[y] != '_') ||
			(str[y] > 'z')) {
				str[y++] = '%';
				str[y++] = hexchars[(unsigned char) s[x] >> 4];
				str[y] = hexchars[(unsigned char) s[x] & 15];
#else /*CHARSET_EBCDIC*/
		if (!isalnum(str[y]) && strchr("_-.", str[y]) != NULL) {
			str[y++] = '%';
			str[y++] = hexchars[os_toascii[(unsigned char) s[x]] >> 4];
			str[y] = hexchars[os_toascii[(unsigned char) s[x]] & 15];
#endif /*CHARSET_EBCDIC*/
		}
	}
	str[y] = '\0';
	if (new_length) {
		*new_length = y;
	}
	return ((char *) str);

	return 0;
}
char * URL::RawDecode( const char * pUrl )
{
	int len = strlen(pUrl);
	char * str = (char*)malloc(strlen(pUrl)+1);
	strcpy(str,pUrl);
	char *dest = str;
	char *data = str;

	while (len--) {
		if (*data == '%' && len >= 2 && isxdigit((int) *(data + 1)) 
			&& isxdigit((int) *(data + 2))) {
#ifndef CHARSET_EBCDIC
				*dest = (char) php_htoi(data + 1);
#else
				*dest = os_toebcdic[(char) php_htoi(data + 1)];
#endif
				data += 2;
				len -= 2;
		} else {
			*dest = *data;
		}
		data++;
		dest++;
	}
	*dest = '\0';
	//return dest - str;
	return dest;
	return 0 ;
}

const char * URL::GetScheme()
{
	return this->scheme;
	return 0;
}
const char * URL::GetUser()
{
	return this->user;
	return 0;
}
const char * URL::GetPass()
{
	return this->pass;
	return 0;
}
const char * URL::GetHost()
{
	return this->host ;
	return 0;
}
unsigned int URL::GetPort()
{
	if( this->port == 0 )
	{
		if( strcasecmp(this->scheme,"HTTP") == 0 )
			return 80;
		if( strcasecmp(this->scheme,"HTTPS") == 0 )
			return 443;
		if( strcasecmp(this->scheme,"FTP") == 0 )
			return 21;
	}
	return this->port ;
	return 0;
}
const char * URL::GetPath()
{
	if( this->path == 0 )
	{
		this->path = (char*) malloc(255) ;
		this->path[0] = '/';
		this->path[1] = '\0';
	}
	return this->path;
	return 0;
}

const char * URL::GetFullPath()
{
	if( this->path == 0 )
	{
		this->path = (char*) malloc(255) ;
		this->path[0] = '/';
		this->path[1] = '\0';
		return this->path; 
	}
	else
	{
		return this->full_path ;
	}
}
const char * URL::GetQuery()
{
	return this->query ;
	return 0;
}
const char * URL::GetFragment()
{
	return this->fragment ;
	return 0;
}

#define emalloc malloc
const char * URL::GetBaseFileName(char * pFileName)
{

	char *s ;
	int len;
	char tname [60] = {0} ;
	char tsuffix [10] = {0} ;
	int nsize = 0;
	char ch ;

	char url [ 255] = {0};
	strcpy(url , this->raw_url);

	len = strlen(url);

	while ( len > 0 )
	{
		if( this->raw_url[ len - 1 ] == '/' )
		{
			break;
		}
		else if( this->raw_url[ len - 1 ] == '?' )
		{
			memset(tname,0,sizeof(tname));
			nsize = -1 ;
		}
		else
		{
			tname[ nsize  ] = this->raw_url [ len -1 ] ; 
		}
		len -- ;
		nsize ++ ;
	}
	tname[nsize] = '\0';



	//strcpy(tname , &tname[nsize-1] );
	if(  strlen(tname)  == 0 )
	{
		return "index.html";
	}

	len = strlen(tname);

	for( nsize = 0 ; nsize < len/2 ; nsize ++)
	{
		ch = tname[nsize];
		tname[nsize] = tname[len-nsize-1] ;
		tname[len-nsize-1] = ch ;
	}

	//fprintf(stderr , "base name------%s\n" , tname);

	strcpy(pFileName,tname);


	return pFileName ;
	//return &this->raw_url[len];//tname ;

	return 0 ;
}

bool URL::SetScheme(const char *pScheme)
{
	strcpy(this->scheme,pScheme);
	return false;
}
bool URL::SetUser(const char *pUser)
{
	strcpy(this->user,pUser);
	return false;
}
bool URL::SetPass(const char *pPass)
{
	strcpy(this->pass,pPass);
	return false;
}
bool URL::SetHost(const char *pHost)
{
	strcpy(this->host,pHost);
	return false;
}
bool URL::SetPort(unsigned int pPort)
{
	this->port = pPort ;
	return false;
}
bool URL::SetPath(const char *pPath)
{
	strcpy(this->path,pPath);
	return false;
}
bool URL::SetQuery(const char *pQuery)
{
	strcpy(this->query,pQuery);
	return false;
}
bool URL::SetFragment(const char * pFragment)
{
	strcpy(this->fragment,pFragment);
	return false;
}


void URL::Dump()
{
	if (this->scheme)
		fprintf(stderr,"scheme: %s\t",this->scheme);
	if (this->user)
		fprintf(stderr,"user: %s\t",this->user);
	if (this->pass)
		fprintf(stderr,"pass: %s\t",this->pass);
	if (this->host)
		fprintf(stderr,"host: %s\t",this->host);
	fprintf(stderr,"port: %d\t",this->port);
	if (this->path)
		fprintf(stderr,"path: %s\t",this->path);
	if (this->query)
		fprintf(stderr,"query: %s\t",this->query);
	if (this->fragment)
		fprintf(stderr,"fragment: %s\t",this->fragment);

	fprintf(stderr,"\n");
}

void URL::Free()
{
	if (this->scheme)
		efree(this->scheme);
	if (this->user)
		efree(this->user);
	if (this->pass)
		efree(this->pass);
	if (this->host)
		efree(this->host);
	if (this->path)
		efree(this->path);
	if (this->query)
		efree(this->query);
	if (this->fragment)
		efree(this->fragment);
	efree(raw_url);	
}

bool URL::RewriteShortHandUrl()
{
	const char *p;
	char * url = this->raw_url ;

	//printf("%s\n" , url);

	/* Look for a ':' or '/'.  The former signifies NcFTP syntax, the
	latter Netscape.  */
	for (p = url; *p && *p != ':' && *p != '/'; p++)
		;

	if (p == url)
		return false ;

	/* If we're looking at "://", it means the URL uses a scheme we
	don't support, which may include "https" when compiled without
	SSL support.  Don't bogusly rewrite such URLs.  */
	if (p[0] == ':' && p[1] == '/' && p[2] == '/')
		return false ;

	if (*p == ':')
	{
		const char *pp;
		char *res;
		/* If the characters after the colon and before the next slash
		or end of string are all digits, it's HTTP.  */
		int digits = 0;
		for (pp = p + 1; isdigit (*pp); pp++)
			++digits;
		if (digits > 0 && (*pp == '/' || *pp == '\0'))
			goto http;

		/* Prepend "ftp://" to the entire URL... */
		res = (char*) malloc (6 + strlen (url) + 1);
		sprintf (res, "ftp://%s", url);
		/* ...and replace ':' with '/'. */
		res[6 + (p - url)] = '/';
		free(this->raw_url) ;this->raw_url = res ;
		return true ;
	}
	else
	{
		char *res;
http:
		/* Just prepend "http://" to what we have. */
		res = (char*) malloc (7 + strlen (url) + 1);
		memset(res, 0 , 7 + strlen (url) + 1);
		sprintf (res, "http://%s", url);
		free(this->raw_url) ;this->raw_url = res ;

		return true ;
	}	

	return false ;
}

int URL::FullPathLength()
{
	int len = 0;
	/*
	#define FROB(el) if (this->el) len += 1 + strlen (this->el)

	FROB (path);
	//FROB (params);
	FROB (query);

	#undef FROB
	*/

	if (this->path)  len += 1 + strlen (this->path ) ; 
	if (this->query) len += 1 + strlen (this->query ) ;

	return len;
}
int URL::FullPathRewrite( char * where )
{
#define FROB(el, chr) do {			\
	char *f_el = this->el;				\
	if (f_el) {					\
	int l = strlen (f_el);			\
	*where++ = chr;				\
	memcpy (where, f_el, l);			\
	where += l;					\
	}						\
} while (0)

	FROB (path, '/');
	//FROB (params, ';');
	FROB (query, '?');

#undef FROB
	return 0 ;
}

////////////////////////////////////////////
/////
////////////////////////////////////////////


/* Add thousand separators to a number already in string form.  Used
by with_thousand_seps and with_thousand_seps_large.  */
//it use a static buff ,but this is not couraged , so change it for multithread env

//static 
char * add_thousand_seps (const char *repr)
{
	//static 
	char outbuf[48];
	int i, i1, mod;
	char *outptr;
	const char *inptr;

	/* Reset the pointers.  */
	outptr = outbuf;
	inptr = repr;

	/* Ignore the sign for the purpose of adding thousand
	separators.  */
	if (*inptr == '-')
	{
		*outptr++ = '-';
		++inptr;
	}
	/* How many digits before the first separator?  */
	mod = strlen (inptr) % 3;
	/* Insert them.  */
	for (i = 0; i < mod; i++)
		*outptr++ = inptr[i];
	/* Now insert the rest of them, putting separator before every
	third digit.  */
	for (i1 = i, i = 0; inptr[i1]; i++, i1++)
	{
		if (i % 3 == 0 && i1 != 0)
			*outptr++ = ',';
		*outptr++ = inptr[i1];
	}
	/* Zero-terminate the string.  */
	*outptr = '\0';
	//return outbuf;
	return strdup(outptr);

}


/* Return non-zero if STRING ends with TAIL.  For instance:

match_tail ("abc", "bc", 0)  -> 1
match_tail ("abc", "ab", 0)  -> 0
match_tail ("abc", "abc", 0) -> 1

If FOLD_CASE_P is non-zero, the comparison will be
case-insensitive.  */

int
match_tail (const char *string, const char *tail, int fold_case_p)
{
	int i, j;

	/* We want this to be fast, so we code two loops, one with
	case-folding, one without. */

	if (!fold_case_p)
	{
		for (i = strlen (string), j = strlen (tail); i >= 0 && j >= 0; i--, j--)
			if (string[i] != tail[j])
				break;
	}
	else
	{
		for (i = strlen (string), j = strlen (tail); i >= 0 && j >= 0; i--, j--)
			//if (TOLOWER (string[i]) != TOLOWER (tail[j]))
			if ( ::tolower (string[i]) != ::tolower (tail[j]))
				break;
	}

	/* If the tail was exhausted, the match was succesful.  */
	if (j == -1)
		return 1;
	else
		return 0;
}



/* Return the location of STR's suffix (file extension).  Examples:
suffix ("foo.bar")       -> "bar"
suffix ("foo.bar.baz")   -> "baz"
suffix ("/foo/bar")      -> NULL
suffix ("/foo.bar/baz")  -> NULL  */
char *
suffix (const char *str)
{
	int i;

	for (i = strlen (str); i && str[i] != '/' && str[i] != '.'; i--)
		;

	if (str[i++] == '.')
		return (char *)str + i;
	else
		return NULL;
}





/* Return non-zero if S contains globbing wildcards (`*', `?', `[' or
`]').  */

int
has_wildcards_p (const char *s)
{
	for (; *s; s++)
		if (*s == '*' || *s == '?' || *s == '[' || *s == ']')
			return 1;
	return 0;
}




/* Return non-zero if FNAME ends with a typical HTML suffix.  The
following (case-insensitive) suffixes are presumed to be HTML files:

html
htm
?html (`?' matches one character)

#### CAVEAT.  This is not necessarily a good indication that FNAME
refers to a file that contains HTML!  */
int
has_html_suffix_p (const char *fname)
{
	char *suf;

	if ((suf = suffix (fname)) == NULL)
		return 0;
	if (!strcasecmp (suf, "html"))
		return 1;
	if (!strcasecmp (suf, "htm"))
		return 1;
	if (suf[0] && !strcasecmp (suf + 1, "html"))
		return 1;
	return 0;
}





/* Encode the string STR of length LENGTH to base64 format and place it
to B64STORE.  The output will be \0-terminated, and must point to a
writable buffer of at least 1+BASE64_LENGTH(length) bytes.  It
returns the length of the resulting base64 data, not counting the
terminating zero.

This implementation will not emit newlines after 76 characters of
base64 data.  */

int
base64_encode (const char *str, int length, char *b64store)
{
	/* Conversion table.  */
	static char tbl[64] = {
		'A','B','C','D','E','F','G','H',
		'I','J','K','L','M','N','O','P',
		'Q','R','S','T','U','V','W','X',
		'Y','Z','a','b','c','d','e','f',
		'g','h','i','j','k','l','m','n',
		'o','p','q','r','s','t','u','v',
		'w','x','y','z','0','1','2','3',
		'4','5','6','7','8','9','+','/'
	};
	int i;
	const unsigned char *s = (const unsigned char *) str;
	char *p = b64store;

	/* Transform the 3x8 bits to 4x6 bits, as required by base64.  */
	for (i = 0; i < length; i += 3)
	{
		*p++ = tbl[s[0] >> 2];
		*p++ = tbl[((s[0] & 3) << 4) + (s[1] >> 4)];
		*p++ = tbl[((s[1] & 0xf) << 2) + (s[2] >> 6)];
		*p++ = tbl[s[2] & 0x3f];
		s += 3;
	}

	/* Pad the result if necessary...  */
	if (i == length + 1)
		*(p - 1) = '=';
	else if (i == length + 2)
		*(p - 1) = *(p - 2) = '=';

	/* ...and zero-terminate it.  */
	*p = '\0';

	return p - b64store;
}

#define IS_ASCII(c) (((c) & 0x80) == 0)
#define IS_BASE64(c) ((IS_ASCII (c) && base64_char_to_value[c] >= 0) || c == '=')

/* Get next character from the string, except that non-base64
characters are ignored, as mandated by rfc2045.  */
#define NEXT_BASE64_CHAR(c, p) do {			\
	c = *p++;						\
} while (c != '\0' && !IS_BASE64 (c))



/* Decode data from BASE64 (assumed to be encoded as base64) into
memory pointed to by TO.  TO should be large enough to accomodate
the decoded data, which is guaranteed to be less than
strlen(base64).

Since TO is assumed to contain binary data, it is not
NUL-terminated.  The function returns the length of the data
written to TO.  -1 is returned in case of error caused by malformed
base64 input.  */

int
base64_decode (const char *base64, char *to)
{
	/* Table of base64 values for first 128 characters.  Note that this
	assumes ASCII (but so does Wget in other places).  */
	static short base64_char_to_value[128] =
	{
		-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,	/*   0-  9 */
		-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,	/*  10- 19 */
		-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,	/*  20- 29 */
		-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,	/*  30- 39 */
		-1,  -1,  -1,  62,  -1,  -1,  -1,  63,  52,  53,	/*  40- 49 */
		54,  55,  56,  57,  58,  59,  60,  61,  -1,  -1,	/*  50- 59 */
		-1,  -1,  -1,  -1,  -1,  0,   1,   2,   3,   4,	/*  60- 69 */
		5,   6,   7,   8,   9,   10,  11,  12,  13,  14,	/*  70- 79 */
		15,  16,  17,  18,  19,  20,  21,  22,  23,  24,	/*  80- 89 */
		25,  -1,  -1,  -1,  -1,  -1,  -1,  26,  27,  28,	/*  90- 99 */
		29,  30,  31,  32,  33,  34,  35,  36,  37,  38,	/* 100-109 */
		39,  40,  41,  42,  43,  44,  45,  46,  47,  48,	/* 110-119 */
		49,  50,  51,  -1,  -1,  -1,  -1,  -1		/* 120-127 */
	};

	const char *p = base64;
	char *q = to;

	while (1)
	{
		unsigned char c;
		unsigned long value;

		/* Process first byte of a quadruplet.  */
		NEXT_BASE64_CHAR (c, p);
		if (!c)
			break;
		if (c == '=')
			return -1;		/* illegal '=' while decoding base64 */
		value = base64_char_to_value[c] << 18;

		/* Process scond byte of a quadruplet.  */
		NEXT_BASE64_CHAR (c, p);
		if (!c)
			return -1;		/* premature EOF while decoding base64 */
		if (c == '=')
			return -1;		/* illegal `=' while decoding base64 */
		value |= base64_char_to_value[c] << 12;
		*q++ = value >> 16;

		/* Process third byte of a quadruplet.  */
		NEXT_BASE64_CHAR (c, p);
		if (!c)
			return -1;		/* premature EOF while decoding base64 */

		if (c == '=')
		{
			NEXT_BASE64_CHAR (c, p);
			if (!c)
				return -1;		/* premature EOF while decoding base64 */
			if (c != '=')
				return -1;		/* padding `=' expected but not found */
			continue;
		}

		value |= base64_char_to_value[c] << 6;
		*q++ = 0xff & value >> 8;

		/* Process fourth byte of a quadruplet.  */
		NEXT_BASE64_CHAR (c, p);
		if (!c)
			return -1;		/* premature EOF while decoding base64 */
		if (c == '=')
			continue;

		value |= base64_char_to_value[c];
		*q++ = 0xff & value;
	}

	return q - to;
}




