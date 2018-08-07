#include "durex.h"

/*
 * a * b % P
 */
static inline uint64_t
mul_mod_p(uint64_t a, uint64_t b, uint64_t P)
{
    uint64_t m = 0;
    uint64_t t = 0;

    while (b) 
	{
        if (b & 1) 
		{
            t = P-a;
            if ( m >= t) 
                m -= t;
			else
                m += a;
        }
        if (a >= P - a) 
            a = a * 2 - P;
		else
            a = a * 2;
        b>>=1;
    }
    return m;
}

/*
 * a ^ b % P
 */
static inline uint64_t
pow_mod_p(uint64_t a, uint64_t b, uint64_t P)
{
    uint64_t t = 0;

    if (b == 1) 
		return a;
	
	t = pow_mod_p(a, b >> 1, P);
    t = mul_mod_p(t, t, P);
    if (b % 2)
        t = mul_mod_p(t, a, P);
    return t;
}

/*
 * calc a^b % p
 */
uint64_t
powmodp(uint64_t a, uint64_t b, uint64_t P) 
{

    if (a > P)
        a %= P;
    return pow_mod_p(a, b, P);
}

/*
 * create a big 64 bits int random 
 */
uint64_t rand_uint64(void)
{
    uint64_t r = 0;
    
    for (int i = 0; i < 64; i += 30)
        r = r*((uint64_t)RAND_MAX + 1) + rand();
    return r;
}

/*
 * check if num is prime ret 1 if success
 */
int is_prime(int num)
{
    if (num <= 1) return 0;
    if (num % 2 == 0 && num > 2) return 0;
    for(int i = 3; i < num / 2; i+= 2)
    {
        if (num % i == 0 )
            return 0;
    }
    return 1;
}

/*
 * ft_isdigit()
 */
static int     ft_isdigit(int c)
{
    if (c >= '0' && c <= '9')
        return (1);
    else
        return (0);
}

/*
 * char to llu 
 */
uint64_t                ft_atollu(const char *str)
{
    uint64_t                res;
    unsigned long long int  i;
    if (!str)
        return 0;
    i = 0;
    res = 0;
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\v' ||
		   *str == '\r' || *str == '\f')
        str++;
    if (*str == '-' || *str == '+')
        str++;
    while (*str == '0')
        str++;
    if (strlen(str) > 20)
        return (0);
    while (ft_isdigit(*str) && *str != '\0')
    {
        i = (int)(*str - '0');
        res = (res * 10) + i;
        str++;
    }
    return (res);
}



/*
 * llu to char 
 */
#define BASE_NBRS "0123456789abcdef"

char	*ft_revstr(char *str)
{
	size_t	i;
	char	c;
	char	*save;

	save = str;
	i = strlen(str);
	while (i >= 2)
	{
		c = *str;
		*str = str[i - 1];
		str[i - 1] = c;
		str++;
		i -= 2;
	}
	return (save);
}
char	*ft_itoa_c(uint64_t value, char *str, int base)
{
	char	*save;
	int		i;

	if (base > 36 || base < 2 || !(save = str))
		return (NULL);
	else if (value == 0)
		*(str++) = '0';
	i = 0;
	while (value != 0)
	{
		str[i++] = BASE_NBRS[value % base];
		value /= base;
	}
	str[i] = '\0';
	ft_revstr(str);
	return (save);
}
char	*ft_itoa_a(uint64_t value, int base)
{
	char	*str;
	int		i;
	if (value > UINT64_MAX)
	  return NULL;
	i = 20;
	if (i == 0 || base > 36 || base < 2)
		return (NULL);
	str = (char*)malloc(sizeof(char) * i + 1);
	if (!str)
		return (NULL);
	return (ft_itoa_c(value, str, base));
}
