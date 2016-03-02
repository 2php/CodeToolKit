#include <mms.h>
#include <mmsx.h>
#include <iostream>
using namespace std;

mms_t* tt = NULL;
mmsx_t* ttx = NULL;
mms_io_t iot;
int data;

int main(int argc, char* argv[])
{
	cout << "prepare to open url!" <<endl;
	//tt = mms_connect(NULL, NULL, "mms://winmedia.cctv.com.cn/baijiajiangtan/2007/12/baijiajiangtan_300_20071220_1.wmv", 100000);
	//if( !tt )
	{
		//cout << "open url failed!" <<endl;
		//return -1;
	}
	//mms_close(tt);

	ttx = mmsx_connect(NULL, NULL, "mms://winmedia.cctv.com.cn/baijiajiangtan/2007/12/baijiajiangtan_300_20071220_1.wmv", 100000);
	cout << "prepare to open url status!" << ttx <<endl;
	if( !ttx )
	{
		cout << "open url failed!" <<endl;
		return -1;
	}
	int length = mmsx_get_length(ttx);
	cout << "mms stream (length=" << length << ")-->(" << float(length/1024/1024) << "M)" << endl;
	
	int pos = mmsx_get_current_pos(ttx);

	int tmlen = mmsx_get_time_length(ttx);
	cout << "mms stream (time=" << tmlen << "s)-->(" << float(tmlen/60) << "m)" << endl;

	bool able = mmsx_get_seekable(ttx);
	//mmsx_seek(NULL, ttx, 10, 0);
	//mmsx_time_seek(NULL, ttx, float(12));
	char buffer[1024] = {'\0'};
	int count = mmsx_read(0, ttx, buffer, 1024);
	cout << "read buffer length=" << count <<endl;
	mmsx_close(ttx);

	getchar();

	return 0;
}
