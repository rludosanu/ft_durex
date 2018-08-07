#include "manix.h"

/*
 * Initialisation of the keys 
 */
void init_key(t_client *c)
{
  srand(time(NULL));
  c->p = 0;
  c->g = 5;
  c->b = 0;
  c->x = 0;
  c->y = 0;
  c->shared_key = 0;
  c->is_key = FALSE;
}

int key_exchange(t_client *c, unsigned char *buf)
{
  char *p_char = NULL;
  char *y_char = NULL;
  char *str = NULL;
  int cpt = 0;

  /*
   * generate a random prime number;
   * send it to the serveur;
   */
  if (!c->p)
    {
      while (1)
	{
	  cpt++;
	  c->p = rand_uint64();
	  if (is_prime(c->p))
	    break;
	  if (cpt == 500)
	    return -2;
	}
      p_char = ft_itoa_a(c->p, 10);
      if (p_char == NULL)
	return (-2);
      if (write(c->sockfd, p_char, strlen((const char *)p_char)) < 0)
	{
	  if (p_char)
	    free(p_char);
	  p_char = NULL;
	return (-2);
	}
    }
  /*
   * get the computed key X from the server
   * generate a random uint64 for private key b
   * send the compute key Y from the server; 
   * (g^b mod p);
   */
  else if (!c->x)
    {
      c->x = ft_atollu((const char *)buf); 
      c->b = rand_uint64();
      
      c->y = powmodp(c->g, c->b, c->p);
      if (c->y == 0 )
	return -2;
      if ((y_char = ft_itoa_a(c->y, 10)) == NULL)
	return -2;
      if (write (c->sockfd, y_char, strlen(y_char)) < 0)
	{
	  if (y_char)
	    free(y_char);
	  y_char = NULL;
	return -2;
	}
      if (y_char)
	free(y_char);
      y_char = NULL;
    }
  /*
   * all public keys are good;
   * create a signature  (X - Y)
   * send it to the client;
   * generate the shared key.
   */
  else if (!c->shared_key)
    {
      /* create signature */
      if ((str = ft_itoa_a(c->x - c->y, 10)) == NULL)
	return -2;

      /* check it */
      if (!strcmp((const char *)buf, str))
	;
      else
	{
	  printf("signature diff\n");
	  if (str)
	    free(str);
	  str = NULL;
	  return (-2);
	}
      if (str)
	free(str);
      str = NULL;

      /* create the shared key */
      c->shared_key = powmodp(c->x, c->b, c->p);
    }
  

    if (!c->shared_key)
      return (-1);
    else
      {
	c->is_key = TRUE;
	return (1);
      }
  
}
