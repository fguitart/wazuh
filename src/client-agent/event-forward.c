/* @(#) $Id$ */

/* Copyright (C) 2005 Daniel B. Cid <dcid@ossec.net>
 * All right reserved.
 *
 * This program is a free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation
 */

/* Part of the OSSEC HIDS
 * Available at http://www.ossec.net/hids/
 */


#include "shared.h"
#include "agentd.h"

#include "os_net/os_net.h"

#include "sec.h"



/* EventForward v0.1, 2005/11/09
 * Receives a message in the internal queue
 * and forward it to the analysis server.
 */
void *EventForward(void *none)
{
    int recv_b;
    char msg[OS_MAXSTR +2];
    

    /* Initializing variables */
    memset(msg, '\0', OS_MAXSTR +2);
    
    
    /* daemon loop */	
    while(1)
    {
        /* locking mutex */
        if(pthread_mutex_lock(&forwarder_mutex) != 0)
        {
            merror(MUTEX_ERROR, ARGV0);
            return(NULL);
        }

        if(available_forwarder == 0)
        {
            pthread_cond_wait(&forwarder_cond, &forwarder_mutex);
        }

        /* Setting availables to 0 */
        available_forwarder = 0;

        /* Unlocking mutex */
        if(pthread_mutex_unlock(&forwarder_mutex) != 0)
        {
            merror(MUTEX_ERROR, ARGV0);
            return(NULL);
        }


        while((recv_b = recv(logr->m_queue, msg, OS_MAXSTR, MSG_DONTWAIT)) > 0)
        {
            msg[recv_b] = '\0';
            
            send_msg(0, msg);

            /* Check if the server has responded */
            if((time(0) - available_server) > (3*NOTIFY_TIME))
            {
                /* If response is not available, set lock and
                 * wait for it.
                 */
                verbose(SERVER_UNAV, ARGV0);     
                os_setwait();
                
                while((time(0) - available_server) > (3*NOTIFY_TIME))
                {
                    sleep(1);
                }

                verbose(SERVER_UP, ARGV0);
                os_delwait();
            }
        }
    }
    
    return(NULL);
}



/* EOF */
