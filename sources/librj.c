#include "../includes/rj.h"

/*
 * copy until n no matter what /!\
 */
char    *ft_strncpyz(char *dst, const char *src, size_t n)
{
    size_t i;

    i = 0;
    while (i < n)
    {
        dst[i] = src[i];
        i++;
    }
    while (i < n)
        dst[i++] = '\0';
    return (dst);
}

/*
 * check every 16 bytes if the is a `\0'
 */
int		ft_strlen_16(char *str)
{
	int i ;

	i = 0;
	while (str[i] || i == 0)
	{
		i += 15;
		i++;
		if (str[i] == '\0')
		break ;
	}
	return i;
	
}

/*
 * strcat not zero blocking 
 */
char        *ft_strncatz(char *dest, const char *src, size_t n)
{
    size_t  i;
    size_t  j;

    j = ft_strlen_16(dest);
    i = 0;
    while (i < n)
    {
        dest[j + i] = src[i];
        i++;
    }
    dest[j + i] = '\0';
    return (dest);
}
