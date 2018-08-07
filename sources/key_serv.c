#include "durex.h"

/*
 * inti all keys of the struct client  
 */
void init_key(t_client *c)
{
  daemon_report(LOG_INFO, "Initalisation of keys...");
  srand(time(NULL));
  c->p = 0;
  c->shared_key = 0;
  c->g = 5;
  c->a = 0;
  c->x = 0;
  c->y = 0;
  c->is_key = FALSE;

}

/*
 * exchange of keys : 
 * - server get the public key P frome the client
 * - server generate his public key X and send it to the user
 * - server recv from the client his public key Y
 * - generate a signature of thos 2 keys send it back 
 * - generate the shared key;
 */

int handle_keys(unsigned char *buf, t_client *c)
{

	char *x_str = NULL;
	char *s_str = NULL;
	char *p_str = NULL;
	char *other = NULL;
	
	if (!c->p)
	  {
	/*
	 * get Public key p from the client
	 * set his private key a 
	 * send the generated key x : g^x MOD p
	 */
	    
	    daemon_report(LOG_INFO, "Recving public key P ... buf = ");
	    daemon_report(LOG_INFO, (char *)buf);
		c->p = ft_atollu((char *)buf);
		if (c->p == 0)
		  {
		  daemon_report(LOG_INFO, "error c->p == 0");
		  return -2;
		  }
		p_str = ft_itoa_a(c->p, 10);

		daemon_report(LOG_INFO, "Public key P recved.");
		if (p_str != NULL)
		  free(p_str);

		c->a = 77100074583237325;

		c->x = powmodp(c->g, c->a, c->p);
      
		if ((x_str = ft_itoa_a(c->x, 10)) == NULL)
		  return -2;

		daemon_report(LOG_INFO, "Sending generated public key X");
		if (write (c->sock, x_str, strlen(x_str)) <= 0)
		  return -2;
		daemon_report(LOG_INFO, x_str);
		if (x_str)
		  free(x_str);
		daemon_report(LOG_INFO, "Public key X send.");
    }
  
	/*
	 * get the public key Y from the client 
	 * generate a signature of public keys X and Y
	 * the create the shared key 
	 */
	else if (!c->y)
    {
      daemon_report(LOG_INFO, "Get Pkey Y from the client");
	    c->y = ft_atollu((const char *)buf);

	    /*
	     * create a signature like PubKey(X - Y)
	     */
	    daemon_report(LOG_INFO, "Generate signature...");
	    other = ft_itoa_a(c->x - c->y, 10);
	    daemon_report(LOG_INFO, other);
	    if (write (c->sock, other, strlen(other)) <= 0)
	      return -2;
	    if (other)
	      free(other);
	    daemon_report(LOG_INFO, "Signature send.");

	    /*
	     * calcul of the sharedkey
	     */
	    c->shared_key = powmodp(c->y, c->a, c->p);
	    daemon_report(LOG_INFO, "Shared key created :");
	    if ((s_str = ft_itoa_a(c->shared_key, 10)) == NULL )
	  return -2;
	    
	    daemon_report(LOG_INFO, s_str);
	    if(s_str)
	      free(s_str);
    }

  /*
   * if everything is ok ( P a X Y ) 
   * is_key is set to TRUE
   * else : stay FALSE
   */

	if (c->shared_key)
	  {
	    	c->is_key = TRUE;
	    	return 1;
	  }
	else
	  	return -1;
}
