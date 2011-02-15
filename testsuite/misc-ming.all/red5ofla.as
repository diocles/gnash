// This test relies on a default deploy of red5 on localhost
//
// Build with:
//        makeswf -n network -o red5test.swf ../Dejagnu.swf red5test.as ../actionscript.all/dejagnu_so_fini.as
// Run with:
//        firefox red5test.swf
// Or:
//        gnash red5test.swf
//
//

note("SWF" + OUTPUT_VERSION + " - " + System.capabilities.version + "\n");
rcsid="red5test.as - <bzr revno here>";

#define info _root.note
#define note _root.note
#define fail_check _root.fail
#define pass_check  _root.pass
#define xfail_check _root.xfail
#define xpass_check _root.xpass

#include "../actionscript.all/check.as"
#include "../actionscript.all/utils.as"

stop();

endOfTest = function()
{
    totals(3);
    trace("ENDOFTEST");
    play();
};

// -P FlashVars='hostname=localhost,rtmptport5080=rtmpport=1935'
hostname = RED5_HOST;

if (rtmpport == undefined) {
    rtmpport = 1935;
    note("No RTMP port specified, defaulting to "+rtmpport);
}

test1 = function(nc)
{
    note("Running test 1");

    ns = new NetStream(nc);

    check_equals(typeof(ns), "object");
    check(ns.constructor == NetStream);

    ns.onMetaData = function(arg) {
    	note("NetStream.onMetaData was called.");
    };

    ns.onCuePoint = function(arg) {
    	note("NetStream.onCuePoint was called.");
    };

    ns.onStatus = function(arg) {
    	note("NetStream.onStatus was called with arg.code of " + arg.code);
    	switch(arg.code) {
	case "NetStream.Play.Start":
	     // This is enough; end the test.
	     ns.close();
	     endOfTest();
	     break;
	case "NetStream.Play.StreamNotFound":
	     // This is not good.
	     fail("Stream not found");
	     endOfTest();
	     break;
	}
    };
    ns.play("avatar.flv");
};

runtests = function(nc)
{
    test1(nc);
};

ncrtmp = new NetConnection();
ncrtmp.statuses = new Array();
ncrtmp.onStatus = function()
{
    this.statuses.push(arguments);
    note('NetConnection.onStatus called with args: ' + dumpObject(arguments));
    lastStatusArgs = ncrtmp.statuses[ncrtmp.statuses.length-1];
    if ((lastStatusArgs[0].level == "status") && (lastStatusArgs[0].code == "NetConnection.Connect.Success")) {
        pass("RTMP connection - status Success");
    } else {
        fail("RTMP connection - status Success");
    }
    runtests(this);
};

rtmpuri = "rtmp://"+hostname+":"+rtmpport+"/oflaDemo";
note("Connecting to "+rtmpuri);
ncrtmp.connect(rtmpuri);

