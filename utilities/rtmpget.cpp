// rtmpget.cpp:  RTMP file downloader utility
// 
//   Copyright (C) 2008, 2009 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
// 


#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "gettext.h"

// classes internal to Gnash
#include "gnash.h"
#include "network.h"
#include "log.h"
#include "http.h"
#include "limits.h"
#include "netstats.h"
#include "statistics.h"
#include "gmemory.h"
#include "arg_parser.h"
#include "amf.h"
#include "rtmp.h"
#include "rtmp_client.h"
#include "rtmp_msg.h"
#include "buffer.h"
#include "network.h"
#include "element.h"
#include "URL.h"

// classes internal to Cygnal
#include "buffer.h"
#include "handler.h"

#ifdef ENABLE_NLS
#include <locale.h>
#endif

#include <string>
#include <iostream>
#include <sstream>
#include <csignal>
#include <vector>
#include <sys/mman.h>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/time_zone_base.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

using gnash::log_debug;
using namespace std;
using namespace gnash;
using namespace amf;

static void usage();
static void version_and_copyright();
static void cntrlc_handler(int sig);

void connection_handler(Network::thread_params_t *args);
void admin_handler(Network::thread_params_t *args);

LogFile& dbglogfile = LogFile::getDefaultInstance();

// The rcfile is loaded and parsed here:
RcInitFile& rcfile = RcInitFile::getDefaultInstance();

// Toggles very verbose debugging info from the network Network class
static bool netdebug = false;

static struct sigaction  act;

std::vector<std::string> infiles;

// The next few global variables have to be global because Boost
// threads don't take arguments. Since these are set in main() before
// any of the threads are started, and it's value should never change,
// it's safe to use these without a mutex, as all threads share the
// same read-only value.

typedef boost::shared_ptr<amf::Buffer> BufferSharedPtr;
typedef boost::shared_ptr<amf::Element> ElementSharedPtr;

// end of globals

int
main(int argc, char *argv[])
{
    // Initialize national language support
#ifdef ENABLE_NLS
    setlocale (LC_ALL, "");
    bindtextdomain (PACKAGE, LOCALEDIR);
    textdomain (PACKAGE);
#endif
    
    // If no command line arguments have been supplied, do nothing but
    // print the  usage message.
    if (argc < 2) {
        usage();
        exit(0);
    }

   const Arg_parser::Option opts[] =
        {
        { 'h', "help",          Arg_parser::no  },
        { 'V', "version",       Arg_parser::no  },
        { 'p', "port-offset",   Arg_parser::yes },
        { 'v', "verbose",       Arg_parser::no  },
        { 'd', "dump",          Arg_parser::no  },
        { 'a', "app",           Arg_parser::yes  },
        { 'p', "path",          Arg_parser::yes  },
        { 'f', "filename",      Arg_parser::yes  },
        { 't', "tcurl",     Arg_parser::yes  },
        { 's', "swfurl",    Arg_parser::yes  },
        { 'u', "url",           Arg_parser::yes  },
        { 'n', "netdebug",      Arg_parser::no  }
        };

    Arg_parser parser(argc, argv, opts);
    if( ! parser.error().empty() )  
    {
        cout << parser.error() << endl;
        exit(EXIT_FAILURE);
    }

    // Set the log file name before trying to write to
    // it, or we might get two.
    dbglogfile.setLogFilename("rtmpget-dbg.log");
    
    if (rcfile.verbosityLevel() > 0) {
        dbglogfile.setVerbosity(rcfile.verbosityLevel());
    }    

    string app; // the application name
    string path; // the path to the file on the server
    string tcUrl; // the tcUrl field
    string swfUrl; // the swfUrl field
    string filename; // the filename to play

    // Handle command line arguments
    for( int i = 0; i < parser.arguments(); ++i ) {
        const int code = parser.code(i);
        try {
            switch( code ) {
              case 'h':
                  version_and_copyright();
                  usage();
                  exit(0);
              case 'V':
                  version_and_copyright();
                  exit(0);
              case 'v':
                  dbglogfile.setVerbosity();
                  log_debug (_("Verbose output turned on"));
                  break;
              case 'a':
                  app = parser.argument(i);
                  break;
              case 'p':
                  path = parser.argument(i);
                  break;
              case 't':
                  tcUrl = parser.argument(i);
                  break;
              case 's':
                  swfUrl = parser.argument(i);
                  break;
              case 'f':
                  filename = parser.argument(i);
                  break;
              case 'n':
                  netdebug = true;
                  break;
              case 'd':
                  rcfile.dump();
                  exit(0);
                  break;
              case 0:
                  infiles.push_back(parser.argument(i));
                  break;
              default:
                  log_error (_("Extraneous argument: %s"), parser.argument(i).c_str());
            }
        }
        
        catch (Arg_parser::ArgParserException &e) {
            cerr << _("Error parsing command line options: ") << e.what() << endl;
            cerr << _("This is a Gnash bug.") << endl;
        }
    }
    
    if (infiles.empty()) {
        cerr << _("Error: no input file was specified. Exiting.") << endl;
        usage();
        return EXIT_FAILURE;
    }

    RTMPClient client;    
    short port = 0;
    string protocol;        // the network protocol, rtmp or http
    string query;       // any queries for the host
    string pageUrl;     // the pageUrl field
    string hostname;        // the hostname of the server
    
    URL url( infiles[0] );
    string portstr;
    
    // Trap ^C (SIGINT) so we can kill all the threads
    act.sa_handler = cntrlc_handler;
    sigaction (SIGINT, &act, NULL);

    protocol = url.protocol();
    hostname = url.hostname();
    log_debug("hostname: %s", hostname);
    portstr = url.port();
    query = url.querystring();

    if ( portstr.empty() )
    {
        if ((protocol == "http") || (protocol == "rtmpt")) {
            port = RTMPT_PORT;
        }
        if (protocol == "rtmp") {
            port = RTMP_PORT;
        }
    } else {
        port = strtol(portstr.c_str(), NULL, 0) & 0xffff;
    }


    if (path.empty()) {
        path = url.path();
    }

    if (filename.empty()) {
        string::size_type end = path.rfind('/');
        if (end != string::npos) {
            filename = path.substr(end + 1);
	    path = path.substr(0, end);
        }
    }

    if (tcUrl.empty()) {
        tcUrl = protocol + "://" + hostname;
        if (!portstr.empty()) {
            tcUrl += ":" + portstr;
        }
        if (!query.empty()) {
            tcUrl += query;
        } else {
            tcUrl += path;
        }
    }
    
    if (app.empty()) {

        // Get the application name
        // rtmp://localhost/application/resource
        //                  ^^^^^^^^^^^ <-- appname is this
        //
        app = path;
	if ( ! filename.empty() )
	{
        	string::size_type end = app.rfind(filename);
		if (end != string::npos) {
		    app = app.substr(0, end);
		}
	}

	// drop slashes
        string::size_type end = app.find_first_not_of('/');
	if (end != string::npos) {
	    app = app.substr(end);
	}
        end = app.find_last_not_of('/');
	if (end != string::npos) {
	    app = app.substr(0, end+1);
	}
        
        if (!query.empty()) {
            app = path;
            app += "?" + query;
        }
    }

    if (swfUrl.empty()) {
	swfUrl = "file://rtmpget.swf";
    }
    if (pageUrl.empty()) {
        pageUrl = ".http://gnashdev.org";
    }
    
    log_debug("URL is %s", url);
    log_debug("Protocol is %s", protocol);
    log_debug("Host is %s", hostname);
    log_debug("Port is %s", port);
    log_debug("Path is %s", path);
    log_debug("Filename is %s", filename);
    log_debug("App is %s", app);
    log_debug("Query is %s", query);
    log_debug("tcUrl is %s", tcUrl);
    log_debug("swfUrl is %s", swfUrl);
    log_debug("pageUrl is %s", pageUrl);

    // Create a tcp/ipc connection to the server. This doesn't require any RTMP.
    client.toggleDebug(netdebug);
    if (client.createClient(hostname, port) == false) {
        log_error("Can't connect to RTMP server %s", hostname);
        exit(-1);
    }

    // Read the handshake from the server
    if (!client.handShakeRequest()) {
        log_error("RTMP handshake request failed");
        exit(EXIT_FAILURE);
    }
    
    // Make a buffer to hold the handshake data.
    Buffer buf(1537);
    // RTMP::rtmp_head_t *rthead = 0;
    // int ret = 0;
    log_debug("Sending NetConnection Connect message,");
    boost::shared_ptr<amf::Buffer> buf2 = client.encodeConnect(app.c_str(), swfUrl.c_str(), tcUrl.c_str(), 615, 124, 1, pageUrl.c_str());
//  rtmpget -vv -n rtmp://localhost/oflaDemo/DarkKnight.flv
//  boost::shared_ptr<amf::Buffer> buf2 = client.encodeConnect("video/2006/sekt/gate06/tablan_valentin", "mediaplayer.swf", "rtmp://velblod.videolectures.net/video/2006/sekt/gate06/tablan_valentin", 615, 124, 1, "http://gnashdev.org");
//  boost::shared_ptr<amf::Buffer> buf2 = client.encodeConnect("oflaDemo", "http://192.168.1.70/software/gnash/tests/ofla_demo.swf", "rtmp://localhost/oflaDemo/stream", 615, 124, 1, "http://192.168.1.70/software/gnash/tests/index.html");
    //buf2->resize(buf2->size() - 6); // FIXME: encodeConnect returns the wrong size for the buffer!
    boost::shared_ptr<amf::Buffer> head2 = client.encodeHeader(0x3, RTMP::HEADER_12,
								      buf2->allocated(), RTMP::INVOKE,
								      RTMPMsg::FROM_CLIENT);
    head2->resize(head2->size() + buf2->size() + 1);
    // FIXME: ugly hack! Should be a single byte header. Do this in Element::encode() instead!
    head2->append(buf2->reference(), 128);
    boost::uint8_t c = 0xc3;
    *head2 += c;
    head2->append(buf2->reference() + 128, buf2->allocated()-128);

    // Finish the handshake process, which has to have the NetConnection::connect() as part
    // of the buffer, or Red5 refuses to answer.
    if (!client.clientFinish(*head2)) {
	log_error("RTMP handshake completion failed");
    }
    
    // give the server time to process our NetConnection::connect() request
    sleep(1); 

    // Read the responses back from the server.  This is usually a series of system
    // messages on channel 2, and the response message on channel 3 from our request.
    boost::shared_ptr<amf::Buffer> response = client.recvMsg();
    if (!response) {
	log_error("Got no response from the RTMP server");
    }

    // when doing remoting calls I don't see this problem with an empty packet from Red5,
    // but when I do streaming, it's always there, so we need to remove it.
    boost::uint8_t *pktstart = response->reference();
    if (*pktstart == 0xff) {
	log_debug("Got empty packet in buffer.");
	pktstart++;
    }

    // The response packet contains multiple messages for multiple channels, so we
    // we have to split the Buffer into seperate messages on a chunksize boundary.
    boost::shared_ptr<RTMP::rtmp_head_t> rthead;
    boost::shared_ptr<RTMP::queues_t> que = client.split(pktstart, response->allocated()-1);

    // If we got no responses, something obviously went wrong.
    if (!que->size()) {
        log_error("No response from INVOKE of NetConnection connect");
        exit(-1);
    }

    // There is a queue of queues used to hold all the messages. The first queue
    // is indexed by the channel number, the second queue is all the messages that
    // have arrived for that channel.
    while (que->size()) {	// see if there are any messages at all
	// Get the CQue for the first channel
	CQue *channel_q = que->front();
	que->pop_front();	// remove this Cque from the top level que

	while (channel_q->size()) {
	    // Get the first message in the channel queue
	    boost::shared_ptr<amf::Buffer> ptr = channel_q->pop();
// 	    channel_q->pop_front();	// remove this Buffer from the Cque
// 	    ptr->dump();
	    
	    log_debug("%s: There are %d messages in the RTMP input queue, %d",
		      __PRETTY_FUNCTION__, que->size(), que->front()->size());
	    if (ptr) {		// If there is legit data
		rthead = client.decodeHeader(ptr->reference());
		if (!rthead) {
		    log_error("Couldn't decode RTMP message header");
		    continue;
		}
		
		RTMPMsg *msg = client.decodeMsgBody(ptr->reference() + rthead->head_size, rthead->bodysize);
		if (msg) {
// 		    msg->dump();
 		    if (msg->getStatus() ==  RTMPMsg::NC_CONNECT_SUCCESS) {
			if (msg->getMethodName() == "_result") {
			    log_debug("Sent NetConnection Connect message sucessfully");
#if 0
 			    log_debug("Got a result: %s", msg->getMethodName());
			    if (msg->getElements().size() > 0) {
				msg->at(0)->dump();
			    }
#endif
			}
 		    }		    
 		    if (msg->getStatus() ==  RTMPMsg::NC_CONNECT_FAILED) {
			if (msg->getMethodName() == "_error") {
			    log_error("Couldn't send NetConnection Connect message,");
#if 0
			    log_error("Got an error: %s", msg->getMethodName());
			    if (msg->getElements().size() > 0) {
				msg->at(0)->dump();
			    }
#endif
			}
		    }
		} else {
		    log_error("Couldn't decode RTMP message Body");
		    continue;
		}
	    }
	}
    }
	
    // make the createStream
    log_debug("Sending NetStream::createStream message,");
    BufferSharedPtr buf3 = client.encodeStream(0x2);
//    buf3->dump();
    client.sendMsg(0x3, RTMP::HEADER_12, buf3->allocated(), RTMP::INVOKE, RTMPMsg::FROM_CLIENT, *buf3);
    
//     RTMPMsg *msg2 = client.sendRecvMsg(0x3, RTMP::HEADER_12, buf3->allocated(), RTMP::INVOKE, RTMPMsg::FROM_CLIENT, buf3);
    double streamID = 0.0;

//     if (!msg2) {
//         log_error("No response from INVOKE of NetStream::createStream");
//         exit(-1);
//     }

    // Read the responses back from the server.  This is usually a series of system
    // messages on channel 2, and the response message on channel 3 from our request.
    response.reset();
    response = client.recvMsg();
    if (!response) {
	log_error("Got no response from the RTMP server");
    }

    // when doing remoting calls I don't see this problem with an empty packet from Red5,
    // but when I do streaming, it's always there, so we need to remove it.
    pktstart = response->reference();
    if (*pktstart == 0xff) {
	log_debug("Got empty packet in buffer.");
	pktstart++;
    }

    // The response packet contains multiple messages for multiple channels, so we
    // we have to split the Buffer into seperate messages on a chunksize boundary.
    rthead.reset();
    que.reset();
    que = client.split(pktstart, response->allocated()-1);

    // If we got no responses, something obviously went wrong.
    if (!que->size()) {
        log_error("No response from INVOKE of NetConnection connect");
        exit(-1);
    }

    // There is a queue of queues used to hold all the messages. The first queue
    // is indexed by the channel number, the second queue is all the messages that
    // have arrived for that channel.
    while (que->size()) {	// see if there are any messages at all
	// Get the CQue for the first channel
	CQue *channel_q = que->front();
	que->pop_front();	// remove this Cque from the top level que

	while (channel_q->size()) {
	    // Get the first message in the channel queue
	    boost::shared_ptr<amf::Buffer> ptr = channel_q->pop();
// 	    channel_q->pop_front();	// remove this Buffer from the Cque
// 	    ptr->dump();
	    
	    log_debug("%s: There are %d messages in the RTMP input queue, %d",
		      __PRETTY_FUNCTION__, que->size(), que->front()->size());
	    if (ptr) {		// If there is legit data
		rthead = client.decodeHeader(ptr->reference());
		if (!rthead) {
		    log_error("Couldn't decode RTMP message header");
		    continue;
		}
		
		RTMPMsg *msg = client.decodeMsgBody(ptr->reference() + rthead->head_size, rthead->bodysize);
		if (msg) {
// 		    msg->dump();
		    if (msg->getMethodName() == "_result") {
			log_debug("Sent NetConnection createStream message sucessfully");
			if (msg->at(1)->getType() == amf::Element::NUMBER_AMF0) {
			    streamID = msg->at(1)->to_number();
			}
			log_debug("Stream ID returned from createStream is: %g", streamID);
#if 0
			if (msg->getElements().size() > 0) {
			    msg->at(1)->dump();
			}
#endif
		    }
		    if (msg->getMethodName() == "_error") {
			log_error("Couldn't send NetConnection createStream message,");
#if 0
			log_error("Got an error: %s", msg->getMethodName());
			if (msg->getElements().size() > 0) {
			    msg->at(0)->dump();
			}
#endif
		    }
		} else {
		    log_error("Couldn't decode RTMP message Body");
		    continue;
		}
	    }
	}
    }    
    
#if 1
    boost::shared_ptr<amf::Buffer> blob(new Buffer("08 00 00 02 00 00 22 14 01 00 00 00 02 00 04 70 6c 61 79 00 00 00 00 00 00 00 00 00 05 02 00 0e 44 61 72 6b 4b 6e 69 67 68 74 2e 66 6c 76 82 00 00 00 00 03 00 00 00 01 00 00 27 10"));
    blob->dump();
//     client.writeNet(*blob);
    
    // We need the streamID for all folowing operations on this stream
    int id = int(streamID);
    
    // make the NetStream::play() operations for ID 2 encoded object
    log_debug("Sending NetStream play message,");
    BufferSharedPtr buf4 = client.encodeStreamOp(id, RTMP::STREAM_PLAY, false, filename.c_str());
//    BufferSharedPtr buf4 = client.encodeStreamOp(0, RTMP::STREAM_PLAY, false, "gate06_tablan_bcueu_01");
//     log_debug("TRACE: buf4: %s", hexify(buf4->reference(), buf4->size(), true));
    buf4->dump();
    client.sendMsg(0x8, RTMP::HEADER_12, buf4->allocated(), RTMP::INVOKE, RTMPMsg::FROM_CLIENT, *buf4);

    // Read the responses back from the server.  This is usually a series of system
    // messages on channel 2, and the response message on channel 3 from our request.
    response.reset();
    response = client.recvMsg();
    if (!response) {
	log_error("Got no response from the RTMP server");
    }

    // when doing remoting calls I don't see this problem with an empty packet from Red5,
    // but when I do streaming, it's always there, so we need to remove it.
    pktstart = response->reference();
    if (*pktstart == 0xff) {
	log_debug("Got empty packet in buffer.");
	pktstart++;
    }

    // The response packet contains multiple messages for multiple channels, so we
    // we have to split the Buffer into seperate messages on a chunksize boundary.
    rthead.reset();
    que.reset();
    que = client.split(pktstart, response->allocated()-1);

    // If we got no responses, something obviously went wrong.
    if (!que->size()) {
        log_error("No response from INVOKE of NetConnection connect");
        exit(-1);
    }

    // There is a queue of queues used to hold all the messages. The first queue
    // is indexed by the channel number, the second queue is all the messages that
    // have arrived for that channel.
    while (que->size()) {	// see if there are any messages at all
	// Get the CQue for the first channel
	CQue *channel_q = que->front();
	que->pop_front();	// remove this Cque from the top level que

	while (channel_q->size()) {
	    // Get the first message in the channel queue
	    boost::shared_ptr<amf::Buffer> ptr = channel_q->pop();
// 	    channel_q->pop_front();	// remove this Buffer from the Cque
	    ptr->dump();
	    
	    log_debug("%s: There are %d messages in the RTMP input queue, %d",
		      __PRETTY_FUNCTION__, que->size(), que->front()->size());
	    if (ptr) {		// If there is legit data
		rthead = client.decodeHeader(ptr->reference());
		if (!rthead) {
		    log_error("Couldn't decode RTMP message header");
		    continue;
		}
		
		RTMPMsg *msg = client.decodeMsgBody(ptr->reference() + rthead->head_size, rthead->bodysize);
		if (msg) {
// 		    msg->dump();
		    if (msg->getMethodName() == "onStatus") {
			log_error("Got a status message: %s", msg->getMethodName());
// 			msg->at(0)->dump();
			boost::shared_ptr<amf::Element> level = msg->findProperty("level");
			if ((level->getType() == amf::Element::STRING_AMF0) && (strcmp(level->to_string(), "error") == 0)) {
			    boost::shared_ptr<amf::Element> description = msg->findProperty("description");
			    if (description) {
				log_debug("Got an error from NetStream::play()! %s", description->to_string());
			    }
			}
		    }
		    if (msg->getMethodName() == "_error") {
			log_error("Got an error message: %s", msg->getMethodName());
//  			msg->at(0)->dump();
		    }
		    if (msg->getMethodName() == "_result") {
			log_debug("Got a result message: %s", msg->getMethodName());
			if (msg->getElements().size() > 0) {
// 			    msg->at(0)->dump();
			}
		    }
		} else {
		    log_error("Couldn't decode RTMP message Body");
		    continue;
		}
	    }
	}
    }
    
#if 0
    if (msg3) {
        msg3->dump();
        if (msg3->getStatus() ==  RTMPMsg::NS_PLAY_START) {
            log_debug("Sent NetStream::play message sucessfully.");
        } else {
            log_error("Couldn't send NetStream::play message,");
//          exit(-1);
        }
    }

    int loop = 20;
    do {
        BufferSharedPtr msgs = client.recvMsg(1);   // use a 1 second timeout
        if (msgs == 0) {
            log_error("Never got any data!");
            exit(-1);
        }
        RTMP::queues_t *que = client.split(msgs);
        if (que == 0) {
            log_error("Never got any messages!");
            exit(-1);
        }
        deque<CQue *>::iterator it;
        for (it = que->begin(); it != que->end(); it++) {
            CQue *q = *(it);
            q->dump();
        }
        while (que->size()) {
            cerr << "QUE SIZE: " << que->size() << endl;
            BufferSharedPtr ptr = que->front()->pop();
            if (!ptr->empty()) {
                que->pop_front();   // delete the item from the queue
                /* RTMP::rtmp_head_t *rthead = */ client.decodeHeader(ptr);
                msg2 = client.decodeMsgBody(ptr);
                if (msg2 == 0) {
    //              log_error("Couldn't process the RTMP message!");
                    continue;
                }
            } else {
                log_error("Buffer size (%d) out of range at %d", ptr->size(), __LINE__);
                break;
            }
        }
    } while(loop--);
#endif
#endif
//     std::vector<amf::Element *> hell = msg2->getElements();
//     std::vector<amf::Element *> props = hell[0]->getProperties();

//     cerr << "HELL Elements: " << hell.size() << endl;
//     cerr << "HELL Properties: " << props.size() << endl;

// //     cerr << props[0]->getName() << endl;
// //     cerr << props[0]->to_string() << endl;
//     cerr << props[0]->getName() << endl;
// //    cerr << props[0]->to_number() << endl;
//     cerr << props[1]->getName() << ": " << props[1]->to_string() << endl;
//     cerr << props[2]->getName() << ": " << props[3]->to_string() << endl;
//     cerr << props[3]->getName() << ": " << props[3]->to_string() << endl;

//     Element *eell = hell[0]->findProperty("level");
//     if (eell) {
//  eell->dump();
//     }
//     *eell = hell[0]->findProperty("code");
//     if (eell) {
//  eell->dump();
//     }

#if 0
    // Write the packet to disk so we can anaylze it with other tools
    int fd = open("outbuf.raw",O_WRONLY|O_CREAT, S_IRWXU);
    if (fd < 0) {
        perror("open");
    }
    cout << "Writing packet to disk: \"outbuf.raw\"" << endl;
//     write(fd, out, 12);
//     write(fd, outbuf.begin(), amf_obj.totalsize());
    write(fd, buf2->reference(), buf2->size());
    write(fd, buf3->reference(), buf3->size());
    close(fd);
#endif    

    exit(0);
}

// Trap Control-C so we can cleanly exit
static void
cntrlc_handler (int /*sig*/)
{
    log_debug(_("Got an interrupt"));

    exit(-1);
}

static void
version_and_copyright()
{
    cout << "rtmpget " << VERSION << endl
        << endl
        << _("Copyright (C) 2008 Free Software Foundation, Inc.\n"
        "Cygnal comes with NO WARRANTY, to the extent permitted by law.\n"
        "You may redistribute copies of Cygnal under the terms of the GNU General\n"
        "Public License.  For more information, see the file named COPYING.\n")
        << endl;
}


static void
usage()
{
    cout << _("rtmpget -- a file downloaded that uses RTMP.") << endl
        << endl
        << _("Usage: rtmpget [options...] <url>") << endl
        << _("  -h,  --help          Print this help and exit") << endl
        << _("  -V,  --version       Print version information and exit") << endl
        << _("  -v,  --verbose       Output verbose debug info") << endl
        << _("  -n,  --netdebug      Verbose networking debug info") << endl
        << _("  -d,  --dump          display init file to terminal") << endl
        << endl;
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
