/** @exampe ex/Intro/HelloWorldQueuePub.c
 */

/*
 *  Copyright 2012-2020 Solace Corporation. All rights reserved.
 *
 *  http://www.solace.com
 *
 *  This source is distributed under the terms and conditions
 *  of any contract or contracts between Solace and you or
 *  your company. If there are no contracts in place use of
 *  this source is not authorized. No support is provided and
 *  no distribution, sharing with others or re-use of this
 *  source is authorized unless specifically stated in the
 *  contracts referred to above.
 *
 *  HelloWorldQueuePub
 *
 *  This sample shows the basics of creating session, connecting a session,
 *  and publishing a guaranteed message to a queue, and how message
 *  acknowledgement is handled. This is meant to be a very basic example
 *  for demonstration purposes.
 */

#include "os.h"
#include "solclient/solClient.h"
#include "solclient/solClientMsg.h"

/*****************************************************************************
 * messageReceiveCallback
 *
 * The message receive callback function is mandatory for session creation.
 *****************************************************************************/
solClient_rxMsgCallback_returnCode_t
messageReceiveCallback ( solClient_opaqueSession_pt opaqueSession_p, solClient_opaqueMsg_pt msg_p, void *user_p )
{
    return SOLCLIENT_CALLBACK_OK;
}

/*****************************************************************************
 * eventCallback
 *
 * The event callback function is mandatory for session creation.
 *****************************************************************************/
void
eventCallback ( solClient_opaqueSession_pt opaqueSession_p,
                solClient_session_eventCallbackInfo_pt eventInfo_p, void *user_p )
{
    if ( ( eventInfo_p->sessionEvent ) == SOLCLIENT_SESSION_EVENT_ACKNOWLEDGEMENT )
        printf ( "Acknowledgement received!\n" );
}

/*****************************************************************************
 * main
 *
 * The entry point to the application.
 *****************************************************************************/
int
main ( int argc, char *argv[] )
{
    /* Context */
    solClient_opaqueContext_pt context_p;
    solClient_context_createFuncInfo_t contextFuncInfo = SOLCLIENT_CONTEXT_CREATEFUNC_INITIALIZER;

    /* Session */
    solClient_opaqueSession_pt session_p;
    solClient_session_createFuncInfo_t sessionFuncInfo = SOLCLIENT_SESSION_CREATEFUNC_INITIALIZER;

    /* Session Properties */
    const char     *sessionProps[20];
    int             propIndex = 0;

    /* Message */
    solClient_opaqueMsg_pt msg_p = NULL;
    solClient_destination_t destination;
    const char *text_p = "Hello world!";

    if ( argc < 5 ) {
        printf ( "Usage: HelloWorldPub <msg_backbone_ip:port> <vpn> <client-username> <queue>\n" );
        return -1;
    }

    /*************************************************************************
     * Initialize the API (and setup logging level)
     *************************************************************************/

    /* solClient needs to be initialized before any other API calls. */
    solClient_initialize ( SOLCLIENT_LOG_DEFAULT_FILTER, NULL );
    printf ( "HelloWorldQueuePub initializing...\n" );

    /*************************************************************************
     * Create a Context
     *************************************************************************/

    /* 
     * Create a Context, and specify that the Context thread should be created 
     * automatically instead of having the application create its own
     * Context thread.
     */
    solClient_context_create ( SOLCLIENT_CONTEXT_PROPS_DEFAULT_WITH_CREATE_THREAD,
                                           &context_p, &contextFuncInfo, sizeof ( contextFuncInfo ) );

    /*************************************************************************
     * Create and connect a Session
     *************************************************************************/

    /*
     * Message receive callback function and the Session event function
     * are both mandatory. In this sample, default functions are used.
     */
    sessionFuncInfo.rxMsgInfo.callback_p = messageReceiveCallback;
    sessionFuncInfo.rxMsgInfo.user_p = NULL;
    sessionFuncInfo.eventInfo.callback_p = eventCallback;
    sessionFuncInfo.eventInfo.user_p = NULL;

    /* Configure the Session properties. */
    propIndex = 0;

    sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_HOST;
    sessionProps[propIndex++] = argv[1];

    sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_VPN_NAME;
    sessionProps[propIndex++] = argv[2];

    sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_USERNAME;
    sessionProps[propIndex++] = argv[3];

    sessionProps[propIndex] = NULL;

    /* Create the Session. */
    solClient_session_create ( ( char ** ) sessionProps,
                               context_p,
                               &session_p, &sessionFuncInfo, sizeof ( sessionFuncInfo ) );

    /* Connect the Session. */
    solClient_session_connect ( session_p );
    printf ( "Connected.\n" );

    /*************************************************************************
     * Publish
     *************************************************************************/

    /* Allocate a message. */
    solClient_msg_alloc ( &msg_p );

    /* Set the delivery mode for the message. */
    solClient_msg_setDeliveryMode ( msg_p, SOLCLIENT_DELIVERY_MODE_PERSISTENT );

    /* Set the destination. */
    destination.destType = SOLCLIENT_QUEUE_DESTINATION;
    destination.dest = argv[4];
    solClient_msg_setDestination ( msg_p, &destination, sizeof ( destination ) );

    /* Add some content to the message. */
    solClient_msg_setBinaryAttachment ( msg_p, text_p, ( solClient_uint32_t ) strlen ( (char *)text_p ) );

    /* Send the message. */
    printf ( "About to send message '%s' to queue '%s'...\n", (char *)text_p, argv[4] );
    solClient_session_sendMsg ( session_p, msg_p );
    printf ( "Message sent.\n" );

    /* Free the message. */
    solClient_msg_free ( &msg_p );

    /* Sleep to allow the message to be acknowledged. */
    SLEEP ( 2 );

    printf ( "Exiting.\n" );

    /*************************************************************************
     * CLEANUP
     *************************************************************************/

    /* Cleanup solClient. */
    solClient_cleanup (  );

    return 0;
}
