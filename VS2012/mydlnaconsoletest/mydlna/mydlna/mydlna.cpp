// mydlna.cpp : 定义控制台应用程序的入口点。
#include "stdafx.h"
#include "DLNACore.h"
#include "DLNADelegation.h"
using namespace deejay;
int _tmain(int argc, _TCHAR* argv[])
{	
/*
	//DLNACoreDelegate* d = new DLNACoreDelegate();	
	DLNACore * dd = new DLNACore(d);

	dd->setProperty("OSVersion", "1.1");
	dd->setProperty("FriendlyName", "NetgareTest");
	const char *dmrProtocolInfo =
    "http-get:*:image/png:*,"
    "http-get:*:image/jpeg:*,"
    "http-get:*:image/bmp:*,"
    "http-get:*:image/gif:*,"
    "http-get:*:audio/mpeg:*,"
    "http-get:*:audio/3gpp:*,"
    "http-get:*:audio/mp4:*,"
    "http-get:*:audio/x-ms-wma:*,"
    "http-get:*:audio/wav:*,"
    "http-get:*:video/mp4:*,"
    "http-get:*:video/mpeg:*,"
    "http-get:*:video/x-ms-wmv:*,"
    "http-get:*:video/x-ms-asf:*,"
    "http-get:*:video/3gpp:*,"
    "http-get:*:video/avi:*,"
    "http-get:*:video/quicktime:*"
    ;
	dd->setProperty("DMRProtocolInfo", dmrProtocolInfo);
	dd->setProperty("PlatformName", "Win8");
	
	//dd->start();
	//NPT_MemoryStream strm(
	//dd->loadConfig(NULL);
	dd->start();

	dd->enableFunction(deejay::DLNACore::Function_MediaRenderer, true);	
	dd->enableFunction(deejay::DLNACore::Function_MediaServer, true);	
	dd->enableFunction(deejay::DLNACore::Function_ControlPoint, true);
	
	dd->start();

	dd->searchDevices(30);

	dd->importFileSystemToMediaServer("c:\\temp", "My Media",false);

	//dd->snapshotMediaServerList();
	//dd->snapshotMediaRendererList();
*/

	DLNADelegation::GetInstance()->startUPNPRender();
	DLNADelegation::GetInstance()->startUPNPServer();
	DLNADelegation::GetInstance()->startUPNPControlPoint();

	char buf[256];
    while (gets(buf)) {
        if (*buf == 'q')
            break;
    }

	return 0;
}